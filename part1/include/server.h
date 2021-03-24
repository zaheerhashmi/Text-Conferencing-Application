#ifndef _SERVER_H_
#define _SERVER_H
#include "rooms.h"_

extern struct IPInfo info;

struct registeredClients {
    char* clientID;
    char* password;
    int activeStatus;
    int sessionID;
    int portNumber;
    char* clientIP;
    
}

// Functions for message processing // 
int message_processing(char* message, int clientFD);
void login_handler(struct Message packetStruct,int clientFD, struct sockaddr_storage remoteaddr,fd_set* master);
void exit_handler(int clientFD);


#endif
