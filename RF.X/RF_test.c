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
    
    TCA0.SINGLE.CMP0 = 1;                          // Can't get to 10us at this TCA0 frequency, shortest is 1/31250 = 32us. should be fine
    TCA0.SINGLE.CMP1 = 4800 / 100;                 // divide to avoid using decimals the compiler won't keep them
                                                   // min = 2.75% (17.18), +45° from min = 5.13% (32.03), 0° = 7.5% (46) but 48 is more accurate, +45° from 0 = 10.25% (64.06)
                                                   // max = 13% (81.25) I will put these in a header file later to make it easier to use
    
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
    
    // RF set up
    
    PORTA.DIRSET = 0b11110000;  // PA4-7 as output
    PORTD.DIRCLR = 0b00001111;  // PD1-4 as input
    
    int8_t servo_pos = 0;
    
    while (1) {
       if (PORTD.IN & PIN1_bm) {        // servo right
           PORTA.OUTSET = PIN4_bm;
           _delay_ms(2000);
           PORTA.OUTCLR = PIN4_bm;
           
           if (servo_pos < 5) {
               switch (servo_pos) {
                   case 0:
                       TCA0.SINGLE.CMP1 = 1718 / 100;   // min
                       break;
                   case 1:
                       TCA0.SINGLE.CMP1 = 3203 / 100;   // min + 45°
                       break;
                   case 2:
                       TCA0.SINGLE.CMP1 = 4800 / 100;   // 0
                       break;
                   case 3:
                       TCA0.SINGLE.CMP1 = 6406 / 100;   // 0 + 45°
                       break;
                   case 4:
                       TCA0.SINGLE.CMP1 = 8125 / 100;   // max
                       break;
               }
               servo_pos++;
               if (servo_pos == 5) servo_pos = 4;
            }
       }
       if (PORTD.IN & PIN2_bm) {    // servo left
           PORTA.OUTSET = PIN5_bm;
           _delay_ms(2000);
           PORTA.OUTCLR = PIN5_bm;
           
           if (servo_pos >= 0) {
               switch (servo_pos) {
                   case 0:
                       TCA0.SINGLE.CMP1 = 1718 / 100;   // min
                       break;
                   case 1:
                       TCA0.SINGLE.CMP1 = 3203 / 100;   // min + 45°
                       break;
                   case 2:
                       TCA0.SINGLE.CMP1 = 4800 / 100;   // 0
                       break;
                   case 3:
                       TCA0.SINGLE.CMP1 = 6406 / 100;   // 0 + 45°
                       break;
                   case 4:
                       TCA0.SINGLE.CMP1 = 8125 / 100;   // max
                       break;
               }
               servo_pos--;
               if (servo_pos == -1) servo_pos = 0;
            }
       } 
       
       if (PORTD.IN & PIN3_bm) {
           PORTA.OUTSET = PIN6_bm;
           _delay_ms(2000);
           PORTA.OUTCLR = PIN6_bm;
       } 
       
       if (PORTD.IN & PIN4_bm) {
           PORTA.OUTSET = PIN7_bm;
           _delay_ms(2000);
           PORTA.OUTCLR = PIN7_bm;
        } 
        
    }
}
