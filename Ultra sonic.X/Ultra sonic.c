/*
 * File:   Ultra sonic.c
 * Author: UOC
 *
 * Created on February 26, 2025, 10:02 AM
 */


#include <avr/io.h>

#include <avr/io.h>
#include <util/delay.h>
#include "i2c_avr128db28.h"
#include "lcd.h"

void init_TCB2_for_Trigger() {
    TCB2.CTRLA = TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm; // Clock source: No prescaler
    TCB2.CTRLB = TCB_CNTMODE_SINGLE_gc;             // Single-shot mode
    TCB2.CCMP = (F_CPU / 1000000) * 10;             // 10µs pulse width
    PORTA.DIRSET = PIN1_bm;                         // Set PA1 (Trigger) as output
}

void send_trigger_pulse() {
    PORTA.OUTSET = PIN1_bm;  // Set PA1 HIGH
    TCB2.CTRLA |= TCB_ENABLE_bm;  // Start single-shot timer
    while (!(TCB2.INTFLAGS & TCB_OVF_bm));  // Wait for pulse to complete
    TCB2.INTFLAGS = TCB_OVF_bm;  // Clear flag after done
    PORTA.OUTCLR = PIN1_bm;  // Set PA1 LOW
}

void init_TCB1_for_Echo() {
    PORTA.DIRCLR = PIN2_bm;   // PA2 (Echo) as input
    TCB1.CTRLA = TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm;
    TCB1.CTRLB = TCB_CNTMODE_FRQ_gc;  // Capture Frequency mode
    TCB1.INTCTRL = TCB_CAPT_bm;       // Enable capture interrupt
}

volatile int echo_time = 0;

ISR(TCB1_INT_vect) {
    echo_time = TCB1.CCMP;   // Capture echo pulse width
    TCB1.INTFLAGS = TCB_CAPT_bm;  // Clear interrupt flag
}

int main() {
    i2c_init(); // Initialize I2C

    _delay_ms(500); // Give I2C some time before checking
    
    lcd_init();
    lcd_clrscr();  // **Clear screen before writing**
    lcd_gotoxy(0, 0);  // **Ensure cursor is at the start**
    
    init_TCB2_for_Trigger();
    init_TCB1_for_Echo();

    while (1) {
        send_trigger_pulse();
        _delay_ms(100); // Wait for response
        int distance_cm = (echo_time * 0.0343) / 2;
        lcd_puts(distance_cm);
        // Use `distance_cm` for further processing
    }
}

