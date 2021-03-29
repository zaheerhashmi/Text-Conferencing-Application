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
extern enum state * clientState;
extern pthread_mutex_t mutex;
extern bool forked;

/** -------------------------------------------
 *  Client Functions
 * -------------------------------------------- */

void handle_return_message(char * message, sock_t); // The only way I can think of to make processing order of ACK messages not matter,
// Is to create an universal function to handle all returns and set variables based on the ACK message we've received.
void * receive_loop(void * sockfd); // this needs to be activated in session 
int send_data(sock_t sockfd, char * finalPacket); // doesn't not modify client state
// int send_n_receive(sock_t sockfd, char * finalPacket, char * message); // changes clientstate based on ack 
int receive();
void construct_packet_client(struct Message, packet_t type, char * username, char * finalPacket);
sock_t login(int nargs, char ** args, sock_t);
void logout(int nargs, char ** args, sock_t);
void join_session(int nargs, char ** args, sock_t);
void leave_session(int nargs, char ** args, sock_t);
void create_session(int nargs, char ** args, sock_t);
void list(int nargs, char ** args, sock_t);
void quit(int nargs, char ** args, sock_t);
void send_text(int nargs, char ** args, sock_t);
void get_history(int nargs, char ** args, sock_t);
void clear_server_context(sock_t sockfd);
void print_menu();
void print_short_menu();



#endif
