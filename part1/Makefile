## ------------------------------------------------------------------------------------------------------
##  CLIENT != client, because the subdirectory confuses its client option with the root's client option,
##  and it ends up running root's client instead of its own client.
## ------------------------------------------------------------------------------------------------------
CLIENT=the_client 
SERVER=the_server
all: $(CLIENT) $(SERVER)

$(CLIENT):
	(cd client && $(MAKE))

$(SERVER):
	(cd server && $(MAKE))
# clean:
# 	(cd client && $(MAKE) clean)