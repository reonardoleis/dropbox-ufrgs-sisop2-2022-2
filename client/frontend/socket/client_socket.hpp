#pragma once
#include "../../../commons/socket/socket.hpp"
#include <string>

typedef struct _server_ip_port {
    std::string server_ip;
    int server_port;
} server_ip_port_t;

class ClientSocket : public Socket {
    private:
        socklen_t clilen;
        struct sockaddr_in serv_addr;
        server_ip_port_t *backup_list;
        int backup_amount;
    public:
        ClientSocket(const char* server_address, int server_port);
        static void error_handler(int signal);
        int connect_to_server();
        struct hostent *server;

        //Wrapper for Socket
        packet client_read_packet();
        int client_write_packet(packet *p);
};