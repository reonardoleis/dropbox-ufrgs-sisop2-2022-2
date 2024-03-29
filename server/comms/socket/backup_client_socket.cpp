#include "backup_client_socket.hpp"
#include <iostream>
#include <netdb.h> 

BackupClientSocket::BackupClientSocket(const char* server_address, int server_port)
{
    cli_logger logger = cli_logger(frontend.get_log_stream());
    std::signal(SocketError::READ_HEADER_ERROR, BackupClientSocket::error_handler);
    std::signal(SocketError::READ_PAYLOAD_ERROR, BackupClientSocket::error_handler);
    std::signal(SocketError::WRITE_ERROR, BackupClientSocket::error_handler);
    std::signal(SocketError::CONNECT_ERROR, BackupClientSocket::error_handler);

    this->is_waiting = false;
    bzero(this->buffer, HEADER_SIZE);
    if ((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        throw SocketError::BIND_ERROR;

    
    

    this->port = server_port;
    
    //char * unconst_server_address = strdup(server_address);
    //strcat(unconst_server_address, "\0");
    this->server = gethostbyname(server_address);
    this->serv_addr.sin_family = AF_INET;     
    this->serv_addr.sin_port = htons(port);    
	this->serv_addr.sin_addr = *((struct in_addr *)server->h_addr);

	bzero(&(this->serv_addr.sin_zero), 8);
}

void BackupClientSocket::reset_connection(const char* server_address, int server_port)
{
    cli_logger logger = cli_logger(frontend.get_log_stream());
    std::signal(SocketError::READ_HEADER_ERROR, BackupClientSocket::error_handler);
    std::signal(SocketError::READ_PAYLOAD_ERROR, BackupClientSocket::error_handler);
    std::signal(SocketError::WRITE_ERROR, BackupClientSocket::error_handler);
    std::signal(SocketError::CONNECT_ERROR, BackupClientSocket::error_handler);

    this->is_waiting = false;
    bzero(this->buffer, HEADER_SIZE);
    if ((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        throw SocketError::BIND_ERROR;

    
    

    this->port = server_port;
   
    //char * unconst_server_address = strdup(server_address);
    //strcat(unconst_server_address, "\0");
    this->server = gethostbyname(server_address);
    this->serv_addr.sin_family = AF_INET;     
    this->serv_addr.sin_port = htons(port);    
	this->serv_addr.sin_addr = *((struct in_addr *)server->h_addr);

	bzero(&(this->serv_addr.sin_zero), 8);
}

int BackupClientSocket::connect_to_server()
{
    int err = connect(this->sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (err < 0) {
        throw SocketError::CONNECT_ERROR;
    }

    return 0;
}

void BackupClientSocket::error_handler(int signal)
{
    std::cout << "Error found: " << signal << std::endl;
    exit(signal);
}