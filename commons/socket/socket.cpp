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
    //cli_logger logger = cli_logger(frontend.get_log_stream());
    //logger.set("Started getting is_waiting").stamp().error();
    bool is_waiting;
    this->is_waiting_lock->lock();
    //pthread_mutex_lock(&(this->is_waiting_lock));
    is_waiting = this->is_waiting;
    //pthread_mutex_unlock(&(this->is_waiting_lock));
    this->is_waiting_lock->unlock();
    //logger.set("Stopped getting is_waiting").stamp().error();
    return is_waiting;
}

void Socket::set_is_waiting(bool is_waiting)
{   
    //cli_logger logger = cli_logger(frontend.get_log_stream());
    //logger.set("Started Setting is_waiting to " + std::to_string(is_waiting)).stamp().error();
    //pthread_mutex_lock(&(this->is_waiting_lock));
    this->is_waiting_lock->lock();
    this->is_waiting = is_waiting;
    //pthread_mutex_unlock(&(this->is_waiting_lock));
    this->is_waiting_lock->unlock();
    //logger.set("Stopped Setting is_waiting to " + std::to_string(is_waiting)).stamp().error();
}

