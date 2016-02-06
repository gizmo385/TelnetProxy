//Helen's Server machine is vm60
//Helen's Socket machine is vm61

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
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
	int len;			//length of 

	bzero((char *)&my_addr, sizeof(my_addr));

		// Create a socket with the socket() system call
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(1);
	}
	
		//Bind the socket to an address using bind() system call
	//assign the address family of the server
	my_addr.sin_family = AF_INET;

	//convert the port to the network byte order specified
	int port = atoi(argv[1]);
	my_addr.sin_port = htons(port);

	//assign the server's ip
	my_addr.sin_addr.s_addr = INADDR_ANY;
	
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
		//do stuff with the client 
		int connection = accept(sockfd, (struct sockaddr *)&my_addr, &len);
		len = recv(connection, buf, sizeof(buf), 0);
		while(len){
			//1st arg (size)
			len = recv(connection, buf, sizeof(buf), 0);
			// payload arg	
			len = recv(connection, buf, sizeof(buf), 0);
			printf("%s\n", buf);
		}

		// close the client
		close(connection);

	}
/*
	4. Accept a connection with accept() -- blocks until a client connects with the server
	5. Send and recieve data 
*/

	return 0;
}
