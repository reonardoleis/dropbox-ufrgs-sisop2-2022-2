#pragma once
#include "../../../commons/socket/socket.hpp"
#include <string>

class ClientSocket : public Socket {
    private:
        socklen_t clilen;
        struct sockaddr_in serv_addr;
        server_list_t backup_list;
    public:
        ClientSocket(const char* server_address, int server_port);
        void set_backups(server_list_t backup_list);
        void read_backups(packet p);
        static void error_handler(int signal);
        int connect_to_server();
        struct hostent *server;

        //Wrapper for Socket
        packet client_read_packet();
        int client_write_packet(packet *p);
};