#pragma once
#include <cstddef>
#include <unistd.h>
#include <stdint.h>

enum packet_type:short {
    sync_dir_req,
    sync_dir_resp
};

typedef struct packet{
 uint16_t type; //Tipo do pacote (p.ex. DATA | CMD)
 uint16_t seqn; //Número de sequência
 uint32_t total_size; //Número total de fragmentos
 uint16_t length; //Comprimento do payload
 char* _payload = NULL; //Dados do pacote
 ~packet();
} packet;



