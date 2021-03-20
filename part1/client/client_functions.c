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

#include "client.h"
#include "network_functions.h"
#include "constants.h"

char my_username[INPUT_LENGTH];

void load_login_info(struct Message packetStruct, char * username, char * password){
    strcpy(packetStruct.data, "<");
    strcat(packetStruct.data, username);
    strcat(packetStruct.data, ":");
    strcat(packetStruct.data, password);
    strcat(packetStruct.data, ">");
    strcpy(my_username, username);
}


/* This function most likely has to return the socket that we want to access */
void login(int nargs, char ** argv, enum state * clientState){
    /**
     * Args: Client ID, Password, Server-IP, Server-Port
     * Description: Log into the server at the given address and port. The
     * IP address is specified in the string dot format.
     */
    if (nargs != 5){
        printf(___space___(Usage: /login <client ID> <password> <server-IP> <server-port>\n));
        return; 
    } 

   
    fd_set select_fds;
    int sockfd;
    double timeoutInterval = 10; // we start out at 10 seconds
    struct timeval timeout; 
     /* ^^^ Timeout Variables ^^^ */
    
    struct sockaddr_storage their_addr;
    int their_length, numbytes;
    char message[MAXBUFLEN + MAXBUFLEN/2 + 1];
    /* ^^^ Send and Receive Variables ^^^ */

    char finalPacket[MAXBUFLEN + MAXBUFLEN/2 + 1];
    /* ^^^ File/packet processing functions ^^^ */
    

    /**
     * TCP SERVER PORT BINDING AND THREE-WAY HANDSHAKE ARE DONE HERE
     */
    if ((sockfd = setup_tcp(argv[3], argv[4])) < 0){
        return -1;
    }
    
    /* Read necessary parameters and construct the finalPacket */
    struct Message messageInfo;
    load_login_info(messageInfo, argv[1], argv[2]); // load username and password into packet;
    construct_packet(messageInfo, LOGIN, finalPacket);

    /**
     * SEND DATA PACKET TO SERVER 
     */
    if ((numbytes = send(sockfd, finalPacket, strlen(finalPacket)+1, 0)) < 0){
        close(sockfd);
        perror("Client: send");
        exit(1);
    }
    
    /* Recv from server. If there is no response in 10s, close socket and quit. */
    FD_ZERO(&select_fds);
    FD_SET(sockfd, &select_fds);  
    timeout.tv_sec = (int) timeoutInterval;
    timeout.tv_usec = (int) ((timeoutInterval - timeout.tv_sec)*1000000);
    int selectSignal = select(sockfd + 1, &select_fds, NULL, NULL, &timeout); // Timeout of 10 seconds is setup for login.
    switch (selectSignal){
        case -1:
            perror("Client: Failed while waiting for server response.");
            close(sockfd);
            return -1;
        case 0:
            perror("Client has not received an ACK from the server in 10 seconds or less. Client timed out! Try Login again.");
            close(sockfd);
            return -1;
        default:
            if((numbytes = recv(sockfd, message, MAXBUFLEN, 0)) < 0){
                perror("Client: recv");
                close(sockfd);
                return -1;
            }

            if (strcmp)
            break;
    } // switch

    printf(___space___(Recv from Server %s: %s), argv[2], message);
    *clientState = ON_SERVER;



















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
//             perror("Could not allocate space for user input.");
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
//                         perror("talker: sendto");
//                         exit(1);
//                 }
//                 printf("\ntalker: sent the message to the listener in Network Byte Order\n");
//                 printf("talker: sent %d bytes to %s\n", bytesRead, argv[1]);

//                 printf("talker: waiting for recvfrom...\n");
//                 if ((numbytes = recvfrom(sockfd, message, MAXBUFLEN + MAXBUFLEN/2, 0, (struct sockaddr *)&their_addr, &their_length)) == -1){
//                     perror("talker: recvfrom"); // why MAXBUFLEN-1
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

void logout(int nargs, char ** args, enum state * clientState){


}

void join_session(int nargs, char ** args, enum state * clientState){
printf("Join Session");


}

void leave_session(int nargs, char ** args, enum state * clientState){
printf("Leave Session");


}

void create_session(int nargs, char ** args, enum state * clientState){
printf("Create Session");


}

void list(int nargs, char ** args, enum state * clientState){
printf("List");


}

void quit(int nargs, char ** args, enum state * clientState){
printf("Quit");


}

void send_text(int nargs, char ** args, enum state * clientState){
printf("Send Text");


}
