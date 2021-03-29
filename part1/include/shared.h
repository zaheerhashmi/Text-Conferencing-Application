#ifndef _SHARED_H_
#define _SHARED_H_
void read_from_file(FILE *f, char ** retval);
void write_to_file(struct Message * responseMessage, char * sessionID);
void gettime(char * timeVal);

#endif