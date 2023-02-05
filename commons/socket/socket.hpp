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
#include <mutex>

#define HEADER_SIZE sizeof(packet) - sizeof(char*)

class Socket {
    protected:
        char buffer[HEADER_SIZE];
        bool is_waiting;

    public:
        Socket();
        Socket(const Socket &s);
        std::mutex *is_waiting_lock;
        int sockfd;
        int port;
        bool get_is_waiting();
        void set_is_waiting(bool is_waiting);
        packet read_packet();
        int write_packet(packet *p);
        void close_connection();
        packet build_packet(uint16_t type, uint16_t seqn, uint32_t total_size, const char* payload);
        packet build_packet_sized(uint16_t type, uint16_t seqn, uint32_t total_size, int payload_size, const char* payload);
};