#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char*argv[]){
	
	if(argc < 2){
		fprintf(stderr, "Error: no client port specified\n");
		exit(1);
	}

	struct sockaddr_in *client_addr = calloc(1, sizeof(sockaddr_in));
	char buf[1024]; 
	int MAX_BUF_SIZE = 1024;

	/*set up the socket*/ 
	int socket = socket(AF_INET, SOCK_STREAM, 0);
	if(socket == -1){
		fprintf(stderr, "Error: client socket creation failed\n");
		exit(1);
	}

	/*send some stuff*/	
	int connection = connect(socket, &client_addr, sizeof(sockaddr_in));
	if(connection == -1){
		fprintf(stderr, "Error: client connection failed\n");
		exit(1);
	}

	while(1){
		getline(buf, *MAX_BUF_SIZE, stdin);
		int strlen = strlen(buf);
		
		write(connection, *buf, 1024);

		close(socket); 
	}

}
