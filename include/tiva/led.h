#ifndef LED_H__
#define LED_H__

#include "hal/tm4c123gh6pm.h"
#include "hal/gpio.h"

enum led
{
    LED_RED     =   (1U << 1U),
    LED_BLUE    =   (1U << 2U),
    LED_GREEN   =   (1U << 3U)
};

/**
 * @brief   Initialise GPIOs for use with LEDS on TIVA C LaunchPad
 * 
 */
void led_init( void );

/**
 * @brief   Turn an LED On
 * 
 * @param led 
 */
void led_on( enum led led );

/**
 * @brief   Turn an LED Off
 * 
 * @param led 
 */
void led_off( enum led led );

/**
 * @brief   Flash an LED with a delay of 0.125s (125ms)
 * 
 * @param led LED to flash
 * @param flashes How many times to flash the LED
 */
void led_flash( enum led led, uint8_t flashes );

/**
 * @brief   Flash multiple LEDS with a delay of 0.125s (125ms)
 * 
 * @param led First LED to light up
 * @param led2 Second LED to light up
 * @param flashes How many time to flash both LEDS
 */
void led_flash_multi( enum led led, enum led led2, uint8_t flashes );

#endif