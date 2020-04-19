#include <stdint.h>

#include "lcd_5110/lcd.h"
#include "hal/tm4c123gh6pm.h"
#include "hal/common.h"
#include "hal/gpio.h"
#include "hal/ssi.h"
#include "util/common.h"

#include "tiva/led.h"

#define MAX_X 83U
#define MAX_Y 47U
#define ROW_BANKS 6U
#define ROWS 48U
#define COLUMNS 84U
#define PIXELS_BYTE 8U
#define BYTES (ROWS * COLUMNS / PIXELS_BYTE)
#define FONT_BASE_WIDTH 5U
#define PADDING_ON 0x01
#define PADDING_OFF 0x00

#define FONT_ROWS 5U
#define FONT_COLUMNS 11U
#define FONT_WIDTH 5U

#define START_DRAWING_BIT 7U

#define DATA_COMMAND_PIN (*(GPIO_PORTA_AHB_DATA_BITS_R + PIN_6))
#define RESET_PIN__N (*(GPIO_PORTA_AHB_DATA_BITS_R + PIN_7))

#define GET_CURSOR_BYTE() (((lcd_cursor_y / PIXELS_BYTE) * (COLUMNS)) + lcd_cursor_x)

// Start each byte from bit 7
#define GET_CURSOR_BIT() (lcd_cursor_y % PIXELS_BYTE)

enum lcd_5110_datatype
{
    LCD5110_COMMAND = 0,
    // Byte is stored in the LCD display buffer
    LCD5110_DATA = PIN_6
};

enum lcd_5110_pin
{
    VCC,
    LIGHT,
    GND,
    CLK = PIN_2,
    // May be active LOW
    CHIP_ENABLE__N = PIN_3,
    CE__N = PIN_3,
    DATA_IN = PIN_5,
    DIN = PIN_5,
    DATA_COMMAND = PIN_6,
    DC = PIN_6,
    RESET__N = PIN_7,
};

enum lcd_5110_status
{
    LCD5110_RESET_LOW = 0,
    LCD5110_RESET_HIGH = (PIN_7)
};

static const char ASCII[][5];

// static enum lcd_5110_font font = LCD_5110_FONT_COURSE;

static uint16_t lcd_cursor_byte = 0;
static uint8_t lcd_cursor_bit = 0;

static uint8_t lcd_cursor_x = 0;
static uint8_t lcd_cursor_y = 0;

static uint8_t lcd_screen_buffer[ROWS * COLUMNS / PIXELS_BYTE] = {0};

static void lcd_send(enum lcd_5110_datatype data_type, uint8_t data);

void lcd_init(void)
{
    // Start SPI Interface
    ssi0_init();

    // Enable Clock for Port A
    SYSCTL_RCGCGPIO_R |= PORT_A;

    // Enable High Performance Bus
    SYSCTL_GPIOHBCTL_R |= PORT_A;

    // Unlock all CR
    GPIO_PORTA_AHB_LOCK_R = GPIO_LOCK_KEY;

    // Set Outputs
    GPIO_PORTA_AHB_DIR_R |= RESET__N | DATA_COMMAND;

    GPIO_PORTA_AHB_AFSEL_R &= ~(RESET__N | DATA_COMMAND);

    // Save programmed pins
    GPIO_PORTA_AHB_CR_R |= 0xFF;

    // Lock CR
    GPIO_PORTA_AHB_LOCK_R = 0;

    // Digital Enable
    GPIO_PORTA_AHB_DEN_R |= RESET__N | DATA_COMMAND;

    // Disable Analog Function
    GPIO_PORTA_AHB_AMSEL_R &= ~(RESET__N | DATA_COMMAND);

    __asm("NOP\n");

    // Apply RESET' pulse (Resets all registers)
    RESET_PIN__N = LCD5110_RESET_LOW;
    delay(1);
    RESET_PIN__N = LCD5110_RESET_HIGH;

    // Set LCD Functions. Chip Active. Horizontal Addressing. Use Extended Instruction Set
    // Addressing Mode chooses whether X or Y gets incremented automatically First.
    // When it reaches max, next ordinate gets incremented. X = 83, Y = 5
    // Horizontal = X. Vertical = Y
    lcd_send(LCD5110_COMMAND, 0x21);

    // Contrast Levels
    lcd_send(LCD5110_COMMAND, 0xC0);

    // Temperature Coef
    lcd_send(LCD5110_COMMAND, 0x4);

    // Set Bias mode
    lcd_send(LCD5110_COMMAND, 0x14);

    // Must send before modifying display control mode
    lcd_send(LCD5110_COMMAND, 0x20);

    lcd_page_flip();

    // Set display to Normal Mode
    lcd_send(LCD5110_COMMAND, 0x0C);

    lcd_clear_screen_buffer();

    lcd_clear_screen();

    lcd_set_buffer_pixel_cursor(0, 0);
}

void lcd_display(void)
{
    // LSB printed first not MSB
    // Set display cursor position to Bank(Y) = 0, X = 0
    lcd_send(LCD5110_COMMAND, 0x80 | (0 & 0x7F));
    lcd_send(LCD5110_COMMAND, 0x40 | ((0 % ROW_BANKS) & 0x07));
    for (uint16_t i = 0; i < (COLUMNS * ROWS / PIXELS_BYTE); i++)
    {
        lcd_send(LCD5110_DATA, lcd_screen_buffer[i]);
    }
}

void lcd_send(enum lcd_5110_datatype data_type, uint8_t data)
{
    DATA_COMMAND_PIN = data_type;
    if (data_type == LCD5110_COMMAND)
    {
        while (!(SSI0_SR_R & SSI_SR_TFE))
            ;
    }
    ssi0_write(data);
}

void lcd_write_pixel(void)
{
    SET_BIT_VALUE(lcd_screen_buffer[lcd_cursor_byte], 1, lcd_cursor_bit);
}

void lcd_nb_write_char(char character)
{
    // Padding on the left
    lcd_send(LCD5110_DATA, 0x00);
    for (uint8_t i = 0; i < FONT_WIDTH; i++)
    {
        lcd_send(LCD5110_DATA, ASCII[character - 0x20][i]);
    }
    // Padding on the right
    lcd_send(LCD5110_DATA, 0x00);
}

void lcd_nb_write_string(char *string)
{
    while (*string)
    {
        lcd_nb_write_char(*string);
        string++;
    }
}

void lcd_nb_write_line(uint8_t x, uint8_t y, char *string)
{
    lcd_nb_set_cursor(x, y);
    lcd_nb_write_string(string);
}

void lcd_nb_set_cursor(uint8_t x, uint8_t y)
{
    // Command for X Co-ordinate: 0b1XXXXXXX
    // Set X Co-ordinate
    lcd_send(LCD5110_COMMAND, 0x80 | (x & 0x7F));

    // Command for Y Co-ordinate: 0b01000YYY
    // Set Y Co-ordinate - Screen is in 6 Row Segments
    lcd_send(LCD5110_COMMAND, 0x40 | ((y % ROW_BANKS) & 0x07));
}

void lcd_write_byte(uint8_t byte)
{
    uint8_t cache_cursor_bit = lcd_cursor_bit;

    // if ( lcd_cursor_bit == START_DRAWING_BIT )
    // {
    //     lcd_screen_buffer[lcd_cursor_byte] = byte;
    //     lcd_cursor_byte++;
    // }
    // else
    {
        // Partial byte writes
        for (uint8_t i = 0; i < PIXELS_BYTE; i++)
        {
            SET_BIT_VALUE(lcd_screen_buffer[lcd_cursor_byte], ((byte >> (i)) & 0x01), lcd_cursor_bit);

            // Written to all of byte so move on to the next byte and continue writing the reset of
            // the bits to that byte
            if (lcd_cursor_bit == START_DRAWING_BIT)
            {
                // Skip rest of columns until back in same Y co-ordinate
                lcd_cursor_byte += COLUMNS;
            }

            // Keep bit value inbound
            lcd_cursor_bit = (uint8_t)(lcd_cursor_bit + 1) % PIXELS_BYTE;
        }

        // Advance to next column - wraps around to 0 from MAX_X
        lcd_cursor_x = (lcd_cursor_x + 1) % COLUMNS;
        lcd_cursor_byte = GET_CURSOR_BYTE();

        // Return to original bit position after writing
        lcd_cursor_bit = cache_cursor_bit;
    }
}

void lcd_write_char(char character)
{
    // Padding on the left
    lcd_write_byte(0x00);

    for (uint8_t i = 0; i < FONT_WIDTH; i++)
    {
        lcd_write_byte(ASCII[character - 0x20][i]);
    }

    // Padding on the right
    lcd_write_byte(0x00);
}

void lcd_write_string(char *string)
{
    while (*string)
    {
        lcd_write_char(*string);
        string++;
    }
}

void lcd_write_line(uint8_t row, uint8_t start_position, char *string)
{
    lcd_set_text_cursor(0, row);
    lcd_write_string(string);
    lcd_set_text_cursor(0, row + 1);
}

void lcd_write_row(uint8_t start_x, uint8_t start_y, uint8_t fill, char *string)
{
    uint8_t fill_mask = 0x00;
    if (fill)
    {
        fill_mask = 0xFF;
    }

    // Fill the beginning of the row until the first character can begin printing
    lcd_set_buffer_pixel_cursor(0, start_y);
    while (lcd_cursor_x < start_x)
    {
        lcd_write_byte(fill_mask ^ 0x00U);
    }

    // Print the string to the screen buffer
    lcd_set_buffer_pixel_cursor((start_x + 0), start_y);
    while (*string)
    {
        lcd_write_byte(fill_mask ^ 0x00U);

        for (int8_t i = 0; i < FONT_WIDTH; i++)
        {
            lcd_write_byte(fill_mask ^ (ASCII[*string - 0x20][i]));
        }
        string++;

        lcd_write_byte(fill_mask ^ 0x00U);
    }

    // Fill the rest of the row from the end of the text printing
    uint8_t tmp = lcd_cursor_x;
    for (uint8_t i = 0; i < (COLUMNS - tmp); i++)
    {
        lcd_write_byte(fill_mask ^ 0x00U);
    }
}

/**
 * @brief Set pixel co-ordinate
 * 
 * @param x Column 0:84
 * @param y Row 0:44
 */
void lcd_set_buffer_pixel_cursor(uint8_t x, uint8_t y)
{
    lcd_cursor_x = x % COLUMNS;
    lcd_cursor_y = y % (MAX_Y + 1);

    lcd_cursor_byte = GET_CURSOR_BYTE();
    lcd_cursor_bit = GET_CURSOR_BIT();
}

void lcd_set_text_cursor(uint8_t column, uint8_t row)
{
    if (row > FONT_ROWS || column > FONT_COLUMNS)
    {
        return;
    }
    lcd_set_buffer_pixel_cursor(column * FONT_WIDTH, row * PIXELS_BYTE);
}

void lcd_draw_screen(uint8_t image[], uint16_t start_x, uint8_t start_y, uint16_t length)
{
    lcd_set_buffer_pixel_cursor(start_x, start_y);
    for (uint16_t i = 0; i < length; i++)
    {
        lcd_write_byte(REVERSE_BYTE_BITS(image[i]));
    }
}

void lcd_clear_screen(void)
{
    lcd_set_buffer_pixel_cursor(0, 0);

    for (uint8_t i = 0; i < COLUMNS; i++)
    {
        lcd_send(LCD5110_DATA, 0x00);
    }

    delay(5);

    lcd_nb_set_cursor(0, 0);
}

void lcd_clear_screen_buffer(void)
{
    for (uint16_t i = 0; i < sizeof(lcd_screen_buffer); i++)
    {
        lcd_screen_buffer[i] = 0x00;
    }
}

void lcd_page_flip(void)
{
    // All Display Segments On
    lcd_send(LCD5110_COMMAND, 0b00001001);
    lcd_set_buffer_pixel_cursor(0, 0);
    lcd_nb_set_cursor(0, 0);
    lcd_clear_screen_buffer();
    lcd_clear_screen();
    delay(1);
    // Normal Mode
    lcd_send(LCD5110_COMMAND, 0x0C);
}

static const char ASCII[][FONT_WIDTH] =
    {
        {0x00, 0x00, 0x00, 0x00, 0x00} // 20
        ,
        {0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
        ,
        {0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
        ,
        {0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
        ,
        {0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
        ,
        {0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
        ,
        {0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
        ,
        {0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
        ,
        {0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
        ,
        {0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
        ,
        {0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
        ,
        {0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
        ,
        {0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
        ,
        {0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
        ,
        {0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
        ,
        {0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
        ,
        {0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
        ,
        {0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
        ,
        {0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
        ,
        {0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
        ,
        {0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
        ,
        {0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
        ,
        {0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
        ,
        {0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
        ,
        {0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
        ,
        {0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
        ,
        {0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
        ,
        {0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
        ,
        {0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
        ,
        {0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
        ,
        {0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
        ,
        {0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
        ,
        {0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
        ,
        {0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
        ,
        {0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
        ,
        {0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
        ,
        {0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
        ,
        {0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
        ,
        {0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
        ,
        {0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
        ,
        {0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
        ,
        {0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
        ,
        {0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
        ,
        {0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
        ,
        {0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
        ,
        {0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
        ,
        {0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
        ,
        {0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
        ,
        {0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
        ,
        {0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
        ,
        {0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
        ,
        {0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
        ,
        {0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
        ,
        {0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
        ,
        {0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
        ,
        {0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
        ,
        {0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
        ,
        {0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
        ,
        {0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
        ,
        {0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
        ,
        {0x02, 0x04, 0x08, 0x10, 0x20} // 5c '\'
        ,
        {0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
        ,
        {0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
        ,
        {0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
        ,
        {0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
        ,
        {0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
        ,
        {0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
        ,
        {0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
        ,
        {0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
        ,
        {0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
        ,
        {0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
        ,
        {0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
        ,
        {0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
        ,
        {0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
        ,
        {0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j
        ,
        {0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
        ,
        {0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
        ,
        {0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
        ,
        {0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
        ,
        {0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
        ,
        {0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
        ,
        {0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
        ,
        {0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
        ,
        {0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
        ,
        {0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
        ,
        {0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
        ,
        {0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
        ,
        {0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
        ,
        {0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
        ,
        {0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
        ,
        {0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
        ,
        {0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
        ,
        {0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
        ,
        {0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
        ,
        {0x10, 0x08, 0x08, 0x10, 0x08} // 7e ~
        ,
        {0x78, 0x46, 0x41, 0x46, 0x78} // 7f DEL
        ,
        {0x1f, 0x24, 0x7c, 0x24, 0x1f} // 7f UT sign
};
