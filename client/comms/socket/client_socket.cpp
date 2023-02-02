#include "client_socket.hpp"
#include <iostream>
#include <netdb.h> 

ClientSocket::ClientSocket(const char* server_address, int server_port)
{
    bzero(this->buffer, HEADER_SIZE);
    if ((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        std::raise(SocketError::BIND_ERROR);

    this->port = server_port;
    this->server = gethostbyname(server_address);
    this->serv_addr.sin_family = AF_INET;     
	this->serv_addr.sin_port = htons(port);    
	this->serv_addr.sin_addr = *((struct in_addr *)server->h_addr);

	bzero(&(this->serv_addr.sin_zero), 8);
}

int ClientSocket::connect_to_server()
{
    int err = connect(this->sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if (err < 0) {
        std::raise(SocketError::CONNECT_ERROR);
    }

    return 0;
}

void ClientSocket::error_handler(int signal)
{
    std::cout << "Error found: " << signal << std::endl;
    exit(signal);
}