#pragma once
#include <cstddef>
#include <unistd.h>
#include <stdint.h>
#include <utility>

enum packet_type:short {
    LOGIN_REQ, //
    LOGIN_ACCEPT_RESP, //
    LOGIN_REFUSE_RESP, //
    SYNC_DIR_REQ,
    SYNC_DIR_ACCEPT_RESP,
    SYNC_DIR_REFUSE_RESP,
    UPLOAD_REQ,
    UPLOAD_ACCEPT_RESP,
    UPLOAD_REFUSE_RESP,
    UPLOAD_BROADCAST,
    DOWNLOAD_REQ,
    DOWNLOAD_ACCEPT_RESP,
    DOWNLOAD_REFUSE_RESP,
    DELETE_REQ,
    DELETE_ACCEPT_RESP,
    DELETE_REFUSE_RESP,
    LIST_REQ,
    LIST_ACCEPT_RESP,
    LIST_REFUSE_RESP,
    LOGOUT_REQ, //
    LOGOUT_RESP, //
    STOP_SERVER_REQ, //
    STOP_SERVER_BROADCAST,
    UNKNOWN_RESP,
    DELETE_BROADCAST,
    MASTER_REQ,
    MASTER_RESP,
    REDUNDANCY_REQ,
    REDUNDANCY_ACK,
    REDUNDANCY_NACK,
    SERVER_HANDSHAKE,
    SERVER_KEEPALIVE,
    JOIN_REQ,
    JOIN_RESP,
    VOTE,
    YOUR_ID_RESP,
    SIGNED_PACKET
};

typedef struct packet{
    uint16_t type; //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn; //Número de sequência
    uint32_t total_size; //Número total de fragmentos
    uint32_t length; //Comprimento do payload
    char* _payload = NULL; //Dados do pacote
    ~packet();
    packet(const packet &p);
    packet();
    friend void swap(packet& first, packet& second) // nothrow
    {
        // enable ADL (not necessary in our case, but good practice)
        using std::swap;

        // by swapping the members of two objects,
        // the two objects are effectively swapped
        swap(first.type, second.type);
        swap(first.seqn, second.seqn);
        swap(first.total_size, second.total_size);
        swap(first.length, second.length);
        swap(first._payload, second._payload);
    }
    packet& operator=(packet other)
    {
        swap(*this, other);
        return *this;
    }
} packet;