/*
 * File:   newavr-main.c
 * Author: UOC
 *
 * Created on February 12, 2025, 4:29 PM
 */


#include <avr/io.h>
#include <util/delay.h>
#include "i2c_avr128db28.h"
#include "lcd.h"


int main(void) {
    i2c_init(); // Initialize I2C

    _delay_ms(500); // Give I2C some time before checking
    
    lcd_init();
    lcd_clrscr();  // **Clear screen before writing**
    lcd_gotoxy(0, 0);  // **Ensure cursor is at the start**
    
    lcd_puts("Hello World1");  // Put string from RAM
    lcd_gotoxy(0, 2);         // Move cursor to line 3
    lcd_puts_p(PSTR("String from flash")); 
    while(1); // Keep program running
}
