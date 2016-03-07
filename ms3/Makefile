CFILES = sproxy.c cproxy.c
FILES = sproxy cproxy
CC = gcc
CFLAGS = -g
CFLAGS += -Wall
#CFLAGS += -w
CFLAGS += -DYYDEBUG=1
CFLAGS += -std=gnu99

all: sproxy cproxy

sproxy: sproxy.o protocol.h
	${CC} ${CFLAGS} -c protocol.h -o sproxy sproxy.o

cproxy: cproxy.o protocol.h
	${CC} ${CFLAGS} -c protocol.h -o cproxy cproxy.o

%.o: %.c %.h
	${CC} ${CFLAGS} -c $^

clean:
	$(RM) cproxy sproxy
