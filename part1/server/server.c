/*
* selectserver.c -- a cheezy multiperson chat server
* Base Code: https://beej.us/guide/bgnet/
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>

#include "constants.h"
#include "server.h"
#include "shared.h"
#include "address_functions.h"

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char remoteIP[INET6_ADDRSTRLEN];
// Client register // 
struct registeredClients registeredClientList[5];
struct IPInfo info;
// Room number counter: Represents ID of a room number // 
int roomNumbers;
// get sockaddr, IPv4 or IPv6:

void pad_space(char * input){
    char * copy = input;

    /* Move copy till we end */
    while (*copy == ' '){
        copy++;
    }

    while (true){
        *input = *copy;
        input++;
        copy++;
        if (*copy == '\0'){
            return;
        }
    } // while
} // pad_space
void gettime(char * retval){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(retval, "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}
void read_from_file(FILE *f, char ** retval){
    int length;
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    fread (*retval, 1, length, f);
}
void write_to_file(struct Message * responseMessage, char * sessionID){
    char filename[INPUT_LENGTH];
    char data[MAXBUFLEN/2];
    strcpy(filename, sessionID);
    strcat(filename, ".txt");
    FILE * file;
    file = fopen(filename, "a");
    strcpy(data, "");
    strcat(data, responseMessage->source);
    strcat(data, "-> ");
    strcat(data, responseMessage->data);
    strcat(data, "\n");
    fwrite(data, 1, strlen(data), file);
    fclose(file);
}

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* https://stackoverflow.com/questions/27908441/remove-duplicated-words-in-string-in-c-program */
void RemoveDuplicates(char *resentence){
    char *temp1 = malloc(100);
    char *temp = NULL;

    *temp1=0;
    temp = strtok(resentence, ",");

    if (temp != NULL){// && strstr(temp1, temp) == NULL)
        strcpy(temp1, temp);
        while ((temp = strtok(NULL, ",")) != NULL){
            if (strstr(temp1, temp) == NULL){
                strcat(temp1, ",");
                strcat(temp1, temp);
            }
        }
    }
    strcpy(resentence, temp1);
    puts(resentence);
    free(temp1);
}

int main(char argc, char *argv[])
{        

    if (argc != 2){
        printf("Usage: server <port number to bind to>\n");
        exit(1);
    }
    get_ip(&info);
    // Room id counter //
    roomNumbers = 0;
    // First client who registered // 

    strcpy(registeredClientList[0].clientID,"harris");
	strcpy(registeredClientList[0].password,"ecestudent");

	// Second client who registered //
	strcpy(registeredClientList[1].clientID,"zaheer");
    strcpy(registeredClientList[1].password,"ecestudent");

	// Third client who registered //
	strcpy(registeredClientList[2].clientID,"hashmi");
	strcpy (registeredClientList[2].password,"ecestudent");

	// Fourth client who registered // 
	 strcpy(registeredClientList[3].clientID,"zheng");
	 strcpy(registeredClientList[3].password ,"ecestudent");

	// Fifth client who registered //  
	 strcpy(registeredClientList[4].clientID ,"prof");
	 strcpy(registeredClientList[4].password,"eceprof");

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[MAXBUFLEN];    // buffer for client data
    int nbytes;

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;
    int numConnections = 0;

    struct addrinfo hints, *ai, *p;

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        exit(1);
    }

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    // Flag for we found a good socket //
    int socketFound = 0;
    char* socketNumber = argv[1];
    int socketCounter = atoi(argv[1])+(1024 - atoi(argv[1]));

while(socketFound == 0){
    if ((rv = getaddrinfo(NULL, socketNumber, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        printf("selectserver: failed to bind to port %s \n",socketNumber);
        // Increase socket Counter // 
        socketCounter++;
        // Copy the socketCounter to socketNumber // 
        sprintf(socketNumber,"%d",socketCounter);

        // exit(2); // Because we couldn't find the port we wanted we will loop through until we find one //

    }
    // P non null -> we are now binded // 
    else{
        // set socketFound = 1 // 
        printf("Bound to port %s\n",socketNumber);
        socketFound = 1;
    }
}

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 0) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
    printf(___space___(The server is running on %s at %s on Port %s), info.hostname, info.IP, socketNumber);
    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {    
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);
                    
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, MAXBUFLEN, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                            
                        } else {
                            perror("recv");
                        }

                        /* On error, we make client inactive. */
                        for (j = 0; j < 5; j++){
                            if (registeredClientList[j].portNumber == i){
                               exit_handler(i, &master, true);
                               break;
                            }
                        }
                        // close(i); // bye!
                    } else {
						// Got data from client: Process data //
                        // printf("About to do message handling \n");
						message_processing(buf,i,remoteaddr,&master,fdmax,listener);
                        
                    }
                    printf(___space___(The server is running on %s at %s on Port %s), info.hostname, info.IP, socketNumber);
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    pthread_mutex_destroy(&mutex);
    return 0;
}


int get_active(){
    int i;
    int counter = 0;
    for (i = 0; i < 5; i++){
        if(registeredClientList[i].activeStatus == 1){
            counter++;
        } 
    }
    return counter;
}

void message_processing(char* message, int clientFD, struct sockaddr_storage remoteaddr,fd_set* master, int fdmax, int listener){
    pthread_mutex_lock(&mutex);
    printf(___space___(This is the message that we are receiving at the server: %s), message);
	// Loading up message struct // 
	struct Message packetStruct;
	deconstruct_packet(&packetStruct,message);

	// Handling Table Functions // 

	if(atoi(packetStruct.type) == LOGIN){
		login_handler(&packetStruct,clientFD,remoteaddr,master);
	}

	else if(atoi(packetStruct.type) == EXIT){
		exit_handler(clientFD,master,false);
	}

	else if (atoi(packetStruct.type) == JOIN){
		join_handler(&packetStruct,clientFD,master);
	}

	else if (atoi(packetStruct.type) == LEAVE_SESS){
		leavesess_handler(&packetStruct, clientFD,master);
	}

	else if (atoi(packetStruct.type) == NEW_SESS){
		newsess_handler(&packetStruct,clientFD,master);
	}

	else if (atoi(packetStruct.type) == MESSAGE){
		message_handler(&packetStruct,clientFD, master);
	}

    else if (atoi(packetStruct.type) == MESSAGE_ALL){
        message_all_handler(&packetStruct, clientFD, master);
    }

	else if(atoi(packetStruct.type) == QUERY){
		query_handler(clientFD, master);   
	}
    
    else if(atoi(packetStruct.type) == HISTORY){
        history_handler((packetStruct.data), clientFD, master);
    } else if (atoi(packetStruct.type) == TEST){

        struct Message responseMessage;
        // Send Acknowledgement of creation of a new session // 
        sprintf(responseMessage.type,"%d", INVITEE);
        strcpy(responseMessage.data, "turtle"); // Session ID
        strcat(responseMessage.data, ",");
        strcat(responseMessage.data, "zaheer"); // User
        sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
        strcpy(responseMessage.source,"I don't care");

        char* acknowledgement = strcat(responseMessage.type,":");
        strcat(acknowledgement,responseMessage.size);
        strcat(acknowledgement,":");
        strcat(acknowledgement,responseMessage.source);
        strcat(acknowledgement,":");
        strcat(acknowledgement,responseMessage.data);
        printf(acknowledgement);
                    
        if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
            perror("send");
            // Done with this we can return now //
            return;
        }
    }

    else if (atoi(packetStruct.type) == INVITE){
        invite_handler((packetStruct.data),clientFD,master);
    }

    else if (atoi(packetStruct.type) == IN_ACK || atoi(packetStruct.type) == IN_NACK){
        forward_message(packetStruct, clientFD, master);
    }

    pthread_mutex_unlock(&mutex);

}

/////////////////////////////////////////////////// HELPER FUNCTIONS /////////////////////////////////////////////////

void forward_message(struct Message packetStruct, int clientFD, fd_set* master){
    int i, inviterPortNumber;
    char inviter[MAXBUFLEN], sessionID[MAXBUFLEN];
    char * temp = (char *)malloc(MAXBUFLEN * sizeof(char));
    strcpy(temp, packetStruct.data);
    for (i = 0; i < 2; i++){
        char * token = strsep(&temp, ",");
        if (token == NULL) break;
        if (i == 0){
            strcpy(inviter, token);
        } else if (i == 1){
            strcpy(sessionID, token);
        }
    }  

    // find inviter to forward invite accept/decline to
    for (i=0;i<5;i++){
        if (!strcmp(registeredClientList[i].clientID, inviter)){
            
            inviterPortNumber = registeredClientList[i].portNumber;
            
            // If session does not exist in inviter anymore, invite failed.
            // Tell user join session failed because room has been emptied.
            if (!look_for_sessionID(sessionID, i)){
                struct Message responseMessage;
                // Send Acknowledgement of creation of a new session // 
                sprintf(responseMessage.type,"%d", JN_INVITE_NACK);
                strcpy(responseMessage.data, sessionID); 
                sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
                strcpy(responseMessage.source,"I don't care");

                char* acknowledgement = strcat(responseMessage.type,":");
                strcat(acknowledgement,responseMessage.size);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.source);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.data);
                printf(acknowledgement);

                if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                    perror("send");
                    exit_handler(clientFD, master, true);
                    // Done with this we can return now //
                    return;
                }
            }
            break;
        }
    }

    
    char* acknowledgement = strcat(packetStruct.type,":");
    strcat(acknowledgement,packetStruct.size);
    strcat(acknowledgement,":");
    strcat(acknowledgement,packetStruct.source);
    strcat(acknowledgement,":");
    strcat(acknowledgement,packetStruct.data);
    printf("Forward Message: %s", acknowledgement);
                
    if (send(inviterPortNumber,acknowledgement, MAXBUFLEN, 0) == -1) {
        perror("send");
        exit_handler(clientFD, master, true);
        // Done with this we can return now //
        return;
    }

} // 
void send_invite(char* guest, char* inviter, char* sessionID){

    int i;
    int portNumber;

    // Obtain the socket of the guest // 

    for (i = 0; i < 5; i++){
        if(!(strcmp(guest,registeredClientList[i].clientID))){
            portNumber = registeredClientList[i].portNumber;
            break;
        }
    }

    struct Message responseMessage;
    sprintf(responseMessage.type,"%d",INVITEE);
    strcpy(responseMessage.data, "");
    strcat(responseMessage.data,inviter);
    strcat(responseMessage.data,",");
    strcat(responseMessage.data,sessionID);
    strcat(responseMessage.data,",");
    strcat(responseMessage.data,guest);
    sprintf(responseMessage.size,"%d",strlen(responseMessage.data));
    strcpy(responseMessage.source,"");

    char * invite = (char *)malloc(MAXBUFLEN);
    strcpy(invite, "");
    strcat(invite, responseMessage.type);
    strcat(invite, ":");
    strcat(invite,responseMessage.size);
    strcat(invite,":");
    strcat(invite,responseMessage.source);
    strcat(invite,":");
    strcat(invite,responseMessage.data);

printf("%s", invite);
if (send(portNumber,invite, MAXBUFLEN, 0) == -1) {
    perror("send");
    // exit_handler
    // Done with this we can return now //
    return;
}
}

void invite_handler(char* packetData, int clientFD, fd_set* master){
    
    // Error Checking: Check if the iniviter isActive && the sessionID exists//
    // If the above is true then check if the invitee isActive; if they are then send the inivitation//

    int i;
    int j;
    char* sessionID = (char *) malloc(MAXBUFLEN);
    char* guest = (char *) malloc(MAXBUFLEN);
    struct Message responseMessage;
    char* token;

    // Obtaining sessionID and guest name from the data sent by the iniviter // 
        // Obtaining the clientID and password from the message sent by the client //
    for(i = 0; i < 2 ; i++){
        token = strsep(&(packetData), ",");
        if (token == NULL) break;
        if (i == 0)
            strcpy(sessionID, token);
        else if (i == 1)
            strcpy(guest, token);
    } 

    // Loop through and ensure that the inviter isActive and session ID exists // 
        for (i=0;i<5;i++){
        if (registeredClientList[i].portNumber == clientFD && registeredClientList[i].activeStatus == 1){
            break;
        }
    }

    // Check if sessionID is valid // 
    int validSession = look_for_sessionID(sessionID,i);
    int activeGuest = 0;
    int guestAlreadyInSession = 0;

    // Check if guest is Active // 

    for(j=0; j<5; j++){
        if(!strcmp(guest,registeredClientList[j].clientID) && registeredClientList[j].activeStatus == 1){
            activeGuest = 1;
            if(look_for_sessionID(sessionID, j)){
                guestAlreadyInSession = 1;
                break;
            }
        }
    }

    // If sessionID is valid and guest isActive send invite to guest // 

    if (validSession && activeGuest ){
        char* inviter = registeredClientList[i].clientID;
        send_invite(guest,inviter,sessionID);
    }

    // Invalid sessionID -> Send IN_NACK to inviter with message "Invalid sessionID" // 
    else if(validSession == 0){
            sprintf(responseMessage.type,"%d",SEND_INVITE_NACK);
            strcpy(responseMessage.data, "Invalid sessionID"); // Session ID
            sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
            strcpy(responseMessage.source,registeredClientList[i].clientID);

            char* acknowledgement = strcat(responseMessage.type,":");
            strcat(acknowledgement,responseMessage.size);
            strcat(acknowledgement,":");
            strcat(acknowledgement,responseMessage.source);
            strcat(acknowledgement,":");
            strcat(acknowledgement,responseMessage.data);
            
            printf("%s", acknowledgement);
            if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                perror("send");
                // Done with this we can return now //
                return;
            }
    }

    // Inactive guest -> Send IN_NACK to inviter with message "Guest inactive"
    else if(guestAlreadyInSession == 1){

            sprintf(responseMessage.type,"%d",SEND_INVITE_NACK);
            strcpy(responseMessage.data, "Guest is already in session. No need to send invite."); // Session ID
            sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
            strcpy(responseMessage.source,registeredClientList[i].clientID);

            char* acknowledgement = strcat(responseMessage.type,":");
            strcat(acknowledgement,responseMessage.size);
            strcat(acknowledgement,":");
            strcat(acknowledgement,responseMessage.source);
            strcat(acknowledgement,":");
            strcat(acknowledgement,responseMessage.data);
            
            printf("%s", acknowledgement);
            if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                perror("send");
                // Done with this we can return now //
                return;
            }
    } 
    else if(activeGuest == 0){

            sprintf(responseMessage.type,"%d",SEND_INVITE_NACK);
            strcpy(responseMessage.data, "Guest is inactive or does not exist."); // Session ID
            sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
            strcpy(responseMessage.source,registeredClientList[i].clientID);

            char* acknowledgement = strcat(responseMessage.type,":");
            strcat(acknowledgement,responseMessage.size);
            strcat(acknowledgement,":");
            strcat(acknowledgement,responseMessage.source);
            strcat(acknowledgement,":");
            strcat(acknowledgement,responseMessage.data);
            
            printf("%s", acknowledgement);
            if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                perror("send");
                // Done with this we can return now //
                return;
            }
    } 


}






int history_handler(char * sessionID, int clientFD, fd_set* master){
    int i;
    for (i=0;i<5;i++){
        if (registeredClientList[i].portNumber == clientFD && registeredClientList[i].activeStatus == 1){
            break;
        }
    }

    /** -------------------------------------------------
    * If that room doesn't exist, we can't get its history. 
    * ---------------------------------------------------*/
    if (!look_for_sessionID(sessionID, i)){
        struct Message responseMessage;
        // Send Acknowledgement of creation of a new session // 
        sprintf(responseMessage.type,"%d",HISTORY_NACK);
        strcpy(responseMessage.data, sessionID); // Session ID
        strcat(responseMessage.data, "|");
        strcat(responseMessage.data, "Invalid Session ID");
        sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
        strcpy(responseMessage.source,registeredClientList[i].clientID);

        char* acknowledgement = strcat(responseMessage.type,":");
        strcat(acknowledgement,responseMessage.size);
        strcat(acknowledgement,":");
        strcat(acknowledgement,responseMessage.source);
        strcat(acknowledgement,":");
        strcat(acknowledgement,responseMessage.data);
        printf(acknowledgement);
                    
        if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
            perror("send");
            // Done with this we can return now //
            return -1;
        }
        return 0;
    }
    
    FILE *file;
    char filename[INPUT_LENGTH], chatroom_info[INPUT_LENGTH];
    strcpy(filename, sessionID);
    strcat(filename, ".txt");
    file = fopen(filename, "r");

    // file exists!
    char * fileContents = (char *)malloc(MAXBUFLEN);
    strcpy(chatroom_info, "");
    strcat(chatroom_info, sessionID);
    strcat(chatroom_info, "|"); // we should use semicolon delimiters here
    read_from_file(file, &fileContents);
    strcat(chatroom_info, fileContents);
    // free(localTime);
    // free(fileContents);
    
    struct Message responseMessage;
    // Send Acknowledgement of creation of a new session // 
    sprintf(responseMessage.type,"%d",HISTORY_ACK);
    strcpy(responseMessage.data, chatroom_info); // Session ID
    sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
    strcpy(responseMessage.source,registeredClientList[i].clientID);

    char* acknowledgement = strcat(responseMessage.type,":");
    strcat(acknowledgement,responseMessage.size);
    strcat(acknowledgement,":");
    strcat(acknowledgement,responseMessage.source);
    strcat(acknowledgement,":");
    strcat(acknowledgement,responseMessage.data);
    
    printf("%s", acknowledgement);
    if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
        perror("send");
        fclose(file);
        // Done with this we can return now //
        return -1;
    }
    fclose(file);
    return 0;
}
void login_handler(struct Message* packetStruct,int clientFD, struct sockaddr_storage remoteaddr,fd_set* master){
    // Parse out the client ID and password from the Messange and Compare with entries in the client register //
        // If found in the table, check status flag
        // If status = active send LO_NACK
        // If status = not active proceed
        // bind server to client and create an entry for the client and to list // 
        // Send LO_ACK to the client // 
    // If not matching then send LO_NACK to the client //

    int i;

    char* clientID = (char *) malloc(MAXBUFLEN);
    char* password = (char *) malloc(MAXBUFLEN);
    char* token;
    struct Message responseMessage;
    char* packetData = packetStruct->data;

    // Obtaining the clientID and password from the message sent by the client //
    for(i = 0; i < 2 ; i++){
        token = strsep(&(packetData), ",");
        if (token == NULL) break;
        if (i == 0)
            strcpy(clientID, token);
        else if (i == 1)
            strcpy(password, token);
    }

    // Checking for clientID and password in the register
    /** --------------------------------------
     * CHANGES LOG:
     *  - Used MAXBUFLEN in the send() function to send message, not sizeof the message
     *  sizeof could be calculating everything in bits. Just don't use it lol.
     *  - Do NOT use the returned string in strcat. Instead, use char* dest
     *    (first argument) to process it.
     * 
     *  - 
     * If we compared the client username with every client in
     * the records and we still have not found the client
     * Then we know the client username is invalid.
     * ---------------------------------------*/
    for(i= 0; i < 5; i++) {
        if(!strcmp(clientID,registeredClientList[i].clientID) && !strcmp(password, registeredClientList[i].password)){
            // Deal with registered or un-registered cases
            if(registeredClientList[i].activeStatus == 1){
                printf(___space___(Client: %s has already logged in),registeredClientList[i].clientID);

                // Send a NACK to the client //
                sprintf(responseMessage.type,"%d",LO_NAK);
                strcpy(responseMessage.data,"You have already logged in");
                sprintf(responseMessage.size,"%d",strlen(responseMessage.data));
                strcpy(responseMessage.source,registeredClientList[i].clientID);
                
                char * acknowledgement = (char *)malloc(MAXBUFLEN);
                strcpy(acknowledgement, "");
                strcat(acknowledgement, responseMessage.type);
                strcat(acknowledgement, ":");
                strcat(acknowledgement,responseMessage.size);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.source);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.data);
                 
                if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                    perror("send");
                    exit_handler(clientFD, master, true);
                    return;
                }
                exit_handler(clientFD, master, false);
                return;
                // Client was not active // 
            } else {
                //Too many clients //
                if (get_active() + 1 > 5){
                    printf(___space___(Connection refused: Too many connections));

                    // Send a NACK to the client //
                    sprintf(responseMessage.type,"%d",LO_NAK);
                    strcpy(responseMessage.data,"Connection refused: Too many connections.");
                    sprintf(responseMessage.size,"%d",strlen(responseMessage.data));
                    strcpy(responseMessage.source,registeredClientList[i].clientID);
                    
                    char * acknowledgement = (char *)malloc(MAXBUFLEN);
                    strcpy(acknowledgement, "");
                    strcat(acknowledgement, responseMessage.type);
                    strcat(acknowledgement, ":");
                    strcat(acknowledgement,responseMessage.size);
                    strcat(acknowledgement,":");
                    strcat(acknowledgement,responseMessage.source);
                    strcat(acknowledgement,":");
                    strcat(acknowledgement,responseMessage.data);
                    
                    if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                        perror("send");
                        exit_handler(clientFD, master, true);
                        return;
                    }

                    exit_handler(clientFD, master, false);
                    return; 
                }
                /** -------------------------------
                 *  We don't need to bind client
                 ---------------------------------- */
                // if ((bind(clientFD,(struct sockaddr*)&remoteaddr, sizeof(remoteaddr))) != 0) { 
                //     printf("socket bind failed...\n"); 
                //     exit(0); 
                // } 

                registeredClientList[i].activeStatus = 1; // Client now active //
                registeredClientList[i].portNumber = clientFD; // Client port //
                strcpy(registeredClientList[i].clientIP, strdup(inet_ntop(remoteaddr.ss_family,
                                                    get_in_addr((struct sockaddr*)&remoteaddr),
                                                    remoteIP, INET6_ADDRSTRLEN))); 

                // Send login acknowledgement // 
                sprintf(responseMessage.type,"%d",LO_ACK);
                strcpy(responseMessage.data,"Login Successful");
                sprintf(responseMessage.size,"%d",strlen(responseMessage.data));
                strcpy(responseMessage.source,registeredClientList[i].clientID);

                char * acknowledgement = (char *)malloc(MAXBUFLEN);
                strcpy(acknowledgement, "");
                strcat(acknowledgement, responseMessage.type);
                strcat(acknowledgement, ":");
                strcat(acknowledgement,responseMessage.size);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.source);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.data);
                 
                if (send(clientFD, acknowledgement, MAXBUFLEN, 0) == -1) {
                    perror("send");
                    exit_handler(clientFD, master, true);
                }

                return;
                            
            }
        }
    }

    sprintf(responseMessage.type,"%d",LO_NAK);
    strcpy(responseMessage.data,"Invalid Username or Password");
    sprintf(responseMessage.size,"%d", strlen(responseMessage.data));
    strcpy(responseMessage.source, clientID);

    char * acknowledgement = (char *) malloc(MAXBUFLEN);
    strcpy(acknowledgement, responseMessage.type);
    strcat(acknowledgement, ":");
    strcat(acknowledgement,responseMessage.size);
    strcat(acknowledgement,":");
    strcat(acknowledgement,responseMessage.source);
    strcat(acknowledgement,":");
    strcat(acknowledgement,responseMessage.data);

    // printf("This is the acknowledgement: %s", acknowledgement);
    if (send(clientFD, acknowledgement, MAXBUFLEN, 0) == -1) {
        perror("send");
        exit_handler(clientFD, master, true);
        return;
    }
    exit_handler(clientFD, master, false);

}

void exit_handler(int clientFD, fd_set* master, bool sock_dead){
    /** 
     * Remember to deal with broken pipe. If sock_dead == True, we don't write to any socket
     * **/
    
    int i,j;
    // Find the the client in register and update // 
    for(i=0; i<5; i++){
        if(registeredClientList[i].portNumber == clientFD && registeredClientList[i].activeStatus == 1){
            // Gather sessions for chat history
            char * sessions = list_sessions(i);
            char * temp = (char *)malloc(MAXBUFLEN * sizeof(char));
            strcpy(temp, sessions);
            // Download every single session file to local

            if (!sock_dead){
               for (j=0; true; j++){
                    char * session = strsep(&temp, ",");
                    pad_space(session);
                    if (session == NULL || strlen(session) == 0) break;
                    history_handler(session, clientFD, master);
                }
            }
            // Delete session IDS
            delete_all_session_ids(i);

            // Delete histories
            for (j=0; true; j++){
                char * session = strsep(&sessions, ",");
                pad_space(session);
                if (session == NULL || strlen(session) == 0) break;
                delete_history_if_session_doesnt_exist(session);
            }
            
            registeredClientList[i].activeStatus = 0;
            registeredClientList[i].portNumber = 0;
            memset(&registeredClientList[i].clientIP[0], 0, sizeof(registeredClientList[i].clientIP));
            if (!sock_dead){
                struct Message responseMessage;
                // ACKNOWLEDGE logout
                // Send Acknowledgement of creation of a new session // 
                sprintf(responseMessage.type,"%d", LOGOUT_ACK);
                strcpy(responseMessage.data, ""); // Session ID
                sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
                strcpy(responseMessage.source,registeredClientList[i].clientID);

                char* acknowledgement = strcat(responseMessage.type,":");
                strcat(acknowledgement,responseMessage.size);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.source);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.data);
                            
                if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                    /** ------------------------------- 
                     * If we notice that the socket has abruptly quit,
                     * we need to also abruptly close the client socket.
                    */
                    
                    perror("send");
                    close(clientFD);
                    FD_CLR(clientFD,master);
                    // Done with this we can return now //
                    return;
                }
            }
            break;
        }
    }

    // Close the socket and remove it from the master set //
    close(clientFD);
    FD_CLR(clientFD,master);
    return;
}


void newsess_handler(struct Message * packetStruct, int clientFD, fd_set* master){
    // Only logged in clients can create a new session and they must'nt be already in a session // 
    int i = 0, j = 0, numSimilar = 0;
    int temp;
    struct Message responseMessage;
    FILE * file;

    for(i =0; i<5; i++){
        /** ------------------------------------
         * Desired Implementation:
         *  Create a new session name followed by 
         *  _<a number representing the number of rooms containing
         *  that name + 1>.
         * 
         * Current Lazy Implementation:
         *  YOU CAN'T CREATE A ROOM WITH THE SAME NAME
         * -------------------------------------- */
        if (clientFD == registeredClientList[i].portNumber && registeredClientList[i].activeStatus == 1){
            for (j = 0; j< 5; j++){

                // If session already exists
                if (look_for_sessionID(packetStruct -> data, j)){
                    // Send Acknowledgement of creation of a new session // 
                    sprintf(responseMessage.type,"%d",NS_NACK);
                    strcpy(responseMessage.data, packetStruct -> data); // Session ID
                    strcat(responseMessage.data, ",");
                    strcat(responseMessage.data, "Room Already Exists");
                    sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
                    strcpy(responseMessage.source,registeredClientList[i].clientID);

                    char* acknowledgement = strcat(responseMessage.type,":");
                    strcat(acknowledgement,responseMessage.size);
                    strcat(acknowledgement,":");
                    strcat(acknowledgement,responseMessage.source);
                    strcat(acknowledgement,":");
                    strcat(acknowledgement,responseMessage.data);
                                
                    if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                        perror("send");
                        // Done with this we can return now //
                        return;
                    }
                    return;
                }
            }
            temp = i;
        
            // Send Acknowledgement of creation of a new session // 
            sprintf(responseMessage.type,"%d",NS_ACK);
            strcpy(responseMessage.data, packetStruct -> data); // Session ID
            sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
            strcpy(responseMessage.source,registeredClientList[temp].clientID);

            char* acknowledgement = strcat(responseMessage.type,":");
            strcat(acknowledgement,responseMessage.size);
            strcat(acknowledgement,":");
            strcat(acknowledgement,responseMessage.source);
            strcat(acknowledgement,":");
            strcat(acknowledgement,responseMessage.data);
                        
            if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                perror("send");
                // Done with this we can return now //
                return;
            }

            // Add the sessionID to the user
            add_session_id(packetStruct, i);
            char filename[INPUT_LENGTH];
            strcpy(filename, packetStruct -> data);
            strcat(filename, ".txt");
            file = fopen(filename, "w"); // clears older files
            fclose(file);
            return;
        }
    }
    
    // We have a malicious client if we don't find the clientFD in the records. //
    close(clientFD);
    FD_CLR(clientFD,master);
    return;
}

void delete_history_if_session_doesnt_exist(char * sessionID){
    int j;
    bool sessionStillExists = false;

    // Remove text file if sessionID doesn't exist in any client  
    for (j = 0; j < 5; j++){
        if (look_for_sessionID(sessionID, j)){
            sessionStillExists = true;
            break;
        }
    } // for

    if (!sessionStillExists){
        char sessionFile[INPUT_LENGTH];
        sprintf(sessionFile, "%s.txt", sessionID);
        int del = remove(sessionFile);
        if (!del)
            printf("The file %s is Deleted successfully", sessionFile);
        else
            printf("the file %s is not Deleted", sessionFile);
        
    }

}
                

void leavesess_handler(struct Message * packetStruct, int clientFD, fd_set* master){
    // Check client isnt malicious // 
    int i = 0, j = 0;
    int err;
    struct Message responseMessage;
    char * clientID = (char *)malloc(MAXBUFLEN); 
    strcpy(clientID, registeredClientList[i].clientID);

    for(i =0; i<5; i++){
        if(clientFD == registeredClientList[i].portNumber && registeredClientList[i].activeStatus == 1){
            // download history and delete session if required before we leave
            if(look_for_sessionID(packetStruct -> data, i)){
                history_handler(packetStruct -> data, clientFD, master);
            }
            
            if(delete_session_id(packetStruct, i) == -1){
                // Unsuccessful // 
                sprintf(responseMessage.type,"%d",LS_NACK);
                strcpy(responseMessage.data, packetStruct -> data); // Session ID
                sprintf(responseMessage.size,"%d", strlen(responseMessage.data));
                strcpy(responseMessage.source, clientID);

                char* acknowledgement = strcat(responseMessage.type,":");
                strcat(acknowledgement,responseMessage.size);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.source);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.data);
                if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                    perror("send");
                }



            } else {
                // Successful //
                sprintf(responseMessage.type,"%d",LS_ACK);
                strcpy(responseMessage.data, packetStruct -> data); // Session ID
                strcat(responseMessage.data, ",");

                char * temp = (char *) malloc(INPUT_LENGTH);
                sprintf(temp, "%d", count_sessions(i));
                strcat(responseMessage.data, temp); // Number of rooms left
                sprintf(responseMessage.size,"%d", strlen(responseMessage.data));
                strcpy(responseMessage.source,clientID);

                char* acknowledgement = strcat(responseMessage.type,":");
                strcat(acknowledgement,responseMessage.size);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.source);
                strcat(acknowledgement,":");
                strcat(acknowledgement,responseMessage.data);
                if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                    perror("send");
                }          

                delete_history_if_session_doesnt_exist(packetStruct -> data);   

            }
            return;
            
        
        }
    }

    // Malicious Client // 
    close(clientFD);
    FD_CLR(clientFD,master);
    return;
}

void join_handler(struct Message* packetStruct,int clientFD,fd_set* master){
    /** ---------------------
     * CHANGE LOG:
     * 
     * Joinsession does not fail on 
     * joining multiple rooms anymore.
     * -------------------- */ 
    int i= 0;
    int j =0;
    char* sessionID = (char *) malloc(MAXBUFLEN); // For some reason, copying sessionID into other strings doesn't work.
    int validSession;
    bool emptyString = false;
    bool alreadyInSession = false;
    struct Message responseMessage;


// Ensure that the client is logged in // 
    for(i =0; i<5; i++){
        if(clientFD == registeredClientList[i].portNumber && registeredClientList[i].activeStatus == 1){
            // Check empty sessionID
            if (strlen(packetStruct -> data) == 0){
                validSession = 0;
                emptyString = true;
                break;
            }

            if(look_for_sessionID(packetStruct -> data, i)){
                validSession = 0;
                alreadyInSession = true;
                break;
            }
        

            // if logged in check that given session id given is valid
            for(j=0; j<5; j++){
                if(look_for_sessionID(packetStruct -> data, j)){
                    validSession = 1;
                    break;
                }
            }

            // we need an already in session check
            break; // forgot this break
        }
    } 
    if(validSession){
        
        // Send an ACK for session joined // 
        // printf(___space___(Copying session ID to %s), registeredClientList[i].clientID);
        sprintf(responseMessage.type,"%d",JN_ACK);
        strcpy(responseMessage.data, packetStruct -> data);
        sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
        strcpy(responseMessage.source,registeredClientList[i].clientID);


        char* acknowledgement = strcat(responseMessage.type,":");
        strcat(acknowledgement,responseMessage.size);
        strcat(acknowledgement,":");
        strcat(acknowledgement,responseMessage.source);
        strcat(acknowledgement,":");
        strcat(acknowledgement,responseMessage.data);
        
        if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
            perror("send");
            free(sessionID);
            return;
        }

        add_session_id(packetStruct, i);
    }

    else{
        // Send a NACK //                 
        sprintf(responseMessage.type,"%d",JN_NAK);
        strcpy(responseMessage.data, packetStruct -> data);
        strcat(responseMessage.data, ",");
        strcat(responseMessage.data, (emptyString) ? "Session name empty" : (alreadyInSession) ? "Already in that session" : "Invalid Session ID");
        sprintf(responseMessage.size,"%d",sizeof(responseMessage.data));
        strcpy(responseMessage.source, packetStruct -> source);

        char* acknowledgement = strcat(responseMessage.type,":");
        strcat(acknowledgement,responseMessage.size);
        strcat(acknowledgement,":");
        strcat(acknowledgement,responseMessage.source);
        strcat(acknowledgement,":");
        strcat(acknowledgement,responseMessage.data);
                        
        if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
            perror("send"); 
            free(sessionID); 
            exit_handler(clientFD, master, true);
            return;
        }

        
    }
}

void message_all_handler(struct Message * packetStruct, int clientFD, fd_set *master){
    /*For every sessionID in the current message sender, we look for it in the every client.*/
    int i, j;
    struct clientSessionID *curr;
    struct Message response;
    char ack[MAXBUFLEN], buf[MAXBUFLEN];
    /** Deconstruct the data field **/
    // temp loses reference here????

    for (i = 0; i < 5; i++){
        if(registeredClientList[i].portNumber == clientFD && registeredClientList[i].activeStatus == 1){
            strcpy(buf, packetStruct -> data);
            sprintf(response.type, "%d", MESSAGE);
            sprintf(response.size, "%d", strlen(buf));
            strcpy(response.source, registeredClientList[i].clientID);
            strcpy(response.data, buf);
            strcpy(ack, response.type);
            strcat(ack, ":");
            strcat(ack, response.size);
            strcat(ack, ":");
            strcat(ack, response.source);
            strcat(ack, ":");
            strcat(ack, response.data);
            curr = registeredClientList[i].sessionList;
            while (curr != NULL){
                write_to_file(&response, curr -> sessionID);
                curr = curr -> next;
            }
    
            for (j = 0; j < 5; j++){
                curr = registeredClientList[i].sessionList;
                while (curr != NULL){
                    char ack_with_session[MAXBUFLEN];
                    if (look_for_sessionID(curr -> sessionID, j)){
                        strcpy(ack_with_session, ack);
                        strcat(ack_with_session, "|");
                        strcat(ack_with_session, curr->sessionID);
                        if (send(registeredClientList[j].portNumber, ack_with_session, MAXBUFLEN, 0) == -1) {
                            perror("send");
                            exit_handler(clientFD, master, true);
                            return;
                        }

                    }
                    curr = curr -> next;
                } // while
            } // for j
            return;
        }
    } // for i
} // message_all_handler

void message_handler(struct Message* packetStruct,int clientFD, fd_set *master){
    // Storing the message // 
    int i, j;
    char * buf;
    char * sessionID_string;
    struct Message responseMessage;
    /** Deconstruct the data field **/
    char * temp = (char *) malloc(MAXBUFLEN/2);
    strcpy(temp, packetStruct -> data);
    
    for (i = 0; i < 2; i++){    
        char * token = strsep(&temp, ";");
        if (token == NULL) break;
        if (i == 0){
            sessionID_string = (char *)malloc(MAXBUFLEN/2);
            strcpy(sessionID_string, token);
        } else if (i == 1){
            buf = (char *)malloc(MAXBUFLEN/2);
            strcpy(buf, token);
        }
    }
    printf(buf);
    // temp loses reference here????

    temp = (char *) malloc(MAXBUFLEN/2);
    strcpy(temp, sessionID_string);
    for (i = 0; i < 5; i++){
        if (registeredClientList[i].portNumber == clientFD && registeredClientList[i].activeStatus == 1){
            /** Read every session ID **/
            for (j = 0; true; j++){
                char * sessionID = strsep(&temp, ",");
                if (sessionID == NULL) break;
                if (!look_for_sessionID(sessionID, i)){
                    // We are sending messages to a room that our user does not belong too! NACK!
                    sprintf(responseMessage.type,"%d", MSG_NACK);
                    strcpy(responseMessage.data, sessionID_string);
                    sprintf(responseMessage.size,"%d", strlen(responseMessage.data));
                    strcpy(responseMessage.source, packetStruct -> source);

                    char* acknowledgement = strcat(responseMessage.type,":");
                    strcat(acknowledgement,responseMessage.size);
                    strcat(acknowledgement,":");
                    strcat(acknowledgement,responseMessage.source);
                    strcat(acknowledgement,":");
                    strcat(acknowledgement,responseMessage.data);
                                    
                    if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
                        perror("send");  
                        exit_handler(clientFD, master, true);
                        return;
                    }
                    return;
                }
            } // for j
            break;
        } // for i
    }

    /** ------------------------------
     *  When we reached here,
     *  we have already confirmed
     *  that room is valid.
     * ------------------------------ **/
    struct Message response;
    char * ack = (char *) malloc(MAXBUFLEN);
    
    sprintf(response.type, "%d", MESSAGE);
    sprintf(response.size, "%d", strlen(buf));
    strcpy(response.source, registeredClientList[i].clientID);
    strcpy(response.data, buf);
    strcpy(ack, response.type);
    strcat(ack, ":");
    strcat(ack, response.size);
    strcat(ack, ":");
    strcat(ack, response.source);
    strcat(ack, ":");
    strcat(ack, response.data);

    /** Write to sessionID you are sending **/
    temp = (char *) malloc(MAXBUFLEN/2);
    strcpy(temp, sessionID_string);
    for (j = 0; true; j++){
        char * sessionID = strsep(&temp, ",");
        if (sessionID == NULL) break;
        if (look_for_sessionID(sessionID, i)){
            write_to_file(&response, sessionID);
        } 
    }

    /* Debug Notes: Make sure sessionID is right, and make sure the port numbers are right*/
    for(i = 0; i < 5; i ++ ){
        /** 
         * CHANGE LOG:
         * It's ok to message yourself, you are important too XD
         * registeredClientList[i].portNumber != clientFD
         * 
         * We are now going to find every receiver of interest sent by client and find it on other clients 
         * **/
        temp = (char *) malloc(MAXBUFLEN/2);
        strcpy(temp, sessionID_string);
        for (j = 0; true; j++){
            char * sessionID = strsep(&temp, ",");
            if (sessionID == NULL) break;
            
            if(look_for_sessionID(sessionID, i)){
                char ack_with_session[MAXBUFLEN];
                strcpy(ack_with_session, ack);
                strcat(ack_with_session, "|");
                strcat(ack_with_session, sessionID);
                if (send(registeredClientList[i].portNumber, ack_with_session, MAXBUFLEN, 0) == -1) {
                    perror("send");
                }

            }

        }
    }
}

void query_handler(int clientFD, fd_set* master){
    int i = 0, j = 0;
    char* clientID;
    char* queryBuffer = (char*)malloc(MAXBUFLEN);
    char* sessionList = (char*)malloc(MAXBUFLEN);
    struct Message queryResponse;

    for (i=0;i<5;i++){
        if (registeredClientList[i].portNumber == clientFD && registeredClientList[i].activeStatus == 1){
            clientID = registeredClientList[i].clientID;
            break;
        }
    }
    // i is our id

    strcpy(queryBuffer, "Here is a table of users and their sessions\n");
    for(j=0;j<5;j++){
        
        // Prepare the client and session listings //s
        if (registeredClientList[j].activeStatus == 1){
            strcat(queryBuffer, "\t");
            strcat(queryBuffer, "User -> ");
            strcat(queryBuffer,registeredClientList[j].clientID);
            strcat(queryBuffer," , ");
            strcat(queryBuffer, "Session -> ");
            if (strlen(list_sessions(j)) != 0){
                strcat(queryBuffer, list_sessions(j));
            }
            strcat(queryBuffer," \n");
            if (registeredClientList[j].sessionList != NULL){
                if (strlen(list_sessions(j)) != 0){
                    strcat(sessionList, list_sessions(j));
                }
            }
        }
    }
    strcat(queryBuffer, "\n\n");
    strcat(queryBuffer, "Here is a list of all user sessions -> ");
    RemoveDuplicates(sessionList);
    strcat(queryBuffer, sessionList);

    sprintf(queryResponse.type,"%d",QU_ACK);
    sprintf(queryResponse.size,"%d",strlen(queryBuffer));
    strcpy(queryResponse.source,clientID);
    strcpy(queryResponse.data,queryBuffer);

    // Prepare ack string // 
    char * acknowledgement = (char *)malloc(MAXBUFLEN);
    strcpy(acknowledgement, "");
    strcat(acknowledgement, queryResponse.type);
    strcat(acknowledgement, ":");
    strcat(acknowledgement,queryResponse.size);
    strcat(acknowledgement,":");
    strcat(acknowledgement,queryResponse.source);
    strcat(acknowledgement,":");
    strcat(acknowledgement,queryResponse.data);

    // Send the query message and acknowledgement // 
    printf("Ack %s", acknowledgement);
    if (send(clientFD,acknowledgement, MAXBUFLEN, 0) == -1) {
        perror("send");  
        exit_handler(clientFD, master, true);
    }

}


// #include <stdio.h> 
// #include <netdb.h> 
// #include <netinet/in.h> 
// #include <stdlib.h> 
// #include <string.h> 
// #include <sys/socket.h> 
// #include <sys/types.h> 
// #define MAX 80 
// #define PORT 8080 
// #define SA struct sockaddr 
  
// // Function designed for chat between client and server. 
// void func(int sockfd) 
// { 
//     char buff[MAX]; 
//     int n; 
//     // infinite loop for chat 
//     for (;;) { 
//         bzero(buff, MAX); 
  
//         // read the message from client and copy it in buffer 
//         read(sockfd, buff, sizeof(buff)); 
//         // print buffer which contains the client contents 
//         printf("From client: %s\t To client : ", buff); 
//         bzero(buff, MAX); 
//         n = 0; 
//         // copy server message in the buffer 
//         while ((buff[n++] = getchar()) != '\n') 
//             ; 
  
//         // and send that buffer to client 
//         write(sockfd, buff, sizeof(buff)); 
  
//         // if msg contains "Exit" then server exit and chat ended. 
//         if (strncmp("exit", buff, 4) == 0) { 
//             printf("Server Exit...\n"); 
//             break; 
//         } 
//     } 
// } 
  
// // Driver function 
// int main() 
// { 
//     int sockfd, connfd, len; 
//     struct sockaddr_in servaddr, cli; 
  
//     // socket create and verification 
//     sockfd = socket(AF_INET, SOCK_STREAM, 0); 
//     if (sockfd == -1) { 
//         printf("socket creation failed...\n"); 
//         exit(0); 
//     } 
//     else
//         printf("Socket successfully created..\n"); 
//     bzero(&servaddr, sizeof(servaddr)); 
  
//     // assign IP, PORT 
//     servaddr.sin_family = AF_INET; 
//     servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
//     servaddr.sin_port = htons(PORT); 
  
//     // Binding newly created socket to given IP and verification 
//     if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
//         printf("socket bind failed...\n"); 
//         exit(0); 
//     } 
//     else
//         printf("Socket successfully binded..\n"); 
  
//     // Now server is ready to listen and verification 
//     if ((listen(sockfd, 5)) != 0) { 
//         printf("Listen failed...\n"); 
//         exit(0); 
//     } 
//     else
//         printf("Server listening..\n"); 
//     len = sizeof(cli); 
  
//     // Accept the data packet from client and verification 
//     connfd = accept(sockfd, (SA*)&cli, &len); 
//     if (connfd < 0) { 
//         printf("server acccept failed...\n"); 
//         exit(0); 
//     } 
//     else
//         printf("server acccept the client...\n"); 
  
//     // Function for chatting between client and server 
//     func(connfd); 
  
//     // After chatting close the socket 
//     close(sockfd); 
// } 
