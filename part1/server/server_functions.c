#include <assert.h>
#include <stdbool.h>
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
#include <pthread.h>
#include "server.h"

void add_session_id(struct Message * packetStruct, int i){
    /**
     * Create a new node, or create a new node and add it to the current session list.
     * We are assuming that the data would contain the new session ID 
     */  
    if (registeredClientList[i].sessionList == NULL){
        registeredClientList[i].sessionList = (struct clientSessionID *)malloc(sizeof(struct clientSessionID));
        registeredClientList[i].sessionList -> blockMessages = false;
        strcpy(registeredClientList[i].sessionList -> sessionID, packetStruct -> data);
        registeredClientList[i].sessionList -> next = NULL;
    } else {
        struct clientSessionID * lastNode;
        struct clientSessionID * curr;
        struct clientSessionID * prev;
        struct clientSessionID * newNode;
        
        curr = registeredClientList[i].sessionList;
        while (curr != NULL){
            prev = curr;
            curr = curr -> next;
        }

        lastNode = prev; // We found the last node in the sessionList. Now we append the new session to the
        // next of last node

        newNode = (struct clientSessionID *)malloc(sizeof(struct clientSessionID));
        strcpy(newNode -> sessionID, packetStruct -> data);
        newNode -> blockMessages = false;  
        newNode -> next = NULL;
        lastNode -> next = newNode;
    }
}

int delete_session_id(struct Message * packetStruct, int i){
    /** -----------------------------------------
     * Needs to be called when a requester leaves a session.
     * Our leave session protocol NEEDS TO CHANGE! 
     * It needs to now specify the sessionID which we need to leave
     * inside packetStruct -> data. 
     * ------------------------------------------- */
    struct clientSessionID * curr;
    struct clientSessionID * prev;
    curr = registeredClientList[i].sessionList;
    while (curr != NULL){
        if (!strcmp(curr -> sessionID, packetStruct -> data)){
            if (count_sessions(i) == 1){
                registeredClientList[i].sessionList = NULL; // idk why this is necessary 
                curr -> next = NULL;
                free(curr);
                curr = NULL;
            } else {
                prev -> next = curr -> next;
                curr -> next = NULL;
                free(curr);
                curr = NULL;
            }
            return 0;
        }    
        prev = curr;
        curr = curr -> next;
    }

    return -1; // sessionID does not exist
}

void delete_all_session_ids(int i){
    struct clientSessionID * curr;
    struct clientSessionID * next;
    curr = registeredClientList[i].sessionList;
    next = curr;
    
    /**
     * Go the next, and free the current one.
     */
    if (next != NULL){
        next = curr -> next;
        free(curr);
        curr = NULL;
    }

    registeredClientList[i].sessionList = NULL;
} 


/** Intended for send message **/
int look_for_sessionID(char * interested_sessionID, int index){
    /** ----------------------------
     * Find if the session ID exists for a particular client
     * -------------------------- */ 
    assert(index < 5);
    struct clientSessionID * curr;
    struct clientSessionID * prev;
    curr = registeredClientList[index].sessionList;
    while (curr != NULL){
        if (!strcmp(curr -> sessionID, interested_sessionID)){
            return 1;
        }    
        curr = curr -> next;
    }

    return 0;
}

char * list_sessions(int index){
    struct clientSessionID * curr;
    char * session_holder = (char *) malloc(MAXBUFLEN);
    curr = registeredClientList[index].sessionList;
    strcpy(session_holder, "");
    while (curr != NULL){
        strcat(session_holder, curr->sessionID);
        strcat(session_holder, ",");
        curr = curr->next;
    }
    printf("Session Holder: %s", session_holder);
    return session_holder;
} 

int count_sessions(int index){
    struct clientSessionID * curr;
    char * session_holder = (char *) malloc(MAXBUFLEN);
    curr = registeredClientList[index].sessionList;
    int count = 0;
    while (curr != NULL){       
        count++;
        curr = curr -> next;
    }

    return count;
}
    