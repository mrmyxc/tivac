#ifndef HAL_SSI_H__
#define HAL_SSI_H__

#include <stdint.h>

#include "tm4c123gh6pm.h"

enum ssiret
{
    SSI_OK,
    SSI_TX_FIFO_FULL,
    SSI_RX_FIFO_EMPTY
};

/**
 * @brief   Initialise SSI0 to ~3Mbps
 *          CLK:                PA2
 *          Slave/Chip Select:  PA3
 *          MOSI:               PA5
 *          MISO:               PA4      
 * 
 * @return enum ssiret 
 */
enum ssiret ssi0_init( void );

/**
 * @brief   Write a byte to SSI0 TX FIFO
 * 
 * @param byte Byte to write
 * @return enum ssiret 
 */
enum ssiret ssi0_write( uint8_t byte );

/**
 * @brief   Read a byte from SSI0 RX FIFO
 * 
 * @param byte Received byte storage pointer
 * @return enum ssiret 
 */
enum ssiret ssi0_read( uint8_t * byte );


#endif