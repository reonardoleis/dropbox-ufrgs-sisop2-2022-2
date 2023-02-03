#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../packet.hpp"
#include "socket.hpp"
#include <cerrno>
#undef sock_errno
#define sock_errno() errno

packet Socket::read_packet()
{
    int n = 0;
    int bytes = 0;

    while(n < HEADER_SIZE) {
        bytes = read(this->sockfd, this->buffer + n, HEADER_SIZE - n);
        if (bytes == 0)
            std::raise(SocketError::READ_HEADER_ERROR);
        n += bytes;
    }

    packet p;

    memcpy((char *)&p, this->buffer, HEADER_SIZE);

    p._payload = (char *) malloc(p.length);

    n = 0;
    while(n < p.length) {
       bytes = read(this->sockfd, p._payload + n, p.length - n);
        if (bytes == 0)
            std::raise(SocketError::READ_PAYLOAD_ERROR);
        n += bytes;
    }

    return p;
}

int Socket::write_packet(packet *p)
{
    cli_logger logger = cli_logger(frontend.get_log_stream());
    char *packet_bytes = (char *)malloc(HEADER_SIZE + p->length);
    memcpy(packet_bytes, (char *)p, HEADER_SIZE);
    memcpy(packet_bytes + HEADER_SIZE, (char *)p->_payload, p->length);

    int n = write(this->sockfd, packet_bytes, HEADER_SIZE + p->length);

    if (n != HEADER_SIZE + p->length)
        std::raise(SocketError::WRITE_ERROR);

    logger.set("Sent packet of " + std::to_string(n) + " bytes").stamp().info();
    return n;
}

void Socket::close_connection()
{
    close(this->sockfd);
    this->sockfd = 0;
}

packet Socket::build_packet(uint16_t type, uint16_t seqn, uint32_t total_size, const char* payload)
{
    packet p;
    p.type = type;
    p.seqn = seqn;
    p.total_size = total_size;
    p.length = strlen(payload);
    p._payload = (char *)malloc(p.length);
    memcpy(p._payload, payload, p.length);

    return p;
}