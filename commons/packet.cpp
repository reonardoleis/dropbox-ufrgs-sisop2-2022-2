#include "packet.hpp"
#include <stdlib.h>
#include <cstdio>
#include <string.h>

packet::~packet() {
    if (this->_payload != NULL)
    {
        free(this->_payload);
        this->_payload = NULL;
    }
}

packet::packet(const packet &p) {
    this->length = p.length;
    this->seqn = p.seqn;
    this->total_size = p.total_size;
    this->type = p.type;
    //free(this->_payload);
    //this->_payload = NULL;
    this->_payload = (char *)malloc(p.length);
    memcpy(this->_payload, p._payload, p.length);
}

packet::packet() {
    this->length = 0;
    this->seqn = 0;
    this->total_size = 0;
    this->type = 50;
    this->_payload = NULL;
}