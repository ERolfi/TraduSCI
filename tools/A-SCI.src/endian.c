#include "common.h"
#include "endian.h"

uint16 get_lsb16 (const uint8 *buffer) {
        return ( (buffer[1] << 8) | buffer[0] );
}

uint16 get_msb16 (const uint8 *buffer) {
        return ( (buffer[0] << 8) | buffer[1] );
}

uint32 get_lsb32 (const uint8 *buffer) {
        return ( (buffer [3] << 24) |
                 (buffer [2] << 16) |
                 (buffer [1] <<  8) |
                  buffer [0] );
}

uint32 get_msb32 (const uint8 *buffer) {
        return ( (buffer [0] << 24) |
                 (buffer [1] << 16) |
                 (buffer [2] <<  8) |
                  buffer [3] );
}

void put_lsb16 (uint8 *buffer, uint16 x) {
        buffer [0] = x & 0xff;
        buffer [1] = x >> 8;
}

void put_msb16 (uint8 *buffer, uint16 x) {
        buffer [0] = x >> 8;
        buffer [1] = x & 0xff;
}

void put_lsb32 (uint8 *buffer, uint32 x) {
        buffer [0] = x & 0xff;
        buffer [1] = x >>  8;
        buffer [2] = x >> 16;
        buffer [3] = x >> 24;
}

void put_msb32 (uint8 *buffer, uint32 x) {
        buffer [0] = x >> 24;
        buffer [1] = x >> 16;
        buffer [2] = x >>  8;
        buffer [3] = x & 0xff;
}
