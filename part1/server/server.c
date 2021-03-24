/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
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

#include "constants.h"
#include "server.h"
#include "address_functions.h"

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

// Client register // 
struct registeredClients registeredClientList[5];

	// First client who registered // 
		registeredClientList[0].clientID = harris;
		registeredClientList[0].password = ecestudent;

	// Second client who registered //
		registeredClientList[1].clientID = zaheer;
		registeredClientList[1].password = ecestudent;

	// Third client who registered //
		registeredClientList[2].clientID = hashmi;
		registeredClientList[2].password = ecestudent;

	// Fourth client who registered // 
		registeredClientList[3].clientID = zheng;
		registeredClientList[3].password = ecestudent;

	// Fifth client who registered //  
		registeredClientList[4].clientID = prof;
		registeredClientList[4].password = eceprof;



struct IPInfo info;
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
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
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

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
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);

                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
						// Got data from client: Process data //
						message_processing(buf,i,remoteaddr);
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}



int message_processing(char* message, int clientFD, struct sockaddr_storage remoteaddr){

	// Loading up message struct // 
	struct Message packetStruct = malloc(sizeof(Message));
	deconstruct_packet(packetStruct,message);

	// Handling Table Functions // 

	if(packetStruct.type == LOGIN){
		login_handler(packetStruct,clientFD, remoteaddr);
	}

	else if(packetStruct.type == EXIT){
		exit_handler();
	}

	else if (packetStruct.type == JOIN){
		join_handler();
	}

	else if (packerStruct.type == LEAVE_SESS){
		leavesess_handler();
	}

	else if (packetStruct.type == NEW_SESS){
		newsess_handler();
	}

	else if (packetStruct.type == MESSAGE){
		message_handler();
	}

	else if(packetStruct.type == QUERY){
		query_handler();
	}

}

void login_handler(struct Message packetStruct,int clientFD, struct sockaddr_storage remoteaddr){
// Parse out the client ID and password from the Messange and Compare with entries in the client register //
	// If found in the table, check status flag
	// If status = active send LO_NACK
	// If status = not active proceed
	// bind server to client and create an entry for the client and to list // 
	// Send LO_ACK to the client // 
// If not matching then send LO_NACK to the client //

    int i;

    char* clientID;
    char* password;
    char* token;
    struct message responseMessage = malloc(sizeof(message));

    // Obtaining the clientID and password from the message sent by the client //
    for(i = 0; i < 2 ; i++){
        token = strsep(&receivedPacket, ",");
        if (token == NULL) break;
        
        if (i == 0)
            strcpy(clientID, token);
        else if (i == 1)
            strcpy(password, token);

    }

    // Checking for clientID and password in the register

    for(i= 0; i < 5; i++) {
        if(!(strcmp(clientID,registeredClientList[i].clientID) 
        && strcmp(password,registeredClientList[i].password)) ){
            if(registeredClientList[i].activeStatus == 1){
                printf("Client: %s has alreay logged in \n",registeredClientList[i].clientID);

                // Send a NACK to the client //
                responseMessage.type = "LO_NACK";
                responseMessage.data = "You have already logged in \n";
                responseMessage.size = sizeof(responseMessage.data);
                responseMessage.source = clientID
                 
                if (send(clientFD, "LO_NACK", sizeof("LO_NACK"), 0) == -1) {
                                        perror("send");
            }
                // Client was not active // 
                else{
                    registeredClientList[i].activeStatus = 1; // Client now active //

                }
        }
    }
}
