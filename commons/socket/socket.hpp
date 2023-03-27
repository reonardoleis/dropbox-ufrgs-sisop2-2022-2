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
#define UDP_IN_PORT 20120
#define UDP_OUT_PORT 20121
#define UDP_MAX_MSG 512



class Socket {
    protected:
        char buffer[HEADER_SIZE];
        bool is_waiting;
        sockaddr_in addr;
        socklen_t addr_len;

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
        packet build_packet_sized(uint16_t type, uint16_t seqn, uint32_t total_size, uint32_t payload_size, const char* payload);
        void udp_server();
        void udp_client();
        packet udp_recv();
        void udp_send(packet *p, sockaddr_in addr);
};

typedef struct _server_ip_port {
    std::string server_ip;
    int server_port;
    pthread_t id;
    Socket socket;
    time_t last_keepalive;
    bool flag;
} server_ip_port_t;

typedef struct _server_list {
    int list_size;
    server_ip_port_t *list;
} server_list_t;