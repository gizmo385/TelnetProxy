CC = gcc
CFLAGS = -g
CFLAGS += -Wall
CFLAGS += -std=gnu99

all: sproxy cproxy

sproxy: sproxy.o protocol.o
	${CC} ${CFLAGS} -o sproxy sproxy.o protocol.o

cproxy: cproxy.o protocol.o
	${CC} ${CFLAGS} -o cproxy cproxy.o protocol.o

%.o: %.c %.h
	${CC} ${CFLAGS} -c $^

clean:
	$(RM) cproxy sproxy
