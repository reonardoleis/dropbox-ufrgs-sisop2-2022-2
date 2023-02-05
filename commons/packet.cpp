#include "packet.hpp"
#include <stdlib.h>
#include <cstdio>

packet::~packet() {
    if (this->_payload != NULL)
        free(this->_payload);
}