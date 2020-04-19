#include <stdint.h>

#include "hal/tm4c123gh6pm.h"
#include "hal/common.h"
#include "hal/ssi.h"

enum ssi_pin
{
    // SSI0 Port A
    SSI0_CLK = PIN_2,
    SSI0_FSS = PIN_3,
    SSI0_RX  = PIN_4,
    SSI0_TX  = PIN_5
};

enum ssi_frame_format
{
    FREESCALE       =   (0U << 4U),
    TEXASINST_SSF   =   (1U << 4U),
    MICROWIRE       =   (2U << 4U)
};

enum ssi_clock_polarity
{
    STEADY_LOW  =   (0U << 6U),
    // Must Enable Pull Up for CLK Pin
    STEADY_HIGH =   (1U << 6U)
};

enum ssi_data_capture_clk_phase
{
    FIRST_EDGE_TRANSITION   =   (0U << 7U),
    SECOND_EDGE_TRANSITION  =   (1U << 7U)
};

enum ssi_datasize
{
    DATASIZE_4 = 0x03,
    DATASIZE_5,
    DATASIZE_6,
    DATASIZE_7,
    DATASIZE_8,
    DATASIZE_9,
    DATASIZE_10,
    DATASIZE_11,
    DATASIZE_12,
    DATASIZE_13,
    DATASIZE_14,
    DATASIZE_15,
    DATASIZE_16
};

enum ssiret ssi0_init( void )
{
    // Enable SSI0 Module Clcok
    SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R0; 

    // Enable Clock for Port A
    SYSCTL_RCGCGPIO_R |= PORT_A;

    // Enable High Performance Bus
    SYSCTL_GPIOHBCTL_R |= PORT_A;

    // Pull Up Enable
    // GPIO_PORTA_AHB_PUR_R |= SSI0_CLK;

    // Don't rely on defaults. Unlock Pins and set AFSEL/PCTL/DIR
    // Unlock all CR
	GPIO_PORTA_AHB_LOCK_R = GPIO_LOCK_KEY;

    // Enable Alternate function on PA2,3,4,5
    GPIO_PORTA_AHB_AFSEL_R |= SSI0_CLK | SSI0_FSS | SSI0_RX | SSI0_TX;           

    // SSI0 Mux, 0x02 for Pins 2, 3, 4, 5
    GPIO_PORTA_AHB_PCTL_R |= (2U << 8U) | (2U << 12U) | (2U << 16U) | (2U << 20U);

    // Disable Analog Mode
    GPIO_PORTA_AHB_AMSEL_R &= ~(SSI0_CLK | SSI0_FSS | SSI0_RX | SSI0_TX);

	// Save programmed pins
	GPIO_PORTA_AHB_CR_R |= 0xFF;

    // Lock CR
    GPIO_PORTA_AHB_LOCK_R = 0;

    // Digital Enable
    GPIO_PORTA_AHB_DEN_R |= SSI0_CLK | SSI0_FSS | SSI0_RX | SSI0_TX;

    // Wait until 0x01 before proceeding - SSI Module Ready
    while ( (SYSCTL_PRSSI_R & SYSCTL_PRSSI_R0) != 0x01 );

    // Ensure SSI Module is Disabled
    SSI0_CR1_R &= ~SSI_CR1_SSE;

    // Master Mode
    SSI0_CR1_R &= ~SSI_CR1_MS;

    // Select Clock Source, System Clock
    SSI0_CC_R |= SSI_CC_CS_SYSPLL;

    // SSI Clock Prescale Divisor. Must be even, between 2 and 254
    SSI0_CPSR_R |= 0x1A;

    // SSI Configurations
    // Serial Clock Rate Using Divisor Above - BR=SysClk/(CPSDVSR * (1 + SCR))
    // 80Mhz / (24 * (1 + 0))
    // SCR = 0
    // SSI0_CR0_R |= (0 << 8U);
    // // Data Capture Serial Clock Phase
    // SSI0_CR0_R |= FIRST_EDGE_TRANSITION;
    // // Clock Polarity
    // SSI0_CR0_R |= STEADY_LOW;
    // // Frame Format
    // SSI0_CR0_R |= FREESCALE;

    SSI0_CR0_R &= ~(SSI_CR0_SPH | SSI_CR0_SPO | SSI_CR0_SCR_M);
    // 8 bit Data Size
    SSI0_CR0_R |= DATASIZE_8;

    // Enable SSI Module
    SSI0_CR1_R |= SSI_CR1_SSE;

    return SSI_OK;
}

enum ssiret ssi0_write( uint8_t byte )
{
    while ( (SSI0_SR_R & SSI_SR_BSY) || !(SSI0_SR_R & SSI_SR_TNF) );
    // Not Busy and Transmit Not Full
    SSI0_DR_R = byte; // Transmit Data

    // Until until transaction complete
    while ( (SSI0_SR_R & SSI_SR_BSY) );
    
    return SSI_OK;
}

enum ssiret ssi0_read( uint8_t * byte )
{
    enum ssiret status = SSI_OK;
    if ( !(SSI0_SR_R & SSI_SR_BSY) && (SSI0_SR_R & SSI_SR_RNE) )
    {
        // Not Busy and Receive Not Empty
        *byte = SSI0_DR_R;
    }
    else
    {
        status = SSI_RX_FIFO_EMPTY;
    }
    
    return status;
}
