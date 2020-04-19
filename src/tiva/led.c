#include "tiva/led.h"
#include "util/common.h"
#include "hal/common.h"

void led_init(void)
{
    // Enable clock for Port
    SYSCTL_RCGCGPIO_R |= PORT_F;

    // Use High Performance Bus
    SYSCTL_GPIOHBCTL_R |= PORT_F;

    __asm("NOP\n");

    // GPIO Software Reset Register. Wait till reset flag cleared
    while (!(SYSCTL_PRGPIO_R & PORT_F))
    {
    }

    // Set Direction
    GPIO_PORTF_AHB_DIR_R |= LED_BLUE | LED_GREEN | LED_RED;

    // enable digital
    GPIO_PORTF_AHB_DEN_R |= LED_BLUE | LED_GREEN | LED_RED;

    // Unlock CR
    GPIO_PORTF_AHB_LOCK_R = GPIO_LOCK_KEY;

    // Save pins
    GPIO_PORTF_AHB_CR_R = 0xFF;

    // Lock CR
    GPIO_PORTF_AHB_LOCK_R = 0;
}

void led_on(enum led led)
{
    GPIO_PORTF_AHB_DATA_BITS_R[led] = led;
}

void led_off(enum led led)
{
    GPIO_PORTF_AHB_DATA_BITS_R[led] = 0;
}

void led_flash(enum led led, uint8_t flashes)
{
    while (flashes)
    {
        GPIO_PORTF_AHB_DATA_BITS_R[led] = led;
        delay(1);
        GPIO_PORTF_AHB_DATA_BITS_R[led] = 0;
        delay(1);
        flashes--;
    }
}

void led_flash_multi(enum led led, enum led led2, uint8_t flashes)
{
    while (flashes)
    {
        GPIO_PORTF_AHB_DATA_BITS_R[led] = led;
        GPIO_PORTF_AHB_DATA_BITS_R[led2] = 0;
        delay(1);
        GPIO_PORTF_AHB_DATA_BITS_R[led] = 0;
        GPIO_PORTF_AHB_DATA_BITS_R[led2] = led2;
        delay(1);
        flashes--;
    }
    GPIO_PORTF_AHB_DATA_BITS_R[led] = 0;
    GPIO_PORTF_AHB_DATA_BITS_R[led2] = 0;
}