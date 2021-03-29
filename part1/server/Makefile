CC	= gcc
CFLAGS = -g -pthread -I../include 
SERVER_OBJFILES = address_functions.o server_functions.o server.o 
SERVER = server
VPATH=../network ## Any path under the VPATH, whose paths are delimited by colons, are searched for cpp files that match .o files
all: $(SERVER)

$(SERVER) : $(SERVER_OBJFILES)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_OBJFILES)

run:
	server

clean:
	rm -f $(SERVER_OBJFILES) $(SERVER) *~
