#pragma once
#include "../../../types/socket/socket.hpp"
class ClientSocket : public Socket {
    public:
        socklen_t clilen;
        struct sockaddr_in serv_addr;
        ClientSocket(const char* server_address, int server_port);
        static void error_handler(int signal);
        int connect_to_server();
        struct hostent *server;
};