/*
   CSC 425 Assignment 1

Authors: Chris Chapline and Helen Jones

Due Date: Wednesday, February 24th

sproxy.c -- Connects to the telnet daemon and listens for the cproxy connection
*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>	//for closing the connection
#include <stdlib.h>
#include <strings.h>
#include <errno.h>

#define TELNET_PORT 23
#define BUFFER_SIZE 1024
#define MAX_PENDING 5

void connect_to_telnet(int socket, int *connection) {
    /* Connect to the telnet daemon on localhost:23 */
    struct hostent *hp = gethostbyname("localhost");

    if(!hp) {
        fprintf(stderr, "ERROR: Unknown host! (lolwut)\n");
        exit(errno);
    }


    // Create client_addr data structure and copy over address
    struct sockaddr_in telnet_client_addr;
    bzero((char *)&telnet_client_addr, sizeof(telnet_client_addr));
    telnet_client_addr.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&telnet_client_addr.sin_addr, hp->h_length);
    telnet_client_addr.sin_port = htons(TELNET_PORT);

    // Open connection to telnet
    *connection = connect(socket, (struct sockaddr *)&telnet_client_addr, sizeof(telnet_client_addr));

    if(*connection == -1){
        fprintf(stderr, "ERROR: Connecting to telnet failed!\n");
        close(*connection);
        exit(errno);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Error: No port specified\n");
        exit(1);
    }

    int listen_port = atoi(argv[1]);

    // Connnect to telnet
    int telnet_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(telnet_sock == -1) {
        fprintf(stderr, "ERROR: Could not create socket for telnet!\n");
        exit(errno);
    }

    int telnet_conn = -1;
    connect_to_telnet(telnet_sock, &telnet_conn);

    // Setup the socket to listen for cproxy
    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(listen_sock == -1) {
        fprintf(stderr, "ERROR: Could not create socket for cproxy!\n");
        close(telnet_sock);
        exit(errno);
    }

    // Setup the sockaddr for the listen socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET; //assign the address family of the server
    server_addr.sin_port = htons(listen_port); // assign server's listen
    server_addr.sin_addr.s_addr = INADDR_ANY; //assign the server's ip

    //Bind the socket to an address using bind() system call
    int bindfd = bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(bindfd == -1){
        fprintf(stderr, "Error: Binding cproxy listen socket failed\n");
        close(listen_sock);
        close(telnet_sock);
        exit(errno);
    }

    //Listen for connections with listen()
    int listen_rt = listen(listen_sock, MAX_PENDING);
    if(listen_rt == -1){
        fprintf(stderr, "Error: Listen call failed\n");
        close(listen_sock);
        close(telnet_sock);
        exit(errno);
    }

    // Accept client connection
    socklen_t len;
    int cproxy_connection = accept(listen_sock, (struct sockaddr *)&server_addr, &len);

    if(cproxy_connection < 0){
        fprintf(stderr, "Error: connection accept failed\n");
        close(listen_sock);
        close(telnet_sock);
        exit(errno);
    } else {
        printf("Accepted connection :D\n");
    }

    // Set up our descriptor set for select
    fd_set socket_fds;
    FD_ZERO(&socket_fds);
    FD_SET(telnet_sock, &socket_fds);
    FD_SET(cproxy_connection, &socket_fds);

    // Set up the arguments
    int max_fd = (telnet_sock > cproxy_connection) ? telnet_sock : cproxy_connection;
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 500000;
    int rv;

    // Create the buffer
    char buf[BUFFER_SIZE];

    // Length of the payload recieved
    int payload_length = -1;

    // Actually forward the data
    while(true) {
        rv = select(max_fd + 1, &socket_fds, NULL, NULL, &tv);

        // Determine the value of rv
        if(rv == -1) {
            // This means that an error occured
            fprintf(stderr, "ERROR: Issue while selecting socket\n");
            close(telnet_sock);
            close(cproxy_connection);
            close(listen_sock);
            exit(errno);
        } else if(rv == 0) {
            // Timeout: This is not an issue in our program
        } else {
            // Determine which socket (or both) has data waiting
            if(FD_ISSET(telnet_sock, &socket_fds)) {
                // Recieve from telnet daemon
                payload_length = read(telnet_sock, &buf, BUFFER_SIZE);

                if(payload_length <= 0) {
                    close(cproxy_connection);
                    close(listen_sock);
                    close(telnet_sock);
                    break;
                }

                printf("Received %d bytes from telnet daemon\n", payload_length);

                // Write to the client
                send(cproxy_connection, (void *) buf, payload_length, 0);
            }

            if(FD_ISSET(cproxy_connection, &socket_fds)) {
                // Recieve from the client
                payload_length = read(cproxy_connection, &buf, BUFFER_SIZE);

                if(payload_length <= 0) {
                    close(cproxy_connection);
                    close(listen_sock);
                    close(telnet_sock);
                    break;
                }

                printf("Received %d bytes from the client\n", payload_length);


                // Write to the telnet connection
                send(telnet_sock, (void *) buf, payload_length, 0);
            }
        }
    }

    // Close our connections
    close(listen_sock);
    close(cproxy_connection);
    close(telnet_sock);
}
