#ifndef HAL_COMMON_H__
#define HAL_COMMON_H__

typedef enum GPIOPort
{
    PORT_A  =   (1U << 0U),
    PORT_B  =   (1U << 1U),
    PORT_C  =   (1U << 2U),
    PORT_D  =   (1U << 3U),
    PORT_E  =   (1U << 4U),
    PORT_F  =   (1U << 5U),
} Port;

typedef enum GPIOPin
{
    PIN_0  =   (1U << 0U),
    PIN_1  =   (1U << 1U),
    PIN_2  =   (1U << 2U),
    PIN_3  =   (1U << 3U),
    PIN_4  =   (1U << 4U),
    PIN_5  =   (1U << 5U),
    PIN_6  =   (1U << 6U),
    PIN_7  =   (1U << 7U),
} Pin;

#endif