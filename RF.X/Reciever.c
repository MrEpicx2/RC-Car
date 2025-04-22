/*
 * File:   avr-main.c
 * Author: UOC
 *
 * Created on March 31, 2025, 2:50 PM
 */


#include <avr/io.h>
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

void bwd(void* mode) {
    if (mode == 1) {
        PORTA.OUTSET = PIN1_bm;
    }
    
    else if (mode == 0) {
        PORTA.OUTCLR = PIN1_bm;
    }
}

void fwd(void* mode) {
    if (mode == 1) {
        PORTA.OUTSET = PIN0_bm;
    }
    
    else if (mode == 0) {
        PORTA.OUTCLR = PIN0_bm;
    }
}

void servo_left(void* unused) {
    PORTA.OUTSET = PIN2_bm;
}


void servo_right(void* unused) {
    PORTA.OUTSET = PIN3_bm;
}

void servo_zero(void* unused) {
    PORTA.OUTCLR = PIN2_bm | PIN3_bm;
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
    ActionFunction output = NULL;
    void* output_arg = NULL;
            
    while (1) {   
        if (is_signal_combo_active(FORWARD)) {
            output = fwd;
            output_arg = (void*)1;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(BACKWARD)) {
            output = bwd;
            output_arg = (void*)1;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(LEFT)) {
            output = servo_left;
            output_arg = NULL;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(RIGHT)) {
            output = servo_right;
            output_arg = NULL;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(FORWARD_LEFT)) {
            output = NULL;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(FORWARD_RIGHT)) {
            output = NULL;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(BACKWARD_LEFT)) {
            output = NULL;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(BACKWARD_RIGHT)) {
            output = NULL;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(RIGHT_LIGHT)) {
            output = NULL;
            dead_time = 0;
        }
        
        if (is_signal_combo_active(LEFT_LIGHT)) {
            output = NULL;
            dead_time = 0;
        }
        
        output(output_arg);     // Call whatever function the signal is
        
        if (!(PORTA.IN & 0x60) && !(PORTD.IN & 0x30)) {
            dead_time++;
            if (dead_time > 1000) {
                output_arg = NULL;
                output = fwd;
                output(output_arg);
                output = bwd;
                output(output_arg);
                output = servo_zero;
                output(output_arg);
                PORTA.OUTCLR = PIN0_bm | PIN1_bm;
                dead_time = 1000;
            }          
        }   
    }
}
