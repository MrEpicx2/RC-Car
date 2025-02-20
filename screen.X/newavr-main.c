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
    
    // Enable global interrupts.
    SREG = 0b10000000;
    
    // Set the ADC reference level to VDD.
    VREF.ADC0REF = 0b10000101;
    
    // Enable the ADC interrupt.
    ADC0.INTCTRL = 0b00000001;
    
    // Select PD2 (AIN2) as the ADC input.
    ADC0.MUXPOS = 0x02;

    // Select minimum clock divide.
    ADC0.CTRLC = 0x00;
    
    // Select single ended mode, 12 bit resolution and free-running modes.
    ADC0.CTRLA = 0b00000011;
    
    // Start conversion.
    ADC0.COMMAND = 0x01;
    

    int result_low = 0;
    int result_high = 0;
    
    
    i2c_init(); // Initialize I2C

    _delay_ms(500); // Give I2C some time before checking
    
    lcd_init();
    lcd_clrscr();  // **Clear screen before writing**
    lcd_gotoxy(0, 0);  // **Ensure cursor is at the start**
    
    lcd_puts("Hello World1");  // Put string from RAM
    lcd_gotoxy(0, 2);         // Move cursor to line 3
    lcd_puts_p(PSTR("String from flash")); 
    _delay_ms(3000);
    lcd_clrscr();
    while(1) {
        if (ADC0.INTFLAGS & 0b00000001) {
            result_low = ADC0.RESL;
            result_high = ADC0.RESH;
            ADC0.INTFLAGS = 0b00000001;
        }
        
        if (result_high == 0x04 && result_low == 0x00) {
            lcd_clrscr();
            lcd_puts("0 - 1.25v");
        }
        
        else if (result_high == 0x07 && result_low == 0x33) {
            lcd_clrscr();
            lcd_puts("1.25v - 2.25v");
        }
        
        else if (result_high == 0x0A && result_low == 0x66) {
            lcd_clrscr();
            lcd_puts("2.25v - 5v");
        }
    }
    
}
