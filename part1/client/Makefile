CC	= gcc
CFLAGS = -pthread -g -I../include ## -g for debugger, -I for adding path to include. This -I path is only "remembered" by the output executable, 
## and not actually added to the cpp include paths..

CLIENT_OBJFILES = client.o client_functions.o address_functions.o
CLIENT = client
VPATH=../network ## Any path under the VPATH, whose paths are delimited by colons, are searched for cpp files that match .o files
all: $(CLIENT)

$(CLIENT) : $(CLIENT_OBJFILES)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_OBJFILES)

run: 
	(client)

clean:
	rm -f $(CLIENT_OBJFILES) $(CLIENT) *~
