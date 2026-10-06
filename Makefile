CC = clang
CFLAGS = -Wall -g

all: server server_multi client

server: server.c
	$(CC) $(CFLAGS) -o server server.c

server_multi: server_multi.c
	$(CC) $(CFLAGS) -o server_multi server_multi.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server server_multi client
