CC = gcc
CFLAGS = -g
CFLAGS += -Wall
CFLAGS += -std=gnu99

all: sproxy cproxy

sproxy: sproxy.o protocol.o
	${CC} ${CFLAGS} -o sproxy sproxy.o protocol.o

cproxy: cproxy.o protocol.o
	${CC} ${CFLAGS} -o cproxy cproxy.o protocol.o

# Generic Object file pattern
%.o: %.c %.h
	${CC} ${CFLAGS} -c $^

# Lists the todos found in the project
todo:
	grep -ni TODO *.c

# Remove object files and executables
clean:
	$(RM) cproxy sproxy *.o
