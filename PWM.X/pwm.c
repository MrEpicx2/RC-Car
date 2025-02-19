/*
 * File:   pwm.c
 * Author: UOC
 *
 * Created on February 12, 2025, 8:26 AM
 */


#include <avr/io.h>

int main(void) {
    CCP = 0XD8;                 // unlock protected I/O registers
    CLKCTRL.OSCHFCTRLA = 0x14;  // Clock set to 8 MHz
    while (CLKCTRL.MCLKSTATUS & 0b00000001) {
        
    }
    // We want to generate a PWM waveform using TCA0 and pin PA0. the frequency will be 1KHz
    
    TCA0.SINGLE.CTRLA = (0x01) << 3 | 0x01;     //Divide by 16 and enable perferal
    TCA0.SINGLE.CTRLB = 0x03;                   // Single slope PWM
    TCA0.SINGLE.CTRLB |= 0x01 << 4;             // PA0 enable as single slope PWM
    TCA0.SINGLE.PER = 500;                      // Frequency set to 1 KHz
    TCA0.SINGLE.CMP0 = 250;                     // Set to 50% duty cycle
    
    PORTA.DIRSET = 0x01;                        // PA0 set as output
    while (1) {
    }
}
