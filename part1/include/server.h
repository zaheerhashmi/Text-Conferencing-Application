#ifndef _SERVER_H_
#define _SERVER_H
#include "rooms.h"_

extern struct IPInfo info;

struct registeredClients {
    char* clientID;
    char* password;
    int activeStatus;
    int sessionID;
    
}

// Functions for message processing // 
int message_processing(char* message, int clientFD);
void loginHandler(struct Message packetStruct,int clientFD, struct sockaddr_storage remoteaddr);


#endif
