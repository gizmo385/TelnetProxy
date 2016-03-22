
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
#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <unistd.h>	//for closing the connection
#include <arpa/inet.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>

#include "list.h"

#define MAX_PENDING 5

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

char *get_ip_address() {
    // Open up a dummy socket
    int temp_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Get the IP for a particular interface (eth1)
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth1", IFNAMSIZ - 1);
    ioctl(temp_fd, SIOCGIFADDR, &ifr);

    // Store the ip as a string
    char *ip = inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr);

    // Close our dummy port
    close(temp_fd);

    return ip;
}

void disconnect_reconnect(int *server_sock, int listen, int client, char *host, int port) {
    // Close the current connection
    printf("Disconnect detected.\n");
    close(*server_sock);
    *server_sock = -1;

    // Re-establish the connection
    printf("Attempting to re-establish connection\n");
    *server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(*server_sock == -1) {
        fprintf(stderr, "ERROR: Could not create socket for server connection!\n");
        close(*server_sock);
        close(listen);
        close(client);
        exit(errno);
    }

    set_socket_opts(*server_sock);
    int telnet_conn = -1;
    connect_to_server(*server_sock, host, port, &telnet_conn);

    // Send a connection message to the server noting that it is a new session
    message_t *conn_message = new_conn_message(OLD_SESSION);
    send_message(*server_sock, conn_message);
    printf("Successfully reconnected\n");
}

int main(int argc, char *argv[]) {
    if(argc < 4) {
        fprintf(stderr, "Usage: ./cproxy listen_port server_ip server_port\n");
        exit(1);
    }

    int listen_port = atoi(argv[1]);
    char *server_hostname = argv[2];
    int server_port = atoi(argv[3]);

    // Setup the socket to listen for telnet connection
    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);

    if(listen_sock == -1) {
        fprintf(stderr, "ERROR: Could not create socket for telnet!\n");
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
        exit(errno);
    }

    //Listen for connections with listen()
    int listen_rt = listen(listen_sock, MAX_PENDING);
    if(listen_rt == -1){
        fprintf(stderr, "Error: Listen call failed\n");
        close(listen_sock);
        exit(errno);
    }
    printf("Waiting for client...\n");

    socklen_t len;
    int client_connection = -1;
    int server_sock = -1;

    // Set up our descriptor set for select
    fd_set socket_fds;

    // Set up the arguments
    int max_fd = -1;
    int rv;

    // Create the buffer
    char *buf = calloc(BUFFER_SIZE, sizeof(char));

    // Length of the payload recieved
    int payload_length = -1;

    // Buffer for messages
    list_t *message_buffer = new_list_t();

	// Accept client connection
	client_connection = accept(listen_sock, (struct sockaddr *)&listen_addr, &len);

	if(client_connection < 0){
		fprintf(stderr, "Error: connection accept failed\n");
		close(listen_sock);
		close(server_sock);
		close(client_connection);
		exit(errno);
	} else if(client_connection == 0){
		fprintf(stderr, "Client connection messed up\n");
		close(listen_sock);
		close(server_sock);
		close(client_connection);
		exit(errno);
	} else {
		// Connnect to sproxy when we have a client
		server_sock = socket(PF_INET, SOCK_STREAM, 0);
		if(server_sock == -1) {
			fprintf(stderr, "ERROR: Could not create socket for server connection!\n");
			close(listen_sock);
			close(server_sock);
			exit(errno);
		}
	}

	set_socket_opts(server_sock);
	int telnet_conn = -1;
	connect_to_server(server_sock, server_hostname, server_port, &telnet_conn);

	// Send a connection message to the server noting that it is a new session
	message_t *conn_message = new_conn_message(NEW_SESSION);
	send_message(server_sock, conn_message);

    struct timeval last_heartbeat_sent;
    struct timeval last_heartbeat_recieved;

	// Actually forward the data
    while(true) {
        bzero(buf, BUFFER_SIZE);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        FD_ZERO(&socket_fds);

        if(server_sock > 0) {
            FD_SET(server_sock, &socket_fds);
        }

        if(client_connection > 0) {
            FD_SET(client_connection, &socket_fds);
            max_fd = (server_sock > client_connection) ? server_sock : client_connection;
        } 
        
		rv = select(max_fd + 1, &socket_fds, NULL, NULL, &timeout);

        // Determine the value of rv
        if(rv == -1) {
            // This means that an error occured
            fprintf(stderr, "ERROR: Issue while selecting socket\n");
            close(server_sock);
            close(client_connection);
            close(listen_sock);
            exit(errno);
        } else if(rv == 0) {
            // Timeout: Send a heartbeat to the server
            if(server_sock > 0) {
                printf("Sending heartbeat to server (select timeout).\n");
                gettimeofday(&last_heartbeat_sent, NULL);
                message_t *heartbeat = new_heartbeat_message();
                send_message(server_sock, heartbeat);

                // If it's been more than 3 seconds, we've timed out
                if((last_heartbeat_sent.tv_sec - last_heartbeat_recieved.tv_sec >= 3)) {
                    disconnect_reconnect(&server_sock, listen_sock, client_connection,
                            server_hostname, server_port);
                }
            }
        } else {
            // Send a heartbeat if it's been at least 1 second
            struct timeval current;
            gettimeofday(&current, NULL);

            if((current.tv_sec - last_heartbeat_sent.tv_sec >= 1)) {
                gettimeofday(&last_heartbeat_sent, NULL);
                message_t *heartbeat = new_heartbeat_message();
                send_message(server_sock, heartbeat);
            }

            // If server_sock has a message, then the server has sent a message to the client
            if(FD_ISSET(server_sock, &socket_fds)) {
                // Receive from the server
                message_t *message = read_message(server_sock);

                if(!message) {
                    disconnect_reconnect(&server_sock, listen_sock, client_connection,
                            server_hostname, server_port);
                    continue;
                }

                switch(message->message_flag) {
                    case DATA_FLAG:
                        {
                            data_message_t *data = message->body->data;

                            if(client_connection < 0) {
                                list_t_add(message_buffer, message);
                            } else {
                                // Clear out the sent buffer if one is present
                                while(message_buffer->head) {
                                    message_t *queued = list_t_pop(message_buffer);
                                    data_message_t *data_in_queue = queued->body->data;
                                    send(client_connection, (void *) data_in_queue->payload,
                                            data_in_queue->message_size, 0);
                                }

                                // Send the current message
                                send(client_connection, (void *) data->payload, data->message_size, 0);
                            }

                            break;
                        }
                    default:
                        // TODO HANDLE ERROR
                        break;
                }
            }

            // If client_connection has a message, then client is sending something to the server
            if(client_connection >= 0 && FD_ISSET(client_connection, &socket_fds)) {
                // Recieve from the client
                payload_length = read(client_connection, buf, BUFFER_SIZE);

                if(payload_length <= 0) {
                    printf("Read payload from client_connection <= 0\n");
                    close(client_connection);
                    close(listen_sock);
                    close(server_sock);
                    break;
                }

                if((strcmp(buf, "exit") == 0) || (strcmp(buf, "logout") == 0)) {
                    printf("Exiting.\n");
                    close(client_connection);
                    close(listen_sock);
                    close(server_sock);
                    break;
                }

                // Write to the server
                message_t *message = new_data_message(0, 0, payload_length, buf);
                send_message(server_sock, message);
            }
        }
    }

    // Close our connections
    close(client_connection);
    close(listen_sock);
    close(server_sock);
}
