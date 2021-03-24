#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#include "client.h"
#include "constants.h"
#include "address_functions.h"

char my_username[INPUT_LENGTH];
char current_session[INPUT_LENGTH];

void load_login_info(struct Message * packetStruct, char * username, char * password){
    strcpy(packetStruct -> data, "<");
    strcat(packetStruct -> data, username);
    strcat(packetStruct -> data, ",");
    strcat(packetStruct -> data, password);
    strcat(packetStruct -> data, ">");
    // printf(___space___(The packet data after loading: %s), packetStruct.data);
}

void load_session_id(struct Message * packetStruct, char * id){
    strcpy(packetStruct -> data, "<");
    strcat(packetStruct -> data, id);
    strcat(packetStruct -> data, ">");
}

/* -------------------------------
* NOTE! C++ does a language-type check
* on pointers if an array is passed
* into final packet, to make sure
* that the array is actually fixed and 
* its length is not determined on runtime.
* --------------------------------- */
int send_n_receive(sock_t sockfd, char * finalPacket, char * message, enum state * clientState){
    /**
     * SEND DATA PACKET TO SERVER 
     */
    pthread_mutex_lock(&mutex);
    int numbytes;

    fd_set select_fds;
    double timeoutInterval = 10; // we start out at 10 seconds
    struct timeval timeout;
    
    // Strlen is fine because we are not sending binary
    if ((numbytes = send(sockfd, finalPacket, strlen(finalPacket)+1, 0)) < 0){
        close(sockfd);
        *clientState = ON_LOCAL;
        memset(&my_username[0], 0, sizeof(my_username));
        memset(&current_session[0], 0, sizeof(current_session));
        perror(___space___(Client: send));
        return -1;
    }
    
    /* Recv from server. If there is no response in 10s, close socket and quit. */
    FD_ZERO(&select_fds);
    FD_SET(sockfd, &select_fds);  
    timeout.tv_sec = (int) timeoutInterval;
    timeout.tv_usec = (int) ((timeoutInterval - timeout.tv_sec)*1000000);
    int selectSignal = select(sockfd + 1, &select_fds, NULL, NULL, &timeout); // Timeout of 10 seconds is setup for login.
    switch (selectSignal){
        case -1:
            perror(___space___(Client: Failed while waiting for server response.));
            *clientState = ON_LOCAL;
            memset(&my_username[0], 0, sizeof(my_username));
            memset(&current_session[0], 0, sizeof(current_session));
            close(sockfd);
            return -1;
        case 0:
            perror(___space___(Client has not received an ACK from the server in 10 seconds or less. Client timed out! Try Login again.));
            *clientState = ON_LOCAL;
            memset(&my_username[0], 0, sizeof(my_username));
            memset(&current_session[0], 0, sizeof(current_session));
            close(sockfd); // packet that comes from server will get lost. That's ok.
            return -1;
        default:
            // We need to close on server side too. They will know that this failed.
            if((numbytes = recv(sockfd, message, MAXBUFLEN, 0)) < 0){
                perror(___space___(Client: recv));
                *clientState = ON_LOCAL;
                memset(&my_username[0], 0, sizeof(my_username));
                memset(&current_session[0], 0, sizeof(current_session));
                close(sockfd);
                return -1;
            }

            printf(___space___(This is the server acknowledgement: %s), message);
            break;
    } // switch
    pthread_mutex_unlock(&mutex);
    return 0;
} 


void construct_packet_client(struct Message packetStruct, packet_t type, char * username, char * finalPacket){
    /**
     * Args: message_struct, final_packet
     * Description: Process the packet so that it is ready to send. Data has to be loaded beforehand, while all the other
     * fields will be taken care of in this function.
     */   
    assert(finalPacket != NULL);

    /* We have to use the username passed in from args, the first time when we create the client id.
     * Note that we will allow to client to use a different username to get back onto the server
     * once they have logged out.*/
    if (username != NULL){
        strcat(packetStruct.source, username);
        strcpy(my_username, packetStruct.source); // Once the client has attempted to login, we will assign him or her a username.
    } else {
        /* When we are already on server level. We pass in NULL into username, to indicate that we are already logged in. 
        * Thus my_username will already have been declared. */
        strcpy(packetStruct.source, my_username);
    }
    sprintf(packetStruct.type, "%d", type);
    sprintf(packetStruct.size, "%d", strlen(packetStruct.data));
    
    strcpy(finalPacket, packetStruct.type);
    strcat(finalPacket, ":");
    strcat(finalPacket, packetStruct.size);
    strcat(finalPacket, ":");
    strcat(finalPacket, packetStruct.source);
    strcat(finalPacket, ":");
    strcat(finalPacket, packetStruct.data);
} 

/* This function most likely has to return the socket that we want to access */
sock_t login(int nargs, char ** argv, sock_t sock_num, enum state * clientState){
    /**
     * Args: Client ID, Password, Server-IP, Server-Port
     * Description: Log into the server at the given address and port. The
     * IP address is specified in the string dot format.
     */
    if (nargs != 5){
        printf(___space___(Usage: /login <client ID> <password> <server-IP> <server-port>));
        return -1; 
    } 

    if (*clientState == ON_SERVER || *clientState == IN_SESSION){
        printf(___space___(You can't login if you've already logged in!));
        return sock_num;
    }

    int new_sockfd;
    struct sockaddr_storage their_addr;

    char finalPacket[MAXBUFLEN];
    char message[MAXBUFLEN];
    /* ^^^ File/packet processing functions ^^^ */
    

    /**
     * TCP SERVER PORT BINDING AND THREE-WAY HANDSHAKE ARE DONE HERE
     */
    if ((new_sockfd = setup_tcp(argv[3], argv[4])) < 0){
        return -1;
    }
    
    /* Read necessary parameters and construct the finalPacket */
    struct Message messageInfo;
    load_login_info(&messageInfo, argv[1], argv[2]); // load username and password into packet;
    printf(___space___(Here is the data after it is loaded: %s), messageInfo.data);
    construct_packet_client(messageInfo, LOGIN, argv[1], finalPacket);
    if(send_n_receive(new_sockfd, finalPacket, message, clientState) == -1){
        return -1;
    }
    
    /**
     *  Process ACK message. this part is different for every part,
     *  so we dont need a function for it.
     */

    *clientState = ON_SERVER;
    return new_sockfd;


















//     /*
//     * 1. Prompt user to enter ftp <filename>
//     * Send and receive if syntax is correct, else ask user to enter again
//     */
//     bool REINPUT = false;
//     while (true){
//         REINPUT=false;
//         printf("\nYou can check for the existence of a file by using the following format: \n\t ftp <filename>. \n\n");
//         char * userInput = (char *) malloc(MAXBUFLEN * sizeof(char));
//         if (userInput == NULL){
//             perror(___space(Could not allocate space for user input.)___);
//             exit(1);
//         }

//         fgets(userInput, MAXBUFLEN, stdin); // Will only write to char array with defined length
//         if (userInput == NULL){
//             printf("Please enter a non-empty string");
//             continue;
//         } // Null pointer check

//         size_t length = strlen(userInput);

//         /* fgets workaround. Sometimes \n is read at the end of buffer, so we wanna replace it with \0*/
//         if (userInput[length - 1] == '\n') {
//             userInput[length - 1] = '\0';
//         }

// /*****  YEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEET **********************************************************************************************/

//         if (REINPUT) continue; // Ask for user input after invalid input.

// /************      FILE PROCESSING    *********************************************************************************************************************************************/
//         if ((file = fopen(filename, "rb")) != NULL){
//             struct PacketInfo packet;
//             fstat(fileno(file), &sb);
//             packet.total_frag = (int) ceil(sb.st_size / (double) MAXBUFLEN);
//             packet.filename = filename; // temporary assignment
            
//             clock_t start, end;
//             start = clock();
            
//             // Process 1000 bytes every time
//             for (int i = 0; ((bytesRead = fread(packet.filedata, 1, 1000, file)) > 0); i++){
//                 /* Clear memory */
//                 memset(finalPacket, 0, (MAXBUFLEN + MAXBUFLEN/2)*sizeof(char));  
//                 packet.total_frag = (int) ceil(sb.st_size / (double) MAXBUFLEN);
//                 packet.frag_no = i + 1;
//                 packet.size = bytesRead;

//                 char tempProcessor[MAXBUFLEN+1]; // For processing into one packet
//                 sprintf(tempProcessor, "%d:", packet.total_frag);
//                 strcpy(finalPacket + strlen(finalPacket), tempProcessor);
//                 sprintf(tempProcessor, "%d:", packet.frag_no);
//                 strcpy(finalPacket + strlen(finalPacket), tempProcessor);
//                 sprintf(tempProcessor, "%d:", packet.size);
//                 strcpy(finalPacket + strlen(finalPacket), tempProcessor);
//                 strcpy(finalPacket + strlen(finalPacket), filename);
//                 strcpy(finalPacket + strlen(finalPacket), ":");
    
//                 // Encode the message into network bits.
//                 u_int16_t decoder = 0;
//                 char returnOutput[bytesRead];
//                 for (size_t i = 0; i < bytesRead; i++){
//                     // if last index is even, we have an odd-sized byte message
//                     if (i == bytesRead - 1 && i % 2 == 0){
//                         returnOutput[i] = packet.filedata[i];
//                         break;
//                     } // if
                
//                     if (i % 2 != 0){
//                         decoder = (packet.filedata[i-1] << 8) + packet.filedata[i];
//                         decoder = htons(decoder); // figure out the right ordering of the 2 bytes for the host. 
//                         // printf("%x\n", decoder);
//                         returnOutput[i-1] = (decoder >> 8) & 0xff;
//                         returnOutput[i] = decoder & 0xff;
//                     } // if
//                 }  // for

//                 // Use packet.filedata is TA is unhappy
//                 memcpy(finalPacket + strlen(finalPacket), packet.filedata, sizeof(packet.filedata));
//                 // memcpy(finalPacket + strlen(finalPacket), returnOutput, sizeof(returnOutput));
//                 get_ip(p->ai_addr, their_ip);
//                 printf("Sending messages to %s...\n", their_ip);
//                 printf("This is what we will send: %s", finalPacket);

                
//                 if ((numbytes = sendto(sockfd, finalPacket, MAXBUFLEN + MAXBUFLEN/2, 0,
//                     p->ai_addr, p->ai_addrlen)) == -1) {
//                         perror(___space(talker: sendto)___);
//                         exit(1);
//                 }
//                 printf("\ntalker: sent the message to the listener in Network Byte Order\n");
//                 printf("talker: sent %d bytes to %s\n", bytesRead, argv[1]);

//                 printf("talker: waiting for recvfrom...\n");
//                 if ((numbytes = recvfrom(sockfd, message, MAXBUFLEN + MAXBUFLEN/2, 0, (struct sockaddr *)&their_addr, &their_length)) == -1){
//                     perror(___space(talker: recvfrom)___); // why MAXBUFLEN-1
//                     exit(1);
//                 }
                

//                 printf("This is the string that we got back: %s\n", message);
//                 printf("This is the original filedata: %s\n", packet.filedata);
//                 // TA Unhappy change mark
//                 printf("Are the messages equal? %d\n", memcmp(message, packet.filedata, sizeof(packet.filedata)));
//                 printf("%d bytes received\n", numbytes);
//             }
//             end = clock();
//             printf("The Round Trip Time is: %.6fs", (end - start)/ (double) CLOCKS_PER_SEC);
//             fclose(file);
//         } else {
//             printf("The file does not exist\n");
//             exit(1);
//         }// https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c/230068#230068
//         // fclose(file);
        
//         // if (strcmp("yes", message) == 0){ 
//         //     printf("A file transfer can start.\n");
//         // } else { 
//         //     exit(1);
//         // }
//     }

//     freeaddrinfo(res);
//     close(sockfd);
}

void logout(int nargs, char ** args, sock_t sockfd, enum state * clientState){
    /**
     *  When we logout, we close our connection with the server.
     */
    if (nargs != 1){
        printf(___space___(Usage: /logout));
        return;
    }

    struct Message messageInfo;
    char finalPacket[MAXBUFLEN + MAXBUFLEN/2];
    char message[MAXBUFLEN + MAXBUFLEN/2];

    /**
     * Recognize that this process is a waterfall (step-by-step dependent
     * on the client state)
     */ 
    if (*clientState == ON_LOCAL){
        printf(___space___(You cannot logout because you are not
        logged in));
        return;
    }

    if (*clientState == IN_SESSION){
        leave_session(1, args, sockfd, clientState);
        if (*clientState == IN_SESSION){
            printf(___space___(Failed to leave room));
            return;
        }
    }
    
    if (*clientState == ON_SERVER){
        construct_packet_client(messageInfo, EXIT, USERNAME_SET, finalPacket);
        strcpy(finalPacket, "");
        if(send_n_receive(sockfd, finalPacket, message, clientState) == -1){
            return;
        } 
    }

    *clientState = ON_LOCAL;
}

void receive_loop(sock_t sockfd, enum state * clientState){
    printf(___space___(Entered receive loop!));
    char buff[MAXBUFLEN + MAXBUFLEN/2 + 1];
    char message[MAXBUFLEN + MAXBUFLEN/2 + 1];
    int numbytes = 0;
    fd_set select_fds;
    double timeoutInterval = 60; // we start out at 10 seconds
    struct timeval timeout;
    /* Recv from server. If there is no response in 10s, close socket and quit. */
    FD_ZERO(&select_fds);
    FD_SET(sockfd, &select_fds);  
    timeout.tv_sec = (int) timeoutInterval;
    timeout.tv_usec = (int) ((timeoutInterval - timeout.tv_sec)*1000000);
    int count = 0;
    while (*clientState == IN_SESSION){   
        int selectSignal = select(sockfd + 1, &select_fds, NULL, NULL, &timeout); // Timeout of 10 seconds is setup for login.
        pthread_mutex_lock(&mutex);
        switch (selectSignal){
            case -1:
                perror(___space___(Client: Failed while waiting for server response.));
                close(sockfd);
                *clientState = ON_LOCAL;
                memset(&current_session[0], 0, sizeof(current_session));
                memset(&my_username[0], 0, sizeof(my_username));
                pthread_mutex_unlock(&mutex);
                return;
            case 0:
                perror(___space___(Client has not received an ACK from the server in 10 seconds or less. Client timed out! Try Login again.));
                close(sockfd);
                *clientState = ON_LOCAL;
                memset(&current_session[0], 0, sizeof(current_session));
                memset(&my_username[0], 0, sizeof(my_username));
                pthread_mutex_unlock(&mutex);
                return;
            default:
                if((numbytes = recv(sockfd, message, MAXBUFLEN, 0)) < 0){
                    perror(___space___(Client: recv));
                    close(sockfd);
                    *clientState = ON_LOCAL;
                    memset(&current_session[0], 0, sizeof(current_session));
                    memset(&my_username[0], 0, sizeof(my_username));
                    pthread_mutex_unlock(&mutex);
                    return;
                }

                printf(___space___(This is the server acknowledgement: %s), message);
                pthread_mutex_unlock(&mutex);
                break;
        } // switch
        if (count == 0){
            printf("We are in the session");
        }
        count++;
    }
    printf(___space___(Receive loop exitted!));

    /** --------------------------------------------------------
     * This function will naturally quit when client is not 
     * in session.
     ---------------------------------------------------------*/
    return;
}

void join_session(int nargs, char ** args, sock_t sockfd, enum state * clientState){
    /**
     * Name: Join_session
     * Description: Join an available session on the server. We don't need to keep track of the room
     * on the client side, because the server side can decide which room our client is in, as long
     * as it has the client ID.
     */
    if (nargs != 2){
        printf(___space___(Usage: /joinsession <session ID>));
        return;
    }

    if (*clientState == ON_LOCAL){
        printf(___space___(You cannot join a session because you are not connected to a server.));
        return;
    } else if (*clientState == IN_SESSION){
        /** -------------------------------------------------------------------------------- 
         * Desired Implementation: 
         * If current client session == session in args, notify client we're already in this room.
         * Else, leave the current session, and join a new one. 
         * 
         * Current Implementation: 
         * You cannot join a session if you are already in a session. 
         * --------------------------------------------------------------------------------- */
        if (!strcmp(args[1], current_session)){
            printf(___space___(You are already in this session.));
            return;
        } else {
            leave_session(1, args, sockfd, clientState);
            if (*clientState == IN_SESSION){
                printf(___space___(Leave room failed.));
                return;
            }
            // leave_session
        }
    } // else clientState == ON_SERVER
        

    struct Message messageInfo;
    char finalPacket[MAXBUFLEN];
    char message[MAXBUFLEN];
    
    /* load session arg */
    load_session_id(&messageInfo, args[1]);
    

    /* Construct the message packet.*/
    construct_packet_client(messageInfo, JOIN, USERNAME_SET, finalPacket);

    /* Send join message */
    if(send_n_receive(sockfd, finalPacket, message, clientState) == -1){
        return;
    }

    /** Process ACK stored in message here **/
    *clientState = IN_SESSION;
    strcpy(current_session, args[1]);

    if (fork() == 0){
        receive_loop(sockfd, clientState); // child_process that ends when client is not in session
        exit(0);
    }
}

void leave_session(int nargs, char ** args, sock_t sockfd, enum state * clientState){
    if (nargs != 1){
        printf(___space___(Usage: /leavesession));
        return;
    }

    if (*clientState == ON_LOCAL){
        printf(___space___(You are not connected to the server. 
        You cannot leave a session));
        return;
    } else if (*clientState == ON_SERVER){
        printf(___space___(You cannot leave a session if you are not in a session));
        return;
    }
    
    struct Message messageInfo;
    char finalPacket[MAXBUFLEN];
    char message[MAXBUFLEN];

    strcpy(messageInfo.data, "");

    construct_packet_client(messageInfo, LEAVE_SESS, USERNAME_SET, finalPacket);

    /**  ---------------------------------------------------------------
     * If client fails sending or receiving, we close it. Server 
     * need close it too. After not getting a response for awhile.
    */
    if(send_n_receive(sockfd, finalPacket, message, clientState) == -1){
        return;
    }
    

    /** Process ACK stored in message here **/
    *clientState = ON_SERVER;
    memset(&current_session[0], 0, sizeof(current_session));
}

void create_session(int nargs, char ** args, sock_t sockfd, enum state * clientState){
    if (nargs != 2){
        printf(___space___(Usage: /createsession <session_ID>));
    }

    if(*clientState == ON_LOCAL){
       printf(___space___(You are not connected to the server. 
        You cannot create a session));
        return;
    } else if (*clientState == IN_SESSION){
        printf(___space___(You cannot create a session while 
        you are in a session));
        return;
    }

    struct Message messageInfo;
    char finalPacket[MAXBUFLEN];
    char message[MAXBUFLEN];

    load_session_id(&messageInfo, args[1]);
    construct_packet_client(messageInfo, NEW_SESS, USERNAME_SET, finalPacket);
    if(send_n_receive(sockfd, finalPacket, message, clientState) == -1){
        return;
    }

    /** Join session will set clientState = IN_SESSION,
     * Set current_session,
     * and it will let us join the room once we created it. **/
    join_session(nargs, args, sockfd, clientState);
}

void list(int nargs, char ** args, sock_t sockfd, enum state * clientState){
    if (nargs != 1){
        printf(___space___(Usage: /list));
        return;
    }

    if (*clientState == ON_LOCAL){
        printf(___space___(Cannot get a list of connected clients
        and available sessions if we are not connected to a server));
        return;
    }

    struct Message messageInfo;
    char finalPacket[MAXBUFLEN];
    char message[MAXBUFLEN];

    strcpy(messageInfo.data, "");

    construct_packet_client(messageInfo, QUERY, USERNAME_SET, finalPacket);
    if(send_n_receive(sockfd, finalPacket, message, clientState) == -1){
        return;
    }

    /** Process ACK stored in message here **/
}

void quit(int nargs, char ** args, sock_t sockfd, enum state * clientState){
    if (nargs != 1){
        printf(___space___(Usage: /quit));
        return;
    }

    /** Leave rooms **/
    if (*clientState == IN_SESSION){
        leave_session(1, args, sockfd, clientState);
        /** ------------------------------------------------------------------
         * If leave session fails, the socket is closed anyways so quit acts
         * without an ACK from the server. 
         * -------------------------------------------------------------------*/
        if (*clientState == IN_SESSION){
            return;
        }
    }
    
    /** Log out **/
    struct Message messageInfo;
    char finalPacket[MAXBUFLEN];
    char message[MAXBUFLEN];

    if (*clientState == ON_SERVER){
        construct_packet_client(messageInfo, EXIT, USERNAME_SET, finalPacket);
        if(send_n_receive(sockfd, finalPacket, message, clientState) == -1){
            return;
        }  

        /** Process ACK **/
        close(sockfd);
        *clientState = ON_LOCAL;
    }

    /** Exit **/
    exit(0);
}

void send_text(char * text, sock_t sockfd, enum state * clientState){
    pthread_mutex_lock(&mutex);
    int numbytes;
    if (*clientState == ON_LOCAL){
        printf(___space___(You cannot send anything because 
        you are not logged into the server!));
        return;
    }

    if (*clientState == ON_SERVER){
        printf(___space___(You cannot send text because
        you are not in a session));
        return;
    }

    if ((numbytes = send(sockfd, text, MAXBUFLEN, 0)) < 0){
        close(sockfd);
        memset(&current_session[0], 0, sizeof(current_session));
        memset(&my_username[0], 0, sizeof(my_username));
        *clientState = ON_LOCAL;
        perror(___space___(Client: Send));
        return;
    } // if
}
