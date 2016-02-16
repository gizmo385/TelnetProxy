/*
   CSC 425 Assignment 1

Authors: Chris Chapline and Helen Jones

Due Date: Wednesday, February 24th

Server.c -- creates an open server that waits for connections
*/

#include <stdio.h>
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
#define MAX_PENDING 5

void connect_to_telnet(int sockfd, int *connection) {
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

    // Create the socket
    /**sockfd = socket(PF_INET, SOCK_STREAM, 0);*/
    /*printf("Socket for telnet connection is: %d\n", *sockfd);*/
    /*if(*sockfd == -1){*/
        /*fprintf(stderr, "ERROR: Creating telnet socket failed\n");*/
        /*exit(errno);*/
    /*}*/

    // Open connection to telnet
    *connection = connect(sockfd, (struct sockaddr *)&telnet_client_addr, sizeof(telnet_client_addr));

    if(*connection == -1){
        fprintf(stderr, "ERROR: Connecting to telnet failed!\n");
        close(*connection);
        exit(errno);
    }
}

void setup_cproxy_connection(int *sockfd, struct sockaddr_in *server_addr, int listen_port) {
    // Setup the sockaddr for the listen socket
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr->sin_family = AF_INET; //assign the address family of the server
    server_addr->sin_addr.s_addr = INADDR_ANY; //assign the server's ip

    // Create a socket with the socket() system call
    *sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(*sockfd == -1){
        fprintf(stderr, "Error: Socket creation failed\n");
        close(*sockfd);
        exit(errno);
    }

    //Bind the socket to an address using bind() system call
    int bindfd = bind(*sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(bindfd == -1){
        fprintf(stderr, "Error: Binding failed\n");
        close(*sockfd);
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
    int telnet_conn = -1;
    connect_to_telnet(&telnet_sock, &telnet_conn);

    // Setup the socket to listen for cproxy
    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    bzero((char *)&server_addr, sizeof(server_addr));
    setup_cproxy_connection(&listen_sock, &server_addr, listen_port);

    //Listen for connections with listen()
    int listen_rt = listen(listen_sock, MAX_PENDING);
    if(listen_rt == -1){
        fprintf(stderr, "Error: Listen call failed\n");
        close(listen_sock);
        exit(errno);
    }

    // Accept client connection
    socklen_t len;
    int client_connection = accept(listen_sock, (struct sockaddr *)&server_addr, &len);
    if(client_connection < 0){
        fprintf(stderr, "Error: connection accept failed\n");
        close(listen_sock);
        exit(errno);
    }

    // Set up our descriptor set for select
    fd_set socket_fds;
    FD_ZERO(&socket_fds);
    FD_SET(telnet_sock, &socket_fds);
    FD_SET(listen_sock, &socket_fds);

    // Set up the arguments
    int max_fd = (telnet_sock > listen_sock) ? telnet_sock : listen_sock;
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 500000;
    int rv;

    // Actually forward the data
    while(true) {
        rv = select(max_fd + 1, &socket_fds, NULL, NULL, &tv);

        // Determine the value of rv
        if(rv == -1) {
            // This means that an error occured
            fprintf(stderr, "ERROR: Issue while selecting socket\n");
            close(telnet_sock);
            close(listen_sock);
            exit(errno);
        } else if(rv == 0) {
            // Timeout: This is not an issue in our program
        } else {
            // Determine which socket (or both) has data waiting
            if(FD_ISSET(telnet_sock, &socket_fds)) {
                // recv from telnet_sock, write to listen_sock
            }

            if(FD_ISSET(listen_sock, &socket_fds)) {
                // recv from listen_sock, write to telnet_sock
            }
        }
    }

    // Close our connections
    close(listen_sock);
    close(telnet_sock);
}
