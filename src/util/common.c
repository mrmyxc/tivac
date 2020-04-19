#include <stdint.h>
#include "util/common.h"

unsigned char common_reverse_bits_lookup[16] =
{
    0b0000,
    0b1000,
    0b0100,
    0b1100,
    0b0010,
    0b1010,
    0b0110,
    0b1110,
    0b0001,
    0b1001,
    0b0101,
    0b1101,
    0b0011,
    0b1011,
    0b0111,
    0b1111
};

void delay(volatile unsigned long eigths)
{
	volatile unsigned long count;

	eigths *= 2UL;

	while (eigths > 0)
	{					 
		count = 197238UL;
		while (count > 0)
		{
			count--;
		}
		eigths--;
	}
}

uint8_t reverse_bits( uint8_t byte )
{
	uint8_t reversed = 0;
	for ( int i = 0, y = 7; i < 8; i++, y-- )
	{
		// Get value at index and shift in opposite index
		reversed |= ((byte & (1U << y)) >> y) << i;
	}
	return reversed;
}