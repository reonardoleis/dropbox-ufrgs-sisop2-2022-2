#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "../types/packet.hpp"


int main(int argc, char *argv[])
{
    int port = 8080;
    if (argc >= 3) {
        port = atoi(argv[2]);
    }

    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	
    char buffer[256];
    if (argc < 2) {
		fprintf(stderr,"usage %s hostname\n", argv[0]);
		exit(0);
    }
	
	server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");
    
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(port);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        printf("ERROR connecting\n");

    printf("Enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 256, stdin);
    
  packet p;
  p.type = 1;
  p.seqn = 1;
  p.total_size = 1;
  p.length = strlen(buffer);
  printf("p.length = %d", p.length);
  p._payload = (char*)malloc(p.length);
  

  int header_size = sizeof(packet) - sizeof(char*);
  memcpy(p._payload, buffer, strlen(buffer) * sizeof(char));

  char* final_buffer = (char*) malloc(256 + header_size);
  memcpy(final_buffer, (char*)&p, header_size);
  memcpy(final_buffer + header_size, p._payload, 256);
	/* write in the socket */
	n = write(sockfd, final_buffer, header_size + 256);
  free(final_buffer);
    if (n < 0) 
		printf("ERROR writing to socket\n");

    bzero(buffer,256);
	
	/* read from the socket */
    n = read(sockfd, buffer, 256);
    if (n < 0) 
		printf("ERROR reading from socket\n");

    printf("%s\n",buffer);
    
	close(sockfd);

  
    return 0;
}