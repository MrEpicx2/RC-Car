/*
 * File:   avr-main.c
 * Author: UOC
 *
 * Created on March 31, 2025, 2:50 PM
 */


#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>
#include <stdbool.h>

int main(void) {    
    CCP = 0xD8;                 // unlock protected I/O registers - page 41
    CLKCTRL.OSCHFCTRLA = 0x14;  // Clock set to 8 MHz
    while (CLKCTRL.MCLKSTATUS & 0b00000001) {
    }
    
    
    // Servo and ultra sonic trigger on TCA0 
    TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_DIV256_gc) | (TCA_SINGLE_ENABLE_bm); // Divide 8MHz by 256 and enable TCA0
    TCA0.SINGLE.CTRLB = (TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_CMP1EN_bm);  // Enable single slope PWM on CMP0 and CMP1
    TCA0.SINGLE.PER = 625;                      // Frequency set to 50 Hz page 236: (31250/625) 625, being the PER value
    
    // CMP0 will be for trigger and CMP1 will be for the servo
    
    TCA0.SINGLE.CMP0 = 1;                        // Can't get to 10us at this TCA0 frequency, shortest is 1/31250 = 32us. should be fine
    TCA0.SINGLE.CMP1 = 47;                      // 7.5% duty cycle for PA1 (Servo middle)
    
    PORTA.DIRSET = PIN0_bm;                      // PA0 and PA1 set as output
    PORTA.DIRSET = PIN1_bm;
    
    // Motor 1 on TCB0 (PA2) Motor 2 on TCB1 (PA3) and speaker on TCB2 (PC0)
    
    TCB0.CTRLB = (TCB_CNTMODE_PWM8_gc) | (TCB_CCMPEN_bm);   // PWM mode and enable WO on PA2
    TCB1.CTRLB = (TCB_CNTMODE_PWM8_gc) | (TCB_CCMPEN_bm);   // PWM mode and enable WO on PA3
    TCB2.CTRLB = (TCB_CNTMODE_PWM8_gc) | (TCB_CCMPEN_bm);   // PWM mode and enable WO on PC0
    
    // CCMP High byte is the duty and low is the period
    
    TCB0.CCMPL = 255;   // 4MHz / 255 = 15686 Hz      // LOW MUST BE SET FIRST
    TCB0.CCMPH = 64;    // 25% duty

    TCB1.CCMPL = 255;    // 4MHz / 255 = 15686 Hz      
    TCB1.CCMPH = 191;    // 75% duty
    
    TCB2.CCMPL = 50;    // 4MHz / 50 = 10,000 Hz      
    TCB2.CCMPH = 25;    // 50% duty

    
    TCB0.CTRLA = (TCB_CLKSEL_DIV2_gc ) | (TCB_ENABLE_bm);     // 4MHz clk and enable TCB
    TCB1.CTRLA = (TCB_CLKSEL_DIV2_gc ) | (TCB_ENABLE_bm);
    TCB2.CTRLA = (TCB_CLKSEL_DIV2_gc ) | (TCB_ENABLE_bm);
    
    
    PORTA.DIRSET = PIN2_bm;
    PORTA.DIRSET = PIN3_bm;
    PORTC.DIRSET = PIN0_bm;
    while (1) {
    
    }
}

