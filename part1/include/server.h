#ifndef _SERVER_H_
#define _SERVER_H
#include "rooms.h"
#include "address_functions.h"
#include "constants.h"

struct clientSessionID { 
    char sessionID[MAXBUFLEN];
    bool blockMessages;
    struct clientSessionID * next;
};
/**
 * New struct for storing multiple session IDs
 */ 
struct registeredClients {
    char clientID[MAXBUFLEN];
    char password[MAXBUFLEN];
    int activeStatus;
    struct clientSessionID * sessionList;
    int portNumber;
    char clientIP[MAXBUFLEN];
};
extern struct registeredClients registeredClientList[5];
extern struct IPInfo info;
// struct registeredClients {
//     char clientID[MAXBUFLEN];
//     char password[MAXBUFLEN];
//     int activeStatus;
//     char sessionID[MAXBUFLEN];
//     int portNumber;
//     char clientIP[MAXBUFLEN];
// };


// Functions for message processing // 
int get_active();
void message_processing(char* message, int clientFD, struct sockaddr_storage remoteaddr,fd_set* master, int fdmax, int listener);
void login_handler(struct Message * packetStruct,int clientFD, struct sockaddr_storage remoteaddr,fd_set* master);
void exit_handler(int clientFD,fd_set* master);
void newsess_handler(struct Message * packetStruct, int clientFD,fd_set* master);
void leavesess_handler(struct Message * packetStruct, int clientFD, fd_set* master);
void join_handler(struct Message * packetStruct,int clientFD,fd_set* master);
void message_handler(struct Message* packetStruct,int clientFD, fd_set* master);
void message_all_handler(struct Message * packetStruct, int clientFD, fd_set *master);
void query_handler(int clientFD, fd_set* master);
void history_handler(char * sessionID, int clientFD, fd_set* master);
void invite_handler (char* packetData, int clientFD, fd_set* master);
void send_invite(char* guest, char* inviter);

// Session ID functions //
void add_session_id(struct Message * packetStruct, int i);
int delete_session_id(struct Message * packetStruct, int i);
int look_for_sessionID(char * sessionID, int index);
void delete_all_session_ids(int i);
int count_sessions(int i);
char * list_sessions(int i);
#endif
