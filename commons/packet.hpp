#pragma once
#include <cstddef>
#include <unistd.h>
#include <stdint.h>

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
    DOWNLOAD_REQ,
    DOWNLOAD_RESP,
    DELETE_REQ,
    DELETE_RESP,
    LIST_REQ,
    LIST_RESP,
    LOGOUT_REQ, //
    LOGOUT_RESP, //
    STOP_SERVER_REQ, //
    STOP_SERVER_BROADCAST
};

typedef struct packet{
 uint16_t type; //Tipo do pacote (p.ex. DATA | CMD)
 uint16_t seqn; //Número de sequência
 uint32_t total_size; //Número total de fragmentos
 uint16_t length; //Comprimento do payload
 char* _payload = NULL; //Dados do pacote
 ~packet();
} packet;



