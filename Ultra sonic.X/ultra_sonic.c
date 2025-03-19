/*
 * File:   newavr-main.c
 * Author: UOC
 *
 * Created on March 12, 2025, 12:54 PM
 */


#include <avr/io.h>
#include <avr/interrupt.h>

uint16_t value[5];
uint8_t count = 0;
uint16_t distance[5];       

int main(void) {
    CCP = 0xD8;                 // unlock protected I/O registers - page 41
    CLKCTRL.OSCHFCTRLA = 0x14;  // Clock set to 8 MHz
    while (CLKCTRL.MCLKSTATUS & 0b00000001) {
    }
    
     TCA0.SINGLE.CTRLA = 0b00001001;         // Counter frequency set to 0.5MHz
    
    // We will manually check the timer and reset the timer
    // so set the period to its max value to avoid an automatic
    // reset.
    TCA0.SINGLE.PER = 0xffff;

    unsigned int timerThresholdOn = 5;
    unsigned int timerThresholdOff = 50;

    PORTA.DIRSET = 0b00000001;
    
    PORTA.DIRSET = 0x01;
    
    EVSYS.CHANNEL0 = 0x41;                      // PORTA pin 1 is set to channel 0, will be the incoming echo signal
    EVSYS.USERTCB2CAPT = 0x01;                  // Select channel 0 for this user (1-1=0)
    
    // TCB2 will be used as the timer to process the echo signal from the ultra sonic sensor
    TCB2.INTCTRL = 0x01;                        // Capture interrupt enabled
    TCB2.EVCTRL = 0x01;                         // Enable input capture event
    TCB2.CTRLB = 0x04;                          // Input capture PW measurement mode
    TCB2.CTRLA = 0x01;                          // Uses clk with no division
    
    sei();                                      // Enable global interrupt
    
    while (1) {
        PORTA.OUT |= 0b00000001;
        
        while( TCA0.SINGLE.CNT <= timerThresholdOn) ;        
        TCA0.SINGLE.CNT = 0;
        
        PORTA.OUT &= 0b11111110;
        
        while( TCA0.SINGLE.CNT <= timerThresholdOff) ;        
        TCA0.SINGLE.CNT = 0;
        
        
        
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
