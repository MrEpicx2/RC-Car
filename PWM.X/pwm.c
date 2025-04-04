/*
 * File:   pwm.c
 * Author: UOC
 *
 * Created on February 12, 2025, 8:26 AM
 */


#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    CCP = 0xD8;                 // unlock protected I/O registers - page 41
    CLKCTRL.OSCHFCTRLA = 0x14;  // Clock set to 8 MHz
    while (CLKCTRL.MCLKSTATUS & 0b00000001) {}
    
    // **Ensure WO1 is mapped to PA3 instead of default PA5**
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc; 
    // We want to generate a PWM waveform using TCA0 and pin PA0. the frequency will be 1KHz
    
    TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_DIV256_gc) | (TCA_SINGLE_ENABLE_bm); // Divide 8MHz by 256 and enable TCA0
    TCA0.SINGLE.CTRLB = (TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_CMP1EN_bm);  // Enable single slope PWM on CMP0 and CMP1
    TCA0.SINGLE.PER = 625;                      // Frequency set to 50 Hz page 236: (31250/625) 625, being the PER value
    TCA0.SINGLE.CMP0 = 31;                      // Set to 5% duty cycle (31 is left, 47 middle, 63 is left)
    TCA0.SINGLE.CMP1 = 47;                      // 7.5% duty cycle for PA5 (Servo middle)
    PORTA.DIRSET = PIN0_bm;                      // PA0 set as output
    PORTA.DIRSET = PIN4_bm;
    
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
    
    uint16_t result;
    uint16_t servo_position1;
    uint16_t servo_position2;
    
    while (1) {
        if (ADC0.INTFLAGS & 0b00000001) {
            result = (ADC0.RESH << 8) | ADC0.RESL;
            ADC0.INTFLAGS = 0b00000001;
        }
        servo_position1 = 31 + ((result * 32UL) / 4095);
        servo_position2 = 31 + ((result * 32UL) / 4095);  // Example: PA5 moving in a different range

        TCA0.SINGLE.CMP0 = servo_position1;
        TCA0.SINGLE.CMP1 = servo_position2;
        _delay_ms(10);
    }
}
