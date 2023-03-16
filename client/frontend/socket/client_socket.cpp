#include "client_socket.hpp"
#include <iostream>
#include <netdb.h> 

ClientSocket::ClientSocket(const char* server_address, int server_port)
{
    std::signal(SocketError::READ_HEADER_ERROR, ClientSocket::error_handler);
    std::signal(SocketError::READ_PAYLOAD_ERROR, ClientSocket::error_handler);
    std::signal(SocketError::WRITE_ERROR, ClientSocket::error_handler);
    std::signal(SocketError::CONNECT_ERROR, ClientSocket::error_handler);

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
        for(int i = 0; i < this->backup_amount; i++)
        {
            ClientSocket sock(this->backup_list[i].server_ip.c_str(), this->backup_list[i].server_port);
            try{
                sock.connect_to_server();
                packet p_req = sock.build_packet(packet_type::MASTER_REQ, 0, 1, "");
                sock.write_packet(&p_req);
                packet p_res = sock.read_packet();
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
                break;
                
            }
            catch(SocketError err){break;}
        }
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
        //this->close_connection();
        for(int i = 0; i < this->backup_amount; i++)
        {
            ClientSocket sock(this->backup_list[i].server_ip.c_str(), this->backup_list[i].server_port);
            try{
                sock.connect_to_server();
                packet p_req = sock.build_packet(packet_type::MASTER_REQ, 0, 1, "");
                sock.write_packet(&p_req);
                packet p_res = sock.read_packet();
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
                break;
                
            }
            catch(SocketError err){break;}
        }
        return this->write_packet(p);
    }
}