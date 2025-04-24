/*
 * File:   newavr-main.c
 * Author: UOC
 *
 * Created on March 12, 2025, 12:54 PM
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include "font.h"
#include "lcd.h"
#include "i2c_avr128db28.h"

volatile uint16_t value[5];
volatile uint8_t count = 0;
volatile uint16_t distance[5];  

uint8_t NumberToAscii(uint32_t u32Number_, uint8_t* pu8AsciiString_) {
  bool bFoundDigit = false;
  uint8_t au8AsciiNumber[11];
  uint8_t u8CharCount = 0;
  uint32_t u32Divider = 1000000000;
  uint8_t u8Temp;
  
  /* Parse out all the digits, start counting after leading zeros */
  for(uint8_t i = 0; i < 10; i++)
  {
    u8Temp = (u32Number_ / u32Divider);
    
    /* Check for first non-leading zero */
    if(u8Temp != 0)
    {
      bFoundDigit = true;
    }

    /* As long as a non-leading zero has been found, add the ASCII char */
    if(bFoundDigit)
    {
      au8AsciiNumber[u8CharCount] = u8Temp + 0x30;
      u8CharCount++;
    }
    
    /* Update for next iteration */
    u32Number_ %= u32Divider;
    u32Divider /= 10;
  }
  
  /* Handle special case where u32Number == 0 */
  if(!bFoundDigit)
  {
    u8CharCount = 1;
    au8AsciiNumber[0] = '0';
  }
  
  /* Add the null and copy to destination */
  au8AsciiNumber[u8CharCount] = '\0';
  strcpy((char *)pu8AsciiString_, (const char*)au8AsciiNumber);
  
  return(u8CharCount);
}

int main(void) {
    CCP = 0xD8;                 // unlock protected I/O registers - page 41
    CLKCTRL.OSCHFCTRLA = 0x14;  // Clock set to 8 MHz
    while (CLKCTRL.MCLKSTATUS & 0b00000001) {
    }
    
    // ******************************************Servo and ultra sonic trigger on TCA0******************************************************************************************* 
    TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_DIV64_gc) | (TCA_SINGLE_ENABLE_bm); // Divide 8MHz by 256 and enable TCA0
    TCA0.SINGLE.CTRLB = (TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm);  // Enable single slope PWM on CMP0 and CMP1
    TCA0.SINGLE.PER = 125;                      // Frequency set to 50 Hz page 236: (31250/625) 625, being the PER value
    
    // CMP0 will be for trigger and CMP1 will be for the servo
    
    TCA0.SINGLE.CMP0 = 2;                          // Can't get to 10us at this TCA0 frequency, shortest is 1/31250 = 32us. should be fine

    PORTA.DIRSET = PIN0_bm;    
    EVSYS.CHANNEL2 = 0x41;                      // PORTC pin 1 is set to channel 0, will be the incoming echo signal
    EVSYS.USERTCB2CAPT = 0x03;                  // Select channel 0 for this user (3-1=2)
    
    // TCB2 will be used as the timer to process the echo signal from the ultra sonic sensor
    TCB2.INTCTRL = 0x01;                        // Capture interrupt enabled
    TCB2.EVCTRL = 0x01;                         // Enable input capture event
    TCB2.CTRLB = 0x04;                          // Input capture PW measurement mode
    TCB2.CTRLA = 0x01;                          // Uses clk with no division
    
    sei();                                      // Enable global interrupt 
    
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
    
    uint8_t string[4];
    while (1) {
        for (uint8_t i = 0; i < 5; i++) {
            NumberToAscii(distance[i], string);
            lcd_puts(string);
            lcd_puts(" CM");
            _delay_ms(1000);
            lcd_clrscr();
        }
        
    }
}

ISR(TCB2_INT_vect) {
    TCB2.INTFLAGS = 0x01;
    value[count] = TCB2.CCMP;
    distance[count] = (value[count] * 0.125) / 58; // Convert to cm times value by 0.125 to account for sys clk, I think 
    count++;
    if (count >= 5) {
        count = 0;
    }
}