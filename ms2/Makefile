CFILES = sproxy.c cproxy.c
FILES = sproxy cproxy
CFLAGS = -g -o

sproxy: sproxy.c
	gcc $(CFLAGS) sproxy sproxy.c

cproxy: cproxy.c
	gcc $(CFLAGS) cproxy cproxy.c

all: sproxy cproxy

clean:
	$(RM) cproxy sproxy
