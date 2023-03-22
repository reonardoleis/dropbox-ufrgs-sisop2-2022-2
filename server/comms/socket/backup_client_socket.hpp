#pragma once
#include "../../../commons/socket/socket.hpp"

class BackupClientSocket : public Socket {
    private:
        socklen_t clilen;
        struct sockaddr_in serv_addr;
        server_list_t backup_list;
    public:
        BackupClientSocket(const char* server_address, int server_port);
        static void error_handler(int signal);
        int connect_to_master_server();
        struct hostent *server;
};