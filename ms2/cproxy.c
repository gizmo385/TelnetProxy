
/*
   CSC 425 Assignment 1

Authors: Chris Chapline and Helen Jones

Due Date: Wednesday, February 24th

cproxy.c -- Connects to the server proxy and listens for a telnet connection
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

#define MAX_PENDING 5
#define BUFFER_SIZE 1024

void set_socket_opts(int socket) {
    int enable = 1;
    if(setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        fprintf(stderr, "Error while attempting to set SO_REUSEADDR!\n");
        exit(errno);
    }

    if(setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        fprintf(stderr, "Error while attempting to set SO_REUSEPORT!\n");
        exit(errno);
    }
}

void connect_to_server(int socket, char *server_hostname, int server_port, int *connection) {
    /* Connect to the server on specified port */
    struct hostent *hp = gethostbyname(server_hostname);

    if(!hp) {
        fprintf(stderr, "ERROR: Unknown host! (lolwut)\n");
        exit(errno);
    }


    // Create client_addr data structure and copy over address
    struct sockaddr_in client_addr;
    bzero((char *)&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&client_addr.sin_addr, hp->h_length);
    client_addr.sin_port = htons(server_port);

    // Open connection to the server
    *connection = connect(socket, (struct sockaddr *)&client_addr, sizeof(client_addr));

    if(*connection == -1){
        fprintf(stderr, "ERROR: Connecting to sproxy failed!\n");
        exit(errno);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 4) {
        fprintf(stderr, "Usage: ./cproxy listen_port server_ip server_port\n");
        exit(1);
    }

    int listen_port = atoi(argv[1]);
    char *server_hostname = argv[2];
    int server_port = atoi(argv[3]);

    // Connnect to server
    int server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(server_sock == -1) {
        fprintf(stderr, "ERROR: Could not create socket for telnet!\n");
	close(server_sock);
        exit(errno);
    }

    set_socket_opts(server_sock);
    int telnet_conn = -1;
    connect_to_server(server_sock, server_hostname, server_port, &telnet_conn);

    // Setup the socket to listen for cproxy
    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);

    if(listen_sock == -1) {
        fprintf(stderr, "ERROR: Could not create socket for cproxy!\n");
        close(server_sock);
	close(listen_sock);
        exit(errno);
    }

    set_socket_opts(listen_sock);

    // Setup the sockaddr for the listen socket
    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET; //assign the address family of the server
    listen_addr.sin_port = htons(listen_port);
    listen_addr.sin_addr.s_addr = INADDR_ANY; //assign the server's ip

    //Bind the socket to an address using bind() system call
    int bindfd = bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr));
    if(bindfd == -1){
        fprintf(stderr, "Error: Binding cproxy listen socket failed\n");
        close(listen_sock);
        close(server_sock);
        exit(errno);
    }

    //Listen for connections with listen()
    int listen_rt = listen(listen_sock, MAX_PENDING);
    if(listen_rt == -1){
        fprintf(stderr, "Error: Listen call failed\n");
        close(listen_sock);
        close(server_sock);
        exit(errno);
    }

    // Accept client connection
    socklen_t len;
    int client_connection = accept(listen_sock, (struct sockaddr *)&listen_addr, &len);

    if(client_connection < 0){
        fprintf(stderr, "Error: connection accept failed\n");
        close(listen_sock);
        close(server_sock);
	close(client_connection);
        exit(errno);
    } else {
        printf("Accepted client! :D\n");
    }

    // Set up our descriptor set for select
    fd_set socket_fds;

    // Set up the arguments
    int max_fd = (server_sock > client_connection) ? server_sock : client_connection;
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
        FD_ZERO(&socket_fds);
        FD_SET(server_sock, &socket_fds);
        FD_SET(client_connection, &socket_fds);
        rv = select(max_fd + 1, &socket_fds, NULL, NULL, NULL);

        // Determine the value of rv
        if(rv == -1) {
            // This means that an error occured
            fprintf(stderr, "ERROR: Issue while selecting socket\n");
            close(server_sock);
            close(client_connection);
            close(listen_sock);
            exit(errno);
        } else if(rv == 0) {
            // Timeout: This is not an issue in our program
        } else {
            // Determine which socket (or both) has data waiting
            if(FD_ISSET(server_sock, &socket_fds)) {
                // Recieve from the server
                payload_length = read(server_sock, &buf, BUFFER_SIZE);

                if(payload_length <= 0) {
                    close(client_connection);
                    close(listen_sock);
                    close(server_sock);
                    break;
                }

                // Write to the client
                send(client_connection, (void *) buf, payload_length, 0);
            }

            if(FD_ISSET(client_connection, &socket_fds)) {
                // Recieve from the client
                payload_length = read(client_connection, &buf, BUFFER_SIZE);

                if(payload_length <= 0) {
                    close(client_connection);
                    close(listen_sock);
                    close(server_sock);
                    break;
                }

		if(strcmp(buf, "exit") == 0) {
                    close(client_connection);
                    close(listen_sock);
                    close(server_sock);
		    break;
		}

                // Write to the telnet connection (client)
                send(server_sock, (void *) buf, payload_length, 0);
            }
        }
    }

    // Close our connections
    close(client_connection);
    close(listen_sock);
    close(server_sock);
}
