#pragma once

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <csignal>
#include "../packet.hpp"
#include "../errors/errors.hpp"

#define HEADER_SIZE sizeof(packet) - sizeof(char*)

class Socket {
    protected:
        char buffer[HEADER_SIZE];
        

    public:
        int sockfd;
        int port;

        packet read_packet();
        int write_packet(packet *p);
        void close_connection();
};