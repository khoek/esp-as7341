#ifndef __LIB__AS7341_PRIVATE_H
#define __LIB__AS7341_PRIVATE_H

#include <driver/i2c.h>

#include "as7341.h"

static const char* TAG = "as7341";

struct as7341 {
    i2c_port_t port;
    uint8_t addr;
};

#endif
