#pragma once

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "../../../commons/types/packet.hpp"

#define HEADER_SIZE sizeof(packet) - sizeof(char*)
#define BUFFER_SIZE HEADER_SIZE + 256


class Socket {
    private:
        char buffer[BUFFER_SIZE];

    public:
        int sockfd, n;
        socklen_t clilen;
        struct sockaddr_in serv_addr, cli_addr;
        

        int port;

        int queue_size;
        Socket(int port, int queue_size);
        Socket(int sockfd);
        int read_packet();
        int write_packet(packet p);
        int bind_and_listen();
        void close_connection();
        packet get_buffer();
        Socket accept_connection();

};