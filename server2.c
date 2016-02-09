#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_MESSAGE_SIZE 1024

int main(int argc, char *argv[]) {

    /* Create our socket */
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);

    if(socket_fd < 0) {
        fprintf(stderr, "An error occurred while creating the server socket! %d returned!\n",
                socket_fd);
        exit(1);
    }

    /* Bind our socket */
    struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htons(1024);

    bind(socket_fd, (struct sockaddr *) addr, sizeof(struct sockaddr_in));

    /* Listen for connections on the socket */
    listen(socket_fd, 5);

    int bytes_read = -1;
    int connection_fd = -1;
    int message_size = -1;

    char message_buffer[MAX_MESSAGE_SIZE];
    memset(&message_buffer, '0', MAX_MESSAGE_SIZE);

    while(true) {
        connection_fd = accept(socket_fd, NULL, NULL);

        if(connection_fd < 0) {
            fprintf(stderr, "Error establishing connection with incoming client!\n");
            continue;
        }

        // Read in the message size from the socket
        bytes_read = recv(connection_fd, &message_size, sizeof(int), 0);

        if(bytes_read != sizeof(int)) {
            fprintf(stderr, "Error while reading message size from client. Read %d bytes!\n",
                    bytes_read);
            continue;
        }

        message_size = ntohl(message_size);

        // Read in the message
        bytes_read = recv(connection_fd, message_buffer, message_size, 0);

        if(bytes_read != message_size) {
            fprintf(stderr, "Error while reading message! Read %d bytes instead of %d.\n",
                    bytes_read, message_size);
            continue;
        }

        printf("%s\n", message_buffer);
        memset(&message_buffer, '0', MAX_MESSAGE_SIZE);

        close(connection_fd);
    }
}
