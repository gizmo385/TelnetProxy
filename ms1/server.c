//Helen's Server machine is vm60
//Helen's Socket machine is vm61

#include <sys/types.h>
#include <sys.socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){

	if(argc < 2){
		fprintf(stderr, "Error: no port specified\n");
		exit(1);
	}

	struct sockaddr_in *my_addr = calloc(1, sizeof(sockaddr_in));

		// 1. Create a socket with the socket() system call
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(1);
	}
	
		// 2. Bind the socket to an address using bind() system call.
	//int port = atoi(argv[1]);
	int bindfd = bind(sockfd, my_addr, sizeof(sockaddr_in));
	if(bindfd == -1){
		fprintf(stderr, "Error: Binding failed\n");
		exit(1);
	}
	
		// 3. Listen for connections with listen()
	int listenrt = listen(sockfd, 5);
	if(listenrt == -1){
		fprintf(stderr, "Error: Listen call failed\n");
		exit(1);
	}

	while(1){
		//do stuff with the client 

		// close the client
	}
/*
	4. Accept a connection with accept() -- blocks until a client connects with the server
	5. Send and recieve data 
*/

	return 0;
}
