#include "hal/tm4c123gh6pm.h"
#include "util/common.h"
#include "hal/common.h"
#include "hal/pll.h"
#include "hal/ssi.h"
#include "tiva/led.h"
#include "lcd_5110/lcd.h"

/**
 * @brief Application Entry Point
 * 
 * @return 0
 */
int main(void)
{
	pll_init();
	led_init();
	lcd_init();

	lcd_write_row(5,20,1, "H E L L O !");

	lcd_display();

	while (1)
	{	
	}

	return 0;
}
