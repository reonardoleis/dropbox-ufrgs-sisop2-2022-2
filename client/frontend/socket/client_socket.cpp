#include "client_socket.hpp"
#include <iostream>
#include <netdb.h> 

ClientSocket::ClientSocket(const char* server_address, int server_port)
{
    std::signal(SocketError::READ_HEADER_ERROR, ClientSocket::error_handler);
    std::signal(SocketError::READ_PAYLOAD_ERROR, ClientSocket::error_handler);
    std::signal(SocketError::WRITE_ERROR, ClientSocket::error_handler);
    std::signal(SocketError::CONNECT_ERROR, ClientSocket::error_handler);

    this->backup_list = {0, NULL};

    this->is_waiting = false;
    bzero(this->buffer, HEADER_SIZE);
    if ((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        throw SocketError::BIND_ERROR;

    
    

    this->port = server_port;
    this->server = gethostbyname(server_address);
    this->serv_addr.sin_family = AF_INET;     
	this->serv_addr.sin_port = htons(port);    
	this->serv_addr.sin_addr = *((struct in_addr *)server->h_addr);

	bzero(&(this->serv_addr.sin_zero), 8);
}

void ClientSocket::set_backups(server_list_t backup_list)
{
    if(backup_list.list != NULL && backup_list.list_size > 0)
    {
        this->backup_list = backup_list;
    }
}

void ClientSocket::read_backups(packet p)
{
    server_list_t sl;
    size_t curr_pos;
    memcpy(&sl.list_size, p._payload + curr_pos, sizeof(int));
    curr_pos += sizeof(int);
    sl.list = (server_ip_port_t *)malloc(sl.list_size * sizeof(server_ip_port_t));
    for(int i = 0; curr_pos < p.length; i++)
    {
        char buff[255];
        bzero(buff, 255);
        strcpy(buff, p._payload + curr_pos);
        curr_pos += strlen(buff);
        sl.list[i].server_ip = buff;
        memcpy(&(sl.list[i].server_port), p._payload + curr_pos, sizeof(int));
        curr_pos += sizeof(int);
    }
    this->backup_list = sl;
}

int ClientSocket::connect_to_server()
{
    int err = connect(this->sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if (err < 0) {
        throw SocketError::CONNECT_ERROR;
    }

    return 0;
}

void ClientSocket::error_handler(int signal)
{
    std::cout << "Error found: " << signal << std::endl;
    exit(signal);
}

packet ClientSocket::client_read_packet()
{
    try
    {
        return this->read_packet();
    }
    catch(SocketError err)
    {
        //this->close_connection();
        
        
        Socket sock;
        sock.udp_server();
        packet p_res = sock.udp_recv();
        char *new_master = p_res._payload;
        char *ip = strtok(new_master, ":");
        int port = atoi(strtok(NULL, ":"));

        if ((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
            throw SocketError::BIND_ERROR;

        
        

        this->port = port;
        this->server = gethostbyname(ip);
        this->serv_addr.sin_family = AF_INET;     
        this->serv_addr.sin_port = htons(port);    
        this->serv_addr.sin_addr = *((struct in_addr *)server->h_addr);

        this->connect_to_server();
        
        return this->read_packet();
    }
}

int ClientSocket::client_write_packet(packet *p)
{
     try
    {
        return this->write_packet(p);
    }
    catch(SocketError err)
    {
        
        Socket sock;
        sock.udp_server();
        packet p_res = sock.udp_recv();
        char *new_master = p_res._payload;
        char *ip = strtok(new_master, ":");
        int port = atoi(strtok(NULL, ":"));

        if ((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
            throw SocketError::BIND_ERROR;

        
        

        this->port = port;
        this->server = gethostbyname(ip);
        this->serv_addr.sin_family = AF_INET;     
        this->serv_addr.sin_port = htons(port);    
        this->serv_addr.sin_addr = *((struct in_addr *)server->h_addr);

        this->connect_to_server();
            
    }
        
        return this->write_packet(p);
    }
}