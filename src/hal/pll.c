#include "hal/pll.h"
#include "hal/tm4c123gh6pm.h"
#define SYSDIV2 7

void pll_init(void)
{
    // set usercc2
    SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;

    // rcc set bypass PLL
    SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;

    // clear xtal
    SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M; 
    // set xtal
    SYSCTL_RCC_R |= SYSCTL_RCC_XTAL_16MHZ;

    // set oscsrc
    SYSCTL_RCC_R &= ~SYSCTL_RCC2_OSCSRC2_M; // clear oscillator source
    SYSCTL_RCC_R |= SYSCTL_RCC2_OSCSRC2_MO; // main

    // clear pwrdwn
    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;

    //SYSCTL_RCC_R |= SYSCTL_RCC_USESYSDIV;
    SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;

    // when DIV400 is set an LSB is appended so shift
    SYSCTL_RCC2_R |= SYSCTL_RCC2_SYSDIV2LSB;

    // clear sysdiv
    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_SYSDIV2_M;

    // set divisor 400MHZ/( frequency + 1 )
    SYSCTL_RCC2_R |= ( 0x04 << SYSCTL_RCC2_SYSDIV2_S ); 

    // wait until expected frequency is ready
    while( !(SYSCTL_RIS_R & SYSCTL_RIS_PLLLRIS) ){}

    // clear bypass
    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
}