#ifndef _SERVER_H_
#define _SERVER_H
#include "rooms.h"
#include "address_functions.h"

extern struct IPInfo info;

struct registeredClients {
    char* clientID;
    char* password;
    int activeStatus;
    int sessionID;
    int portNumber;
    char* clientIP;
    };


// Functions for message processing // 
void message_processing(char* message, int clientFD, struct sockaddr_storage remoteaddr,fd_set* master, int fdmax, int listener);
void login_handler(struct Message* packetStruct,int clientFD, struct sockaddr_storage remoteaddr,fd_set* master);
void exit_handler(int clientFD,fd_set* master);
void newsess_handler(int clientFD,fd_set* master);
void leavesess_handler(int clientFD, fd_set* master);
void join_handler(struct Message* packetStruct,int clientFD,fd_set* master);
void message_handler(struct Message* packetStruct,int clientFD);
void query_handler();

#endif
