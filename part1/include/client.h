#include "constants.h"
#include "address_functions.h"
#ifndef _CLIENT_H_
#define _CLIENT_H_

enum state {
    ON_LOCAL,
    ON_SERVER,
    IN_SESSION
};

/**
 *  The client number to be shared globally
 */
extern char my_username[INPUT_LENGTH];
extern char current_session[INPUT_LENGTH];
extern struct IPInfo info;
extern pthread_mutex_t mutex;

/** -------------------------------------------
 *  Client Functions
 * -------------------------------------------- */
void receive_loop(sock_t sockfd, enum state *); // this needs to be activated in session 
int send_n_receive(sock_t sockfd, char * finalPacket, char * message, enum state *);
void construct_packet_client(struct Message, packet_t type, char * username, char * finalPacket);
sock_t login(int nargs, char ** args, sock_t, enum state *);
void logout(int nargs, char ** args, sock_t, enum state *);
void join_session(int nargs, char ** args, sock_t, enum state *);
void leave_session(int nargs, char ** args, sock_t, enum state *);
void create_session(int nargs, char ** args, sock_t, enum state *);
void list(int nargs, char ** args, sock_t, enum state *);
void quit(int nargs, char ** args, sock_t, enum state *);
void send_text(char * text, sock_t, enum state *);
void print_menu();
void print_short_menu();



#endif
