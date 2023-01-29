#include "server_socket.hpp"
#include <cerrno>
#include <iostream>
#undef sock_errno
#define sock_errno() errno

// master socket
ServerSocket::ServerSocket(int port, int queue_size)
{
    std::signal(SocketError::CONNECT_ERROR, ServerSocket::error_handler);
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        printf("ERROR opening socket");

    this->queue_size = queue_size;
    if (queue_size < 0)
    {
        this->queue_size = 1;
    }

    this->port = port;
    if (this->port < 0)
    {
        this->port = 8080;
    }

    this->queue_size = queue_size;
    if (queue_size < 0)
    {
        this->queue_size = 1;
    }

    this->port = port;
    if (this->port < 0)
    {
        this->port = 8080;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(this->port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);

    bzero(buffer, HEADER_SIZE);
}

// slave socket
ServerSocket::ServerSocket(int sockfd)
{
    this->sockfd = sockfd;
    this->queue_size = 1;
    bzero(buffer, HEADER_SIZE);
}

int ServerSocket::bind_and_listen()
{
    int bind_result = bind(this->sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (bind_result < 0)
    {
        printf("ERROR on binding\n");
        return bind_result;
    }

    return listen(this->sockfd, this->queue_size);
}

ServerSocket ServerSocket::accept_connection()
{
    printf("waiting for connection...\n");

    this->clilen = sizeof(struct sockaddr_in);
    int newsockfd = accept(this->sockfd, (struct sockaddr *)&cli_addr, &(this->clilen));
    if (newsockfd < 0)
    {
        printf("ERROR on accept\n");
        std::raise(SocketError::CONNECT_ERROR);
    }

    printf("connection accepted\n");
    bzero(buffer, HEADER_SIZE);

    return ServerSocket(newsockfd);
}

void ServerSocket::error_handler(int signal)
{
    std::cout << "Error found: " << signal << std::endl;
    exit(signal);
}