#ifndef UTIL_COMMON_H__
#define UTIL_COMMON_H__

#include <stdint.h>

extern uint8_t common_reverse_bits_lookup[];

#define REVERSE_BYTE_BITS(value) \
        ((common_reverse_bits_lookup[value & 0x0F] << 4U) | common_reverse_bits_lookup[value >> 4U])

#define SET_BIT_VALUE(source, value, position) \
                                (source = ((source & ~(1U << position)) | (value << position)))

void delay(volatile unsigned long halfsecs);

uint8_t reverse_bits( uint8_t byte );

#endif