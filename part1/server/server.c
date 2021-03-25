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

#include "constants.h"
#include "server.h"
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

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
                               exit_handler(i, &master);
                            }
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
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
    // printf(___space___(This is the message that we are receiving: %s), message);
	// Loading up message struct // 
	struct Message packetStruct;
	deconstruct_packet(&packetStruct,message);

	// Handling Table Functions // 

	if(atoi(packetStruct.type) == LOGIN){
		login_handler(&packetStruct,clientFD,remoteaddr,master);
	}

	else if(atoi(packetStruct.type) == EXIT){
		exit_handler(clientFD,master);
	}

	else if (atoi(packetStruct.type) == JOIN){
		join_handler(&packetStruct,clientFD,master);
	}

	else if (atoi(packetStruct.type) == LEAVE_SESS){
		leavesess_handler(clientFD,master);
	}

	else if (atoi(packetStruct.type) == NEW_SESS){
		newsess_handler(&packetStruct,clientFD,master);
	}

	else if (atoi(packetStruct.type) == MESSAGE){
		message_handler(&packetStruct,clientFD);
	}

	else if(atoi(packetStruct.type) == QUERY){
		query_handler(clientFD, master);
	}
    pthread_mutex_unlock(&mutex);

}

/////////////////////////////////////////////////// HELPER FUNCTIONS /////////////////////////////////////////////////

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
                    exit_handler(clientFD, master);
                }

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

                        
                    }

                    exit_handler(clientFD, master);
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
        exit_handler(clientFD, master);
    }

}

void exit_handler(int clientFD, fd_set* master){
    int i;
    // Close the socket and remove it from the master set //
    close(clientFD);
    FD_CLR(clientFD,master);

    // Find the the client in register and update // 
    for(i =0; i<5; i++){
    if(registeredClientList[i].portNumber == clientFD && registeredClientList[i].activeStatus == 1){
        registeredClientList[i].activeStatus = 0;
        registeredClientList[i].portNumber = 0;
        memset(&registeredClientList[i].clientIP[0], 0, sizeof(registeredClientList[i].clientIP));
        memset(&registeredClientList[i].sessionID[0], 0, sizeof(registeredClientList[i].sessionID));
        return;
        }
    }
    return;
}


void newsess_handler(struct Message * packetStruct, int clientFD, fd_set* master){
    // Only logged in clients can create a new session and they must'nt be already in a session // 
    int i = 0, j = 0, numSimilar = 0;
    int temp;
    struct Message responseMessage;

    for(i =0; i<5; i++){
        if (clientFD == registeredClientList[i].portNumber && registeredClientList[i].activeStatus == 1){
            strcpy(registeredClientList[i].sessionID, packetStruct -> data);
            // find 
            char * check_dup_pointer = (char *)malloc(MAXBUFLEN);
            for (j = 0; j<5; j++){
                if (j == i || registeredClientList[j].sessionID[0] == '\0') continue;
                // Take the pointer from our records
                strcpy(check_dup_pointer, registeredClientList[j].sessionID);
                
                /* Without _ (pure match)*/
                printf(___space___(Our Session ID: %s \n Their Session ID %s), registeredClientList[i].sessionID, registeredClientList[j].sessionID);
                if(!strcmp(registeredClientList[i].sessionID, registeredClientList[j].sessionID)){
                    numSimilar++;
                    printf(___space___(Pure Match. num Similar +1 to %d), numSimilar);
                    continue;
                }


                char * token = strsep(&check_dup_pointer, "_");
                printf("This is the token: %s", token);
                if (token == NULL) continue;

                /* If one our records partially matches their string, we accept */
                if(!strcmp(token, registeredClientList[i].sessionID)){
                    numSimilar++;
                    printf(___space___(Partial Match. num Similar +1 to %d), numSimilar);
                }
            }
            printf(___space___(Num Similar: %d), numSimilar);
            if (numSimilar != 0){
                char * tempNumber = (char *)malloc(MAXBUFLEN);
                strcat(registeredClientList[i].sessionID, "_");
                sprintf(tempNumber, "%d", numSimilar);
                strcat(registeredClientList[i].sessionID, tempNumber);
            }
            temp = i;
        
            // Send Acknowledgement of creation of a new session // 
            sprintf(responseMessage.type,"%d",NS_ACK);
            strcpy(responseMessage.data, registeredClientList[temp].sessionID); // Session ID
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
            return;
        }
    }
    
    // We have a malicious client if we don't find the clientFD in the records. //
    close(clientFD);
    FD_CLR(clientFD,master);
    return;
}

void leavesess_handler(int clientFD, fd_set* master){
    // Check client isnt malicious // 
    int i= 0;
    
    for(i =0; i<5; i++){
        if(clientFD == registeredClientList[i].portNumber && registeredClientList[i].activeStatus == 1){
            memset(&registeredClientList[i].sessionID[0], 0, sizeof(registeredClientList[i].sessionID)); // does not belong to any session now //
            return;
        }
    }

    // Malicious Client // 
    close(clientFD);
    FD_CLR(clientFD,master);
    return;
}

void join_handler(struct Message* packetStruct,int clientFD,fd_set* master){

    int i= 0;
    int j =0;
    char* sessionID = (char *) malloc(MAXBUFLEN); // For some reason, copying sessionID into other strings doesn't work.
    int validSession;
    bool alreadyInSession = false;
    struct Message responseMessage;

// Ensure that the client is logged in // 
    for(i =0; i<5; i++){
        if(clientFD == registeredClientList[i].portNumber && registeredClientList[i].activeStatus == 1){
            // Session already exists for the current user
            if (registeredClientList[i].sessionID[0] != '\0'){
                validSession = 0;
                alreadyInSession = true;
                break;
            }
            // if logged in check for valid session id
            for(j=0; j<5; j++){

                if(!strcmp(registeredClientList[j].sessionID, packetStruct->data)){
                    validSession = 1;
                    break;
                }
            }
            break; // forgot this break
        }
    } 
    if(validSession){
        
        // Send an ACK for session joined // 
        // printf(___space___(Copying session ID to %s), registeredClientList[i].clientID);
        strcpy(registeredClientList[i].sessionID, packetStruct -> data);
        sprintf(responseMessage.type,"%d",JN_ACK);
        strcpy(responseMessage.data, registeredClientList[i].sessionID);
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
        }
    }

    else{
        // Send a NACK //                 
        sprintf(responseMessage.type,"%d",JN_NAK);
        strcpy(responseMessage.data, packetStruct -> data);
        strcat(responseMessage.data, ",");
        strcat(responseMessage.data, (alreadyInSession) ? "Already in Session": "Invalid Session ID");
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
            exit_handler(clientFD, master);
        }

        
    }
}


void message_handler(struct Message* packetStruct,int clientFD){
    // Storing the message // 
    char* buf = packetStruct->data;
    char * ack = (char *) malloc(MAXBUFLEN);
    int i = 0;
    char * sessionID = (char *) malloc(MAXBUFLEN);
    struct Message response;
    for(i = 0; i < 5; i ++ ){
        if(registeredClientList[i].portNumber == clientFD && registeredClientList[i].activeStatus == 1 ){
            strcpy(sessionID, registeredClientList[i].sessionID);
            break;
        }
    }
    
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


    /* Debug Notes: Make sure sessionID is right, and make sure the port numbers are right*/
    for(i = 0; i < 5; i ++ ){
        /** 
         * CHANGE LOG:
         * It's ok to message yourself, you are important too XD
         * registeredClientList[i].portNumber != clientFD
         * **/
        if((!strcmp(registeredClientList[i].sessionID, sessionID)) && (registeredClientList[i].activeStatus == 1)){
                if (send(registeredClientList[i].portNumber, ack, MAXBUFLEN, 0) == -1) {
                    perror("send");
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
        }
    }

    strcpy(queryBuffer, "Here is a table of users and their sessions\n");
    for(j=0;j<5;j++){
        
        // Prepare the client and session listings //s
        if (registeredClientList[j].activeStatus == 1){
            strcat(queryBuffer, "\t");
            strcat(queryBuffer, "User -> ");
            strcat(queryBuffer,registeredClientList[j].clientID);
            strcat(queryBuffer," , ");
            strcat(queryBuffer, "Session -> ");
            strcat(queryBuffer,registeredClientList[j].sessionID);
            strcat(queryBuffer," \n");
            if (registeredClientList[j].sessionID[0] != '\0'){
                strcat(sessionList, registeredClientList[j].sessionID);
                strcat(sessionList, ",");
            }
        }
    }
    strcat(queryBuffer, "\n\n");
    strcat(queryBuffer, "Here is a list of all user sessions -> ");
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
        exit_handler(clientFD, master);
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
