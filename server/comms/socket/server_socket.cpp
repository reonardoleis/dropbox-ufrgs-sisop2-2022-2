#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "../../../types/packet.hpp"
#include "server_socket.hpp"
#include <cerrno>
#undef sock_errno
#define sock_errno() errno

// master socket
Socket::Socket(int port, int queue_size) {
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        printf("ERROR opening socket");

    this->queue_size = queue_size;
    if (queue_size < 0) {
        this->queue_size = 1;
    }

    this->port = port;
    if (this->port < 0) {
        this->port = 8080;
    }

    this->queue_size = queue_size;
    if (queue_size < 0) {
        this->queue_size = 1;
    }

    this->port = port;
    if (this->port < 0) {
        this->port = 8080;
    }

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(this->port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);
	
	bzero(buffer, BUFFER_SIZE); 
}

// slave socket
Socket::Socket(int sockfd) {
    this->sockfd = sockfd;
    this->queue_size = 1;
	bzero(buffer, BUFFER_SIZE);
}

int Socket::bind_and_listen() {
    if (bind(this->sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding\n");
    return listen(this->sockfd, this->queue_size);
}

Socket Socket::accept_connection() {
    printf("waiting for connection...\n");

    this->clilen = sizeof(struct sockaddr_in);
    int newsockfd = accept(this->sockfd, (struct sockaddr *) &cli_addr, &(this->clilen));
    if (newsockfd < 0) 
        printf("ERROR on accept\n");
        return newsockfd;
    
    printf("connection accepted\n");
    bzero(buffer, BUFFER_SIZE);

    return Socket(newsockfd);
}


int Socket::read_packet() {
    int n = read(this->sockfd, this->buffer, BUFFER_SIZE);
    if (n < 0) 
        printf("\n\nERROR reading from socket %s\n\n", strerror(sock_errno()));

    printf("read %d bytes\n", n);
    return n;
}

int Socket::write_packet(packet p) {
    char* packet_bytes = (char*) malloc(HEADER_SIZE + p.length);
    memcpy(packet_bytes, (char*)&p, HEADER_SIZE);
    memcpy(packet_bytes + HEADER_SIZE, (char*)p._payload, p.length);

    int n = write(this->sockfd, packet_bytes, sizeof(packet_bytes));
    free(packet_bytes);
    if (n < 0) 
        printf("ERROR writing to socket\n");
    
    printf("wrote %d bytes\n", n);
    return n;
}

void Socket::close_connection() {
    close(this->sockfd);
}

packet Socket::get_buffer() {
    packet p;

    memcpy((char*)&p, this->buffer, HEADER_SIZE); 

    p._payload = (char*) malloc(p.length);

    memcpy((char*)p._payload, this->buffer + HEADER_SIZE, p.length);

    return p;
}