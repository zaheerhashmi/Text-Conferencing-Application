/**
 *  Author: Harris Zheng
 *  Date: March 19th, 2021
 *  Description: Network functions that we may use in the client and server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>

// Our personal libraries 
#include "network_functions.h"
#include "constants.h"
#include "client.h"

void get_ip(struct IPInfo *info){
    struct hostent *host_entry;
    gethostname(info -> hostname, sizeof info -> hostname);
    host_entry = gethostbyname(info -> hostname); //find host information
    strcpy(info -> IP, inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]))); //Convert into IP string
}

void init_hints(struct addrinfo * hints){
    hints -> ai_family = AF_UNSPEC;
    hints -> ai_protocol = 0; // Socket address with any protocol can be returned by addrinfo
    hints -> ai_socktype = SOCK_STREAM;
    hints -> ai_flags = AI_NUMERICHOST; // Accept numbers-and-dots notation only
}

void construct_packet(struct Message packetStruct, packet_t type, char * finalPacket){
    /**
     * Args: message_struct, final_packet
     * Description: Process the packet so that it is ready to send.
     */   
    assert(finalPacket != NULL);
    strcpy(packetStruct.source, info.IP);
    strcat(packetStruct.source, my_username);
    sprintf(packetStruct.type, "%d", type);
    sprintf(packetStruct.size, "%d", strlen(packetStruct.data));
    
    strcpy(finalPacket, packetStruct.type);
    strcat(finalPacket, ":");
    strcat(finalPacket, packetStruct.size);
    strcat(finalPacket, ":");
    strcat(finalPacket, packetStruct.source);
    strcat(finalPacket, ":");
    strcat(finalPacket, packetStruct.data);
} 

void deconstruct_packet(struct Message packetStruct, char * receivedPacket){
    int i;
    int tokenLength;
    char * token;
    for (i = 0 ; i < 4; i++){
        token = strsep(&receivedPacket, ":");
        if (token == NULL) break;
        if (i == 0)
            strcpy(packetStruct.type, token);
        else if (i == 1)
            strcpy(packetStruct.size, token);
        else if (i == 2)
            strcpy(packetStruct.source, token);
        else if (i == 3)
            strcpy(packetStruct.data, token);
    } // for
} 

void acknowledge(struct Message packetStruct){
    
}

int setup_tcp(char * server_ip, char * server_port){
    /* Socket connection variables */
    struct addrinfo hints, *res, *p;
    int status, sockfd;
    memset(&hints, 0, sizeof hints);
    init_hints(&hints);
    /* ---------------------------
        Set up IP Address 
    ------------------------------*/
    if ((status = getaddrinfo(server_ip, server_port, &hints, &res)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    /* -------------------------------
        Link sockets.
        This is the client strategy to
        find an available server socket (IP Number)
    ---------------------------------- */
    for (p = res; p != NULL; p = p -> ai_next){
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
            perror("client: socket");
            continue; 
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL){
        fprintf(stderr, "talker: failed to connect\n");
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return sockfd;
}