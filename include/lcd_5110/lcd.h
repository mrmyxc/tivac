#ifndef LCD_5110_NOKIA_H__
#define LCD_5110_NOKIA_H__

#include "hal/tm4c123gh6pm.h"
#include "hal/common.h"
#include "hal/gpio.h"
#include "hal/ssi.h"

enum lcd_5110_font
{
    LCD_5110_FONT_COURSE = 0,
    LCD_5110_FONT_MINE = 1,
};

enum lcd_5110_text_mode
{
    LCD_5110_TEXT_MODE_NORMAL = 0x00U,
    LCD_5110_TEXT_MODE_INVERSE = 0
};

/**
 * @brief   Initialiase LCD to use SSI0
 * 
 */
void lcd_init(void);

/**
 * @brief   Push screen buffer data to the LCD screen
 * 
 */
void lcd_display(void);

/**
 * @brief   Write a single pixel to te buffer at the current pixel cursor 
 * 
 */
void lcd_write_pixel(void);

/**
 * @brief   Write a character directly to the screen
 * 
 * @param character 
 */
void lcd_nb_write_char(char character);

/**
 * @brief   Write a string directly to the screen
 * 
 * @param character 
 */
void lcd_nb_write_string(char * character);

/**
 * @brief   Write a string in a bank directly to the screen
 * 
 * @param column X ordinate to start writing from
 * @param screen_bank Bank to write in
 * @param character Character to write
 */
void lcd_nb_write_line(uint8_t column, uint8_t screen_bank, char * character);

/**
 * @brief   Set the cursor of the screen directly
 * 
 * @param column X ordinate to set cursor
 * @param screen_bank Bank to set cursor
 */
void lcd_nb_set_cursor(uint8_t column, uint8_t screen_bank);

/**
 * @brief   Write a byte to the screen buffer
 * 
 * @param byte Byte to write
 */
void lcd_write_byte(uint8_t byte);

/**
 * @brief   Write a string to the screen buffer
 *          String will wrap around to the x=0 if it's too long
 *          
 * @param string    String to write
 */
void lcd_write_string(char *string);

/**
 * @brief   Write a line using the screen buffer. 
 *          Advances cursor to the next line.
 * 
 * @param line Corresponds to the actual screen's bank 
 * @param start_position Column to start writing from
 * @param string String to write
 */
void lcd_write_line(uint8_t line, uint8_t start_position, char *string);

/**
 * @brief   Write a string to a row in the screen buffer.
 *          The row can be any y co-ordinate as opposed to the other writeline functions
 *          which use the banks
 * 
 * @param start_x Column to start writing from
 * @param start_y Row to start writing from
 * @param fill Invert text or don't
 * @param string String to write
 */
void lcd_write_row(uint8_t start_x, uint8_t start_y, uint8_t fill, char *string);

/**
 * @brief   Set the pixel position for the screen buffer
 * 
 * @param x Column select
 * @param y Row select
 */
void lcd_set_buffer_pixel_cursor(uint8_t x, uint8_t y);

/**
 * @brief   Set the text cursor of the screen buffer rather than raw pixel cursor
 * 
 * @param column Text column to start next write
 * @param row Text row to start next write
 */
void lcd_set_text_cursor(uint8_t column, uint8_t row );

/**
 * @brief Write raw bytes to the entire screen buffer 
 * 
 * @param image Data array to write
 * @param start_x Pixel column to start
 * @param start_y Pixel row to start
 * @param length Length of the array
 */
void lcd_draw_screen(uint8_t image[], uint16_t start_x, uint8_t start_y, uint16_t length);

/**
 * @brief   Clear the LCD screen's internal buffer
 * 
 */
void lcd_clear_screen(void);

/**
 * @brief   Clear the software screen buffer
 * 
 */
void lcd_clear_screen_buffer(void);

/**
 * @brief   Do a page flip animation type thing.
 *          Clears all buffers and resets cursors to 0,0
 * 
 */
void lcd_page_flip(void);

#endif