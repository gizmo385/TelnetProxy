CFILES = sproxy.c cproxy.c
FILES = sproxy cproxy
CFLAGS = -g -o

all: sproxy cproxy

sproxy: sproxy.c
	gcc $(CFLAGS) sproxy sproxy.c

cproxy: cproxy.c
	gcc $(CFLAGS) cproxy cproxy.c

clean:
	$(RM) cproxy sproxy
