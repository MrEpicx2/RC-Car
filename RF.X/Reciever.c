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

typedef struct {
    bool current;
    bool previous;
} ButtonState;

bool is_signal_combo_active(SignalCombo sig) {
    /*
     Check if a port is null and if it's not return the value of the pin
     If it's null return true so the other port isn't blocked
     */
    bool port1_check = sig.port1 ? (*sig.port1 & sig.mask1) : true;
    bool port2_check = sig.port2 ? (*sig.port2 & sig.mask2) : true;
    return port1_check && port2_check;
}


int main(void) {
    PORTA.DIRSET = 0b00001111;
    PORTA.DIRCLR = 0b01100000;
    PORTD.DIRCLR = 0b00110000;
    while (1) {   
        if (is_signal_combo_active(FORWARD))        PORTA.OUT = PIN0_bm;
        
        if (is_signal_combo_active(BACKWARD))       PORTA.OUT = PIN1_bm;
        
        if (is_signal_combo_active(LEFT))           PORTA.OUT = PIN0_bm | PIN1_bm;
        
        if (is_signal_combo_active(RIGHT))          PORTA.OUT = PIN2_bm;
        
        if (is_signal_combo_active(FORWARD_LEFT))   PORTA.OUT = PIN0_bm | PIN2_bm;
        
        if (is_signal_combo_active(FORWARD_RIGHT))  PORTA.OUT = PIN1_bm | PIN2_bm;
        
        if (is_signal_combo_active(BACKWARD_LEFT))  PORTA.OUT = PIN0_bm | PIN1_bm | PIN2_bm;
        
        if (is_signal_combo_active(BACKWARD_RIGHT)) PORTA.OUT = PIN3_bm;
        
        if (is_signal_combo_active(RIGHT_LIGHT))    PORTA.OUT = PIN0_bm | PIN3_bm;
        
        if (is_signal_combo_active(LEFT_LIGHT))     PORTA.OUT = PIN1_bm | PIN3_bm;
        
        _delay_ms(2000);
        PORTA.OUTCLR = 0b00001111;
        _delay_ms(2000);

        
    }
}
