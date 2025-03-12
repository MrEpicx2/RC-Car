/*
 * File:   test.c
 * Author: UOC
 *
 * Created on March 5, 2025, 8:42 AM
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include "i2c_avr128db28.h"
#include "lcd.h"
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>

uint16_t value[5];
uint8_t count = 0;
uint16_t distance[5];
uint8_t cm[10];
uint16_t duty = 250;
bool newDataReady = false;

void trigger_pulse(void) {
    PORTC.OUTSET = PIN0_bm; // Set PC0 high
    TCB0.CTRLA |= TCB_ENABLE_bm; // Start the timer
    while (!(TCB0.INTFLAGS & TCB_CAPT_bm));             // Wait for timer completion
    TCB0.INTFLAGS = 0x01;                               // Clear interrupt
    PORTC.OUTCLR = PIN0_bm; // Clear PC0 (pulse done)
}

uint8_t NumberToAscii(uint32_t u32Number_, uint8_t* pu8AsciiString_)
{
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
    /*
    TCA0.SINGLE.CTRLA = (0x01) << 3 | 0x01;     // Divide by 16 and enable peripheral
    TCA0.SINGLE.CTRLB = 0x03;                   // single slope PWM
    TCA0.SINGLE.CTRLB |= 0x01 << 4;             // PA0 enabled as single slope
    TCA0.SINGLE.PER = 500-1;                    // Freq set to 1KHz
    TCA0.SINGLE.CMP0 = duty;                    // Duty cycle
    */
    
    // 10us timer setup for trigger on PC0
    TCB0.CTRLA = 0x01;                          // Enable TCB0 with the 8MHz CLK
    TCB0.CTRLB = 0x00;                          // Periodic interrupt mode
    TCB0.CCMP = 80;                             // 10us / 125ns(clk period) = 80
    TCB0.INTCTRL = 0x01;                        // Enable interrupt on capture (when CCMP reaches TOP an interrupt is generated)
    PORTC.DIRSET = 0x01;                        // PC0 as output
    
    EVSYS.CHANNEL1 = 0x41;                      // PORTC pin 1 is set to channel 2, will be the incoming echo signal
    EVSYS.USERTCB1CAPT = 0x03;                  // Select channel 2 for this user (3-1=2)
    
    // TCB1 will be used as the timer to process the echo signal from the ultra sonic sensor
    TCB1.INTCTRL = 0x01;                        // Capture interrupt enabled
    TCB1.EVCTRL = 0x01;                         // Enable input capture event
    TCB1.CTRLB = 0x04;                          // Input capture PW measurement mode
    TCB1.CTRLA = 0x01;                          // Uses clk with no division
    
    sei();                                      // Enable global interrupt
    
    //PORTA.DIRSET = 0x01;                        // PA0 as output
    
    
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
    
    
     while (1) {
        trigger_pulse(); // Send ultrasonic pulse

        if (newDataReady) { // Only process when new data is available
            newDataReady = false; // Reset flag
            lcd_clrscr();
            for (uint8_t i = 0; i < 5; i++) {
                NumberToAscii(distance[i], cm); // Convert distance to ASCII
                lcd_puts(cm);
            }
        }
    }
}

ISR(TCB1_INT_vect) {
    TCB1.INTFLAGS = 0x01; // Clear interrupt flag
    value[count] = TCB1.CCMP; // Read CNT value
    distance[count] = value[count] / 58; // Convert to cm

    count++;
    if (count >= 5) {
        count = 0;
        newDataReady = true; // Set flag when 5 samples collected
    }
}