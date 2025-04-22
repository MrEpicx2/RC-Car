/*
 * File:   avr-main.c
 * Author: UOC
 *
 * Created on March 31, 2025, 2:50 PM
 */


#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include "Reciever.h"


bool is_signal_combo_active(SignalCombo sig) {
    /*
     Check if a port is null and if it's not return the value of the pin
     If it's null return true so the other port isn't blocked
     */
    bool port1_check = sig.port1 ? ((*sig.port1 & sig.mask1) == sig.mask1) : true;
    bool port2_check = sig.port2 ? ((*sig.port2 & sig.mask2) == sig.mask2) : true;
    return port1_check && port2_check;
}



int main(void) {
    CCP = 0xD8;                 // unlock protected I/O registers - page 41
    CLKCTRL.OSCHFCTRLA = 0x14;  // Clock set to 8 MHz
    while (CLKCTRL.MCLKSTATUS & 0b00000001) {
    }
    
    PORTA.DIRSET = 0b00001111;
    PORTA.DIRCLR = 0b01100000;
    PORTD.DIRCLR = 0b00110000;
    
    uint16_t dead_time = 1000;
    uint8_t output = 0;
            
    while (1) {   
        if (is_signal_combo_active(FORWARD)) {
            output = PIN0_bm;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(BACKWARD)) {
            output = PIN1_bm;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(LEFT)) {
            output = PIN0_bm | PIN1_bm;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(RIGHT)) {
            output = PIN2_bm;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(FORWARD_LEFT)) {
            output = PIN0_bm | PIN2_bm;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(FORWARD_RIGHT)) {
            output = PIN1_bm | PIN2_bm;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(BACKWARD_LEFT)) {
            output = PIN0_bm | PIN1_bm | PIN2_bm;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(BACKWARD_RIGHT)) {
            output = PIN3_bm;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(RIGHT_LIGHT)) {
            output = PIN0_bm | PIN3_bm;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(LEFT_LIGHT)) {
            output = PIN1_bm | PIN3_bm;
            dead_time = 0;
        }
        PORTA.OUTSET = output;
        
        
        if (!(PORTA.IN & 0x60) && !(PORTD.IN & 0x30)) {
            dead_time++;
            if (dead_time > 1000) {
                PORTA.OUTCLR = 0b00001111;
                dead_time = 1000;
                output = 0;
            }          
        }   
    }
}
