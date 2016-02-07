CFILES = server.c client.c
FILES = server client
CFLAGS = -g -Wall -o

server: server.c
	gcc $(CFLAGS) server server.c

client: client.c
	gcc $(CFLAGS) client client.c

all: $(CFILES)
	gcc $(CFLAGS) $(FILES) $(CFILES) 

clean:
	$(RM) client server
