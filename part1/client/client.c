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
#include <math.h>
#include <time.h>
#include <pthread.h>

#include "constants.h"
#include "client.h"
#include "address_functions.h"

struct IPInfo info;
enum state * clientState;
bool forked = false;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// void print_session(){
//     printf("\n");
//     printf("#--------------------------------------------------#");
//     printf("| You're in a session room! Make yourself at home! |");
//     printf("#--------------------------------------------------#");
//     printf("\n");
// }
void print_menu(){
    printf("\n");
    printf("Main Menu\n");
    printf("/login \t --- \t Log into the server at the given address and port \
    The IP address is specified in the string dot format\n");
    printf("/logout \t --- \t Exit the server\n");
    printf("/joinsession \t --- \t Join the conference session with the given session id\n");
    printf("/leavesession \t --- \t Leave the currently established session\n");
    printf("/createsession \t --- \t Create a new conference session and join it\n");
    printf("/list \t --- \t Get the list of the connected clients and available sessions\n");
    printf("/quit \t --- \t Terminate the program\n");
    printf("<text> \t --- \t Send a message to the current conference session. The message is sent after the new line\n");
    printf("\n");
    printf("To see a shorter version of the menu, press /s.");
    printf("\n");
    printf("To hide the menu, press /h");
    printf("\n");
}

void print_short_menu(){
    printf("\n");
    printf("/login /logout /joinsession /leavesession /createsession /list /quit <text>\n");
    printf("\n");
    printf("To see a longer version of the menu, press /s.");
    printf("\n");
    printf("To hide the menu, press /h");
    printf("\n");
}
void pad_space(char * input){
    char * copy = input;

    /* Move copy till we end */
    while (*copy == ' '){
        copy++;
    }

    while (true){
        *input = *copy;
        input++;
        copy++;
        if (*copy == '\0'){
            return;
        }
    } // while
} // pad_space

int get_command(char * userInput, char ** args, char ** command){
    int i;
    int tokenLength;
    char * token;
    for (i = 0 ; true; i++){
        token = strsep(&userInput, " ");
        if (token == NULL) break;
        if (i == 0){
            strcpy(*command, token);
            if (!strcmp(*command, "/send")){
                break;
            }
        } else {
            tokenLength = strlen(token);
            args[i] = (char *) malloc((tokenLength + 1));
            strcpy(args[i], token);
        } // if 
    } // for

    // Custom argument building for send
    if (!strcmp(*command, "/send")){
        for (i = 1 ; true; i++){
            token = strsep(&userInput, " ");
            if (token == NULL) return MIN(i, 3); // 3 arguments 
            if (i == 1){
                args[1] = (char *) malloc(MAXBUFLEN/2);
                strcpy(args[1], token);
            } else if (i == 2){
                args[2] = (char *) malloc(MAXBUFLEN/2);
                strcpy(args[2], token);
            } else {
                strcat(args[2], " ");
                strcat(args[2], token);
            }
        } // for
    } // if

    return i; // returns the number of arguments
} // read_inputs

int main(int argc, char *argv[]){
    clientState = malloc(sizeof(enum state));
    *clientState = ON_LOCAL;
    int i;
    int sockfd = -1;
    bool shortVersion = false, hide = true;
    pthread_t tid;

    if (argc > 1){
        fprintf(stderr, "Usage: client\n");
        exit(1);
    }

    

    /* Wait for user input*/
    char * text = (char *) malloc(INPUT_LENGTH * sizeof(char));
    char * userInput = (char *) malloc(INPUT_LENGTH * sizeof(char));
    char * commandOfInterest = (char *)malloc(INPUT_LENGTH * sizeof(char));
    char ** args = (char **)malloc(sizeof(char *));
    get_ip(&info);

    bool REINPUT = true;
    while (REINPUT){
        if (!hide){
            if (!shortVersion) print_menu(); else print_short_menu();
            printf(___space___(The client is running on %s at %s), info.hostname, info.IP);
        }

        fgets(userInput, INPUT_LENGTH, stdin);
        if (userInput == NULL){
            perror("Command unsuccessful"); // empty string read
            exit(0);
        }

        pad_space(userInput);
        // printf("This is the translated string: %s", userInput);

        size_t length = strlen(userInput);

        /* fgets workaround. Sometimes \n is read at the end of buffer, so we wanna replace it with \0*/
        if (userInput[length - 1] == '\n') {
            userInput[length - 1] = '\0';
        }
        
        /**
         * Strsep will mess up the original string in get_command,
         * so we want to save a copy of it if we just want to send a simple text
         */
        strcpy(text, userInput); 
        int nargs = get_command(userInput, args, &commandOfInterest);
        // We will add an args check if time allots

        if(!strcmp("/login", commandOfInterest)){
            sockfd = login(nargs, args, sockfd);
            // We only fork when we send login request successfully
            if (sockfd != -1){
                if(pthread_create(&tid, NULL, &receive_loop, (void *)&sockfd) == -1){
                    perror(___space___(Uh oh));
                    exit(0);
                }
            }
        } else if (!strcmp("/logout", commandOfInterest)){
            logout(nargs, args, sockfd);
        } else if (!strcmp("/joinsession", commandOfInterest)){ 
            join_session(nargs, args, sockfd);
        } else if (!strcmp("/leavesession", commandOfInterest)){
            leave_session(nargs, args, sockfd);
        } else if (!strcmp("/createsession", commandOfInterest)){
            create_session(nargs, args, sockfd);
        } else if (!strcmp("/list", commandOfInterest)){
            list(nargs, args, sockfd);
        } else if (!strcmp("/quit", commandOfInterest)){
            quit(nargs, args, sockfd);
        } else if (!strcmp("/s", commandOfInterest)){
            shortVersion = !shortVersion;
        } else if (!strcmp("/status", commandOfInterest)){
            printf(___space___(Client State: %d), *clientState);
            printf("Current my_username: %s\n", (my_username[0] == '\0') ? "NULL" : my_username);
            printf("Socket file descriptor: %d \n\n", sockfd);
        } else if (!strcmp("/h", commandOfInterest)){
            hide = !hide;
        } else if (!strcmp("/send", commandOfInterest)){
            send_text(nargs, args, sockfd);
        } else if (!strcmp("/history", commandOfInterest)){
            get_history(nargs, args, sockfd);
        } else {
            if (*(userInput) == '/'){
                printf("Invalid command, please try again\n");
            } else {
                send_text_all(userInput, sockfd);
            }
        }

        /** We will just remove some garbage pointers now**/
        for (i = 0; i < 30; i++){
            args[i] = NULL;
        }
    } // while
    return 0;    
}

// /*
// ** client.c -- a stream socket client demo
// */

// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <errno.h>
// #include <string.h>
// #include <netdb.h>
// #include <sys/types.h>
// #include <netinet/in.h>
// #include <sys/socket.h>

// #include <arpa/inet.h>

// #define PORT "3490" // the port client will be connecting to 

// #define MAXDATASIZE 100 // max number of bytes we can get at once 

// // get sockaddr, IPv4 or IPv6:
// void *get_in_addr(struct sockaddr *sa)
// {
// 	if (sa->sa_family == AF_INET) {
// 		return &(((struct sockaddr_in*)sa)->sin_addr);
// 	}

// 	return &(((struct sockaddr_in6*)sa)->sin6_addr);
// }

// int main(int argc, char *argv[])
// {
// 	int sockfd, numbytes;  
// 	char buf[MAXDATASIZE];
// 	struct addrinfo hints, *servinfo, *p;
// 	int rv;
// 	char s[INET6_ADDRSTRLEN];

// 	if (argc != 2) {
// 	    fprintf(stderr,"usage: client hostname\n");
// 	    exit(1);
// 	}

// 	memset(&hints, 0, sizeof hints);
// 	hints.ai_family = AF_UNSPEC;
// 	hints.ai_socktype = SOCK_STREAM;

// 	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
// 		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
// 		return 1;
// 	}

// 	// loop through all the results and connect to the first we can
// 	for(p = servinfo; p != NULL; p = p->ai_next) {
// 		if ((sockfd = socket(p->ai_family, p->ai_socktype,
// 				p->ai_protocol)) == -1) {
// 			perror("client: socket");
// 			continue;
// 		}

// 		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
// 			perror("client: connect");
// 			close(sockfd);
// 			continue;
// 		}

// 		break;
// 	}

// 	if (p == NULL) {
// 		fprintf(stderr, "client: failed to connect\n");
// 		return 2;
// 	}

// 	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
// 			s, sizeof s);
// 	printf("client: connecting to %s\n", s);

// 	freeaddrinfo(servinfo); // all done with this structure

// 	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
// 	    perror("recv");
// 	    exit(1);
// 	}

// 	buf[numbytes] = '\0';

// 	printf("client: received '%s'\n",buf);

// 	close(sockfd);

// 	return 0;
// }
