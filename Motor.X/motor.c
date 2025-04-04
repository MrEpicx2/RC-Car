/*
 * File:   motor.c
 * Author: UOC
 *
 * Created on March 25, 2025, 3:14 PM
 */


#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    CCP = 0xD8;                 // unlock protected I/O registers - page 41
    CLKCTRL.OSCHFCTRLA = 0x14;  // Clock set to 8 MHz
    while (CLKCTRL.MCLKSTATUS & 0b00000001) {}
    
    TCA0.SINGLE.CTRLA = 0x01;
    TCA0.SINGLE.CTRLB = 0b0110011;
    TCA0.SINGLE.PER = 255;
    TCA0.SINGLE.CMP0 = 64; // 50% duty cycle for WO0
    TCA0.SINGLE.CMP1 = 64;
    
    PORTA.DIRSET = 0b00000011;
    PORTA.PIN1CTRL = 0b10000000;
    
    
}
