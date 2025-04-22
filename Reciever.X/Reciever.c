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

SignalAction signal_actions[] = {
    { FORWARD,        fwd,  (void*)1 },
    { BACKWARD,       bwd,  (void*)1 },
    { LEFT,           servo_left,  NULL },
    { RIGHT,          servo_right, NULL },
    { FORWARD_LEFT,   NULL, NULL },  // Placeholder for future
    { FORWARD_RIGHT,  NULL, NULL },
    { BACKWARD_LEFT,  NULL, NULL },
    { BACKWARD_RIGHT, NULL, NULL },
    { RIGHT_LIGHT,    NULL, NULL },
    { LEFT_LIGHT,     NULL, NULL }
};

const size_t NUM_ACTIONS = sizeof(signal_actions) / sizeof(signal_actions[0]);


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
        /*
    }
        bool found_active = false;
        
        for (size_t i = 0; i < NUM_ACTIONS; ++i) {
            if (is_signal_combo_active(signal_actions[i].combo)) {
                if (signal_actions[i].func) {
                    signal_actions[i].func(signal_actions[i].arg);
                }
                dead_time = 0;
                found_active = true;
                break; // Exit loop on first match
            }
        }

        if (!found_active) {
            dead_time++;
            if (dead_time > 1000) {
                fwd((void*)0);
                bwd((void*)0);
                servo_zero(NULL);
                dead_time = 1000;
            }
        }
        */
        
        
        
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
