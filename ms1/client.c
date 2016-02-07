#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>	//for closing the connection
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#define MAX_LINE 1024

int main(int argc, char*argv[]){
	
	if(argc < 3){
		fprintf(stderr, "Error: no client port or IP specified\n");
		exit(1);
	}

	struct hostent *hp;		//struct for host IP
	struct sockaddr_in client_addr;	//my address
	char *buf;			//the buffer we write to for the payload 
	int port;			//client's port (specified in cmd line)
	char *sip;			//server IP
	size_t MAX_LINE_SIZE_T = 1024;	//Max size to be passed for getline()

	sip = argv[1]; 
	port = atoi(argv[2]);
	
	/*set IP address*/
	hp = gethostbyname(sip);
	if(!hp){
		fprintf(stderr, "Error: unknown host\n");
		exit(1);	
	}

	/* Create client_addr data structure and copy over address*/
	memset(&client_addr, 0, sizeof(client_addr));
	bzero((char *)&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	bcopy(hp->h_addr, (char *)&client_addr.sin_addr, hp->h_length);
	client_addr.sin_port = htons(port);
	
	/*set up the socket -- this one is an active open*/ 
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		fprintf(stderr, "Error: client socket creation failed\n");
		exit(1);
	}
	
	/*open a connection to a server*/	
	int connection = connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr));
	if(connection == -1){
		fprintf(stderr, "Error: client connection failed\n");
		close(connection);
		exit(1);
	}

	buf = malloc(MAX_LINE);
	memset(&buf, 0, sizeof(buf));
	uint32_t size;
	/* send information */
	while((size = getline(&buf, &MAX_LINE_SIZE_T, stdin))){
		
		/*EOF was reached */
		if(size == -1){
			exit(0);
		}
		send(connection, htonl(size), sizeof(size), 0);	
		send(connection, buf, MAX_LINE, 0);

		close(sockfd); 
	}
	return 0;
}
