/*
 * File:   ADC.c
 * Author: UOC
 *
 * Created on February 12, 2025, 9:19 AM
 */


#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    
    // Enable global interrupts.
    SREG = 0b10000000;
    
    // Set the ADC reference level to VDD.
    VREF.ADC0REF = 0b10000101;
    
    // Enable the ADC interrupt.
    ADC0.INTCTRL = 0b00000001;
    
    // Select PD2 (AIN2) as the ADC input.
    ADC0.MUXPOS = 0x02;

    // Select minimum clock divide.
    ADC0.CTRLC = 0x00;
    
    // Select single ended mode, 12 bit resolution and free-running modes.
    ADC0.CTRLA = 0b00000011;
    
    // Start conversion.
    ADC0.COMMAND = 0x01;
    

    int result_low = 0;
    int result_high = 0;
    
    while (1) {
        if (ADC0.INTFLAGS & 0b00000001) {
            result_low = ADC0.RESL;
            result_high = ADC0.RESH;
            ADC0.INTFLAGS = 0b00000001;
        }
    }
}
