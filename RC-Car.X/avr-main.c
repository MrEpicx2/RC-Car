/*
 * File:   avr-main.c
 * Author: UOC
 *
 * Created on March 31, 2025, 2:50 PM
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#include "font.h"
#include "lcd.h"
#include "i2c_avr128db28.h"
#include "Reciever.h"

//***********************************************GLOBALS******************************************************** 

bool fwd_on = false;
bool bwd_on = false;


void bwd(uint8_t mode) {
    if (mode == 1) {
        TCB0.CCMPH = 102;

        for (uint8_t duty = 102; duty <= 254; duty++) {
            TCB0.CCMPH = duty;
            _delay_ms(10);
        }
        bwd_on = true;
    }
    
    else if (mode == 0){
        TCB0.CCMPH = 0;
        bwd_on = false;
    }
}

void fwd(uint8_t mode) {
    if (mode == 1) {
        TCB1.CCMPH = 102;

        for (uint8_t duty = 102; duty <= 254; duty++) {
            TCB1.CCMPH = duty;
            _delay_ms(10);
        }
        fwd_on = true;
    }
    
    else if (mode == 0){
        TCB1.CCMPH = 0;
        fwd_on = false;
    }
}

void servo_right(void){
    for (uint16_t servo_pos = 4800; servo_pos >= 1719; servo_pos--) {
        TCA0.SINGLE.CMP1 = servo_pos / 100;
        _delay_ms(1);
    }
}

void servo_left(void) {
    for (uint16_t servo_pos = 4800; servo_pos <= 8124; servo_pos++) {
        TCA0.SINGLE.CMP1 = servo_pos / 100;
        _delay_ms(1);
    }
}

int main(void) {    
    CCP = 0xD8;                 // unlock protected I/O registers - page 41
    CLKCTRL.OSCHFCTRLA = 0x14;  // Clock set to 8 MHz
    while (CLKCTRL.MCLKSTATUS & 0b00000001) {
    }
    
    
    // ******************************************Servo and ultra sonic trigger on TCA0******************************************************************************************* 
    TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_DIV256_gc) | (TCA_SINGLE_ENABLE_bm); // Divide 8MHz by 256 and enable TCA0
    TCA0.SINGLE.CTRLB = (TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_CMP1EN_bm);  // Enable single slope PWM on CMP0 and CMP1
    TCA0.SINGLE.PER = 625;                      // Frequency set to 50 Hz page 236: (31250/625) 625, being the PER value
    
    // CMP0 will be for trigger and CMP1 will be for the servo
    
    TCA0.SINGLE.CMP0 = 1;                          // Can't get to 10us at this TCA0 frequency, shortest is 1/31250 = 32us. should be fine
    TCA0.SINGLE.CMP1 = 4800 / 100;                 // divide to avoid using decimals the compiler won't keep them
                                                   // min = 2.75% (17.18), +45° from min = 5.13% (32.03), 0° = 7.5% (46) but 48 is more accurate, +45° from 0 = 10.25% (64.06)
                                                   // max = 13% (81.25) I will put these in a header file later to make it easier to use
    
    PORTA.DIRSET = PIN0_bm;                      // PA0 and PA1 set as output
    PORTA.DIRSET = PIN1_bm;
    
    
    // **********************************************Motor 1 on TCB0 (PA2) Motor 2 on TCB1 (PA3) and speaker on TCB2 (PC0)*******************************************************
    
    TCB0.CTRLB = (TCB_CNTMODE_PWM8_gc) | (TCB_CCMPEN_bm);   // PWM mode and enable WO on PA2 H-
    TCB1.CTRLB = (TCB_CNTMODE_PWM8_gc) | (TCB_CCMPEN_bm);   // PWM mode and enable WO on PA3 H+
    TCB2.CTRLB = (TCB_CNTMODE_PWM8_gc) | (TCB_CCMPEN_bm);   // PWM mode and enable WO on PC0
    
    // CCMP High byte is the duty and low is the period
    
    TCB0.CCMPL = 255;   // 4MHz / 255 = 15686 Hz      // LOW MUST BE SET FIRST
    TCB0.CCMPH = 0;     // duty

    TCB1.CCMPL = 255;    // 4MHz / 255 = 15686 Hz      
    TCB1.CCMPH = 0;      // duty
    
    TCB2.CCMPL = 50;    // 4MHz / 50 = 10,000 Hz      
    TCB2.CCMPH = 25;    // duty

    
    TCB0.CTRLA = (TCB_CLKSEL_DIV2_gc ) | (TCB_ENABLE_bm);     // 4MHz clk and enable TCB
    TCB1.CTRLA = (TCB_CLKSEL_DIV2_gc ) | (TCB_ENABLE_bm);
    TCB2.CTRLA = (TCB_CLKSEL_DIV2_gc ) | (TCB_ENABLE_bm);
    
    
    PORTA.DIRSET = PIN2_bm;
    PORTA.DIRSET = PIN3_bm;
    PORTC.DIRSET = PIN0_bm;
    
    // RF set up
    
    PORTA.DIRSET = 0b11110000;  // PA4-7 as output
    PORTD.DIRCLR = 0b00001111;  // PD1-4 as input
    
    _delay_ms(2000);            // Delay because receiver sends a=out a signal when turned on
            
    while (1) {
        
    }
}
