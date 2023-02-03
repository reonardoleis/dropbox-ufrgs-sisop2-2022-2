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
#include "../ui/cli_types.hpp"
#include "../ui/ui_template.hpp"

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
        packet build_packet(uint16_t type, uint16_t seqn, uint32_t total_size, const char* payload);
};