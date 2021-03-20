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
extern struct IPInfo info;

/** -------------------------------------------
 *  Client Functions
 * -------------------------------------------- */
void construct_packet_client(struct Message, packet_t type, char * finalPacket);
sock_t login(int nargs, char ** args, enum state *);
void logout(int nargs, char ** args, enum state *);
void join_session(int nargs, char ** args, enum state *);
void leave_session(int nargs, char ** args, enum state *);
void create_session(int nargs, char ** args, enum state *);
void list(int nargs, char ** args, enum state *);
void quit(int nargs, char ** args, enum state *);
void send_text(int nargs, char ** args, enum state *);
void print_menu();



#endif
