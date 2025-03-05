/*
 * File:   test.c
 * Author: UOC
 *
 * Created on March 5, 2025, 8:42 AM
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include "util/delay.h"

uint16_t value[5];
uint8_t count = 0;
uint16_t duty = 250;

void trigger_pulse(void) {
    PORTC.OUTSET = PIN1_bm; // Set PC1 high
    TCB0.CTRLA |= TCB_ENABLE_bm; // Start the timer
    while (TCB0.STATUS & TCB_RUN_bm); // Wait for completion
    PORTC.OUTCLR = PIN1_bm; // Clear PC1 (pulse done)
}

int main(void) {
    CCP = 0xD8;                 // unlock protected I/O registers - page 41
    CLKCTRL.OSCHFCTRLA = 0x14;  // Clock set to 8 MHz
    while (CLKCTRL.MCLKSTATUS & 0b00000001) {
    }
    
    TCA0.SINGLE.CTRLA = (0x01) << 3 | 0x01;     // Divide by 16 and enable peripheral
    TCA0.SINGLE.CTRLB = 0x03;                   // single slope PWM
    TCA0.SINGLE.CTRLB |= 0x01 << 4;             // PA0 enabled as single slope
    TCA0.SINGLE.PER = 500-1;                    // Freq set to 1KHz
    TCA0.SINGLE.CMP0 = duty;                    // Duty cycle
    
    
    // 10us timer setup for trigger on PC0
    TCB0.CTRLA = 0x01;                          // Enable TCB0 with the 8MHz CLK
    TCB0.CTRLB = 0x00;                          // Periodic interrupt mode
    TCB0.CCMP = 80;                             // 10us / 125ns (clk period) = 80
    PORTC.DIRSET = 0x01;                        // PC1 as output
    
    EVSYS.CHANNEL2 = 0x41;                      // PORTC pin 1 is set to channel 2, will be the incoming echo signal
    EVSYS.USERTCB1CAPT = 0x03;                  // Select channel 2 for this user (3-1=2)
    
    // TCB1 will be used as the timer to process the echo signal from the ultra sonic sensor
    TCB1.INTCTRL = 0x01;                        // Capture interrupt enabled
    TCB1.EVCTRL = 0x01;                         // Enable input capture event
    TCB1.CTRLB = 0x04;                          // Input capture PW measurement mode
    TCB1.CTRLA = 0x01;                          // Uses clk with no division
    
    
    sei();                                      // Enable global interrupt
    
    PORTA.DIRSET = 0x01;                        // PA0 as output
    
    while (1) {
        _delay_ms(100);
        trigger_pulse();
    }
}

ISR(TCB1_INT_vect) {
        TCB1.INTFLAGS = 0x01;                   // Clear interrupt flag
        TCA0.SINGLE.CMP0 = duty;
        value[count] = TCB1.CCMP;               // Read CNT value
        count++;
        
        if (count >= 5) count = 0;
    }