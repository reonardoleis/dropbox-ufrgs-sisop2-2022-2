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

Socket::Socket() {
    this->sockfd = 0;
    this->port = 0;
    this->is_waiting = false;
    this->is_waiting_lock = new std::mutex();
}

Socket::Socket(const Socket &s)
{
    this->sockfd = s.sockfd;
    this->port = s.port;
    this->is_waiting = s.is_waiting;
    this->is_waiting_lock = new std::mutex();
}

packet Socket::read_packet()
{
    int n = 0;
    int bytes = 0;

    while(n < HEADER_SIZE) {
        bytes = read(this->sockfd, this->buffer + n, HEADER_SIZE - n);
        if (bytes == 0)
        {
            throw SocketError::READ_HEADER_ERROR;
        }
        n += bytes;
    }

    packet p;

    memcpy((char *)&p, this->buffer, HEADER_SIZE);

    p._payload = (char *) malloc(p.length);

    n = 0;
    while(n < p.length) {
       bytes = read(this->sockfd, p._payload + n, p.length - n);
        if (bytes == 0)
            throw SocketError::READ_PAYLOAD_ERROR;
        n += bytes;
    }

    return p;
}

int Socket::write_packet(packet *p)
{
    char *packet_bytes = (char *)malloc(HEADER_SIZE + p->length);
    memcpy(packet_bytes, (char *)p, HEADER_SIZE);
    memcpy(packet_bytes + HEADER_SIZE, (char *)p->_payload, p->length);

    int n = write(this->sockfd, packet_bytes, HEADER_SIZE + p->length);

    if (n != HEADER_SIZE + p->length)
        throw SocketError::WRITE_ERROR;

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
    p.length = strlen(payload) + 1;
    p._payload = (char *)malloc(p.length);
    memcpy(p._payload, payload, p.length);

    return p;
}

packet Socket::build_packet_sized(uint16_t type, uint16_t seqn, uint32_t total_size, uint32_t payload_size, const char* payload)
{
    packet p;
    p.type = type;
    p.seqn = seqn;
    p.total_size = total_size;
    p.length = payload_size;
    p._payload = (char *)malloc(p.length);
    memcpy(p._payload, payload, p.length);

    return p;
}

bool Socket::get_is_waiting()
{
    bool is_waiting;
    this->is_waiting_lock->lock();
    is_waiting = this->is_waiting;
    this->is_waiting_lock->unlock();
    return is_waiting;
}

void Socket::set_is_waiting(bool is_waiting)
{   
    this->is_waiting_lock->lock();
    this->is_waiting = is_waiting;
    this->is_waiting_lock->unlock();
}

void Socket::udp_client()
{
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
}

void Socket::udp_server()
{
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    this->port = UDP_IN_PORT;
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(this->port);
    address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(address.sin_zero), 8);

    bzero(buffer, HEADER_SIZE);
    bind(this->sockfd, (const struct sockaddr *)&address, sizeof(address));

}

void Socket::udp_send(packet *p, sockaddr_in addr)
{
    char * buff = (char *) malloc(HEADER_SIZE + p->length);
    memcpy(buff, p, HEADER_SIZE);
    memcpy(buff + HEADER_SIZE, p->_payload, p->length);
    sendto(this->sockfd, buff, p->length + HEADER_SIZE, MSG_CONFIRM, (const struct sockaddr *)&addr, sizeof(addr));
}

packet Socket::udp_recv()
{
    char * buff = (char *) malloc(HEADER_SIZE + UDP_MAX_MSG);
    this->addr_len = sizeof(this->addr);
    recvfrom(this->sockfd, buff, HEADER_SIZE + UDP_MAX_MSG, MSG_WAITALL, (struct sockaddr *)&(this->addr), &(this->addr_len));
    packet p;

    memcpy((char *)&p, buff, HEADER_SIZE);
    p._payload = (char *) malloc(p.length);
    memcpy((char *)p._payload, buff + HEADER_SIZE, p.length);
    return p;
}
