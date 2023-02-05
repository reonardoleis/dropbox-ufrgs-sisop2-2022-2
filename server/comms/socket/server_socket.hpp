#pragma once
#include "../../../commons/socket/socket.hpp"
#include "../../../commons/ui/ui_template.hpp"
#include "../../../commons/ui/cli_types.hpp"


class ServerSocket : public Socket {
    public:
        int queue_size;
        socklen_t clilen;
        struct sockaddr_in serv_addr, cli_addr;
        ServerSocket(int port, int queue_size);
        ServerSocket(int sockfd);
        ServerSocket(const ServerSocket &s);
        int bind_and_listen();
        ServerSocket accept_connection();
        static void error_handler(int signal);
};