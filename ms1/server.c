//Helen's Server machine is vm60
//Helen's Socket machine is vm61

/*
CSC 425 Assignment 1 -- Create a simple client and server application
	that can send messages. Server prints out the length of the message
	and the message recieved.

Authors: Chris Chapline and Helen Jones

Due Date: Wednesday, February 10

Server.c -- creates an open server that waits for connections
*/

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>	//for closing the connection
#include <stdlib.h>
#include <strings.h>

#define MAX_LINE 1024
#define MAX_PENDING 5

int main(int argc, char* argv[]){

	if(argc < 2){
		fprintf(stderr, "Error: no port specified\n");
		exit(1);
	}

	struct sockaddr_in my_addr;
	char buf[MAX_LINE];
	int len;			//length of the read for the recieve call

	bzero((char *)&my_addr, sizeof(my_addr));

	//assign the address family of the server
	my_addr.sin_family = AF_INET;

	//convert the port to the network byte order specified
	int port = atoi(argv[1]);
	my_addr.sin_port = htons(port);

	//assign the server's ip
	my_addr.sin_addr.s_addr = INADDR_ANY;
		
	// Create a socket with the socket() system call
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(1);
	}

	//Bind the socket to an address using bind() system call
	int bindfd = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
	if(bindfd == -1){
		fprintf(stderr, "Error: Binding failed\n");
		exit(1);
	}

	//Listen for connections with listen()
	int listenrt = listen(sockfd, MAX_PENDING);
	if(listenrt == -1){
		fprintf(stderr, "Error: Listen call failed\n");
		exit(1);
	}

	while(1){
		// Accept client connection
		/*printf("Before the connection\n");*/
		int connection = accept(sockfd, (struct sockaddr *)&my_addr, &len);
		if(connection < 0){
			fprintf(stderr, "Error: connection accept failed\n");
			exit(1);
		}

		int len = 0;
		int payload_size = 0;

		while(true) {
		    bzero((char *)&buf, sizeof(buf));
		    len = read(connection, &payload_size, sizeof(int));

		    if(len <= 0) {
			close(connection);
			break;
		    }

		    payload_size = ntohl(payload_size);

		    len = read(connection, &buf, payload_size);

		    if(len <= 0) {
			close(connection);
			break;
		    }
		    printf("%d\n", len); 
		    printf("%s", buf);
		}
	}
	close(sockfd);
	return 0;
}
