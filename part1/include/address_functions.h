#include "constants.h" // ensures constants is defined in this environment
#ifndef _NETWORK_FUNCTIONS_H_
#define _NETWORK_FUNCTIONS_H_

/** ----------------------------
 * Basic Network Information
 * --------------------------- */
struct IPInfo {
    char hostname[INPUT_LENGTH];
    char IP[INPUT_LENGTH];
};

struct Message {
    char type[INPUT_LENGTH];
    char size[INPUT_LENGTH];
    unsigned char source[INPUT_LENGTH];
    unsigned char data[MAXBUFLEN];
};
extern struct IPInfo info;

/** ------------------------------
 * All possible types of packets
 * ------------------------------ */
enum packet_type{
    LOGIN,
    LO_ACK,
    LO_NAK,
    EXIT,
    JOIN,
    JN_ACK,
    JN_NAK,
    LEAVE_SESS,
    LS_ACK,
    LS_NACK,
    NEW_SESS,
    NS_ACK,
    NS_NACK,
    MESSAGE,
    QUERY,
    QU_ACK
};

/** -------------------------------------------
 * Get IP and hostname of the current computer
 * ------------------------------------------- */  
void get_ip(struct IPInfo *);
void init_hints(struct addrinfo *);

/** ---------------------------------------------------------------------
 *  General Functions for Packet Construction and Packet Send-and-Receives
 * ----------------------------------------------------------------------*/
void deconstruct_packet(struct Message *, char * receivedPacket);
void acknowledge(struct Message packetStruct);
// int send_n_receive(sock_t sockfd, char * finalPacket);
int setup_tcp(char * server_ip, char * server_port);

#endif
