/*
 * File:   newavr-main.c
 * Author: UOC
 *
 * Created on March 12, 2025, 12:54 PM
 */


#include <avr/io.h>
#include <avr/interrupt.h>

uint16_t value[5];
uint8_t count = 0;
uint16_t distance[5];       

int main(void) {
    CCP = 0xD8;                 // unlock protected I/O registers - page 41
    CLKCTRL.OSCHFCTRLA = 0x14;  // Clock set to 8 MHz
    while (CLKCTRL.MCLKSTATUS & 0b00000001) {
    }
    
    // ******************************************Servo and ultra sonic trigger on TCA0******************************************************************************************* 
    TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_DIV256_gc) | (TCA_SINGLE_ENABLE_bm); // Divide 8MHz by 256 and enable TCA0
    TCA0.SINGLE.CTRLB = (TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm);  // Enable single slope PWM on CMP0 and CMP1
    TCA0.SINGLE.PER = 625;                      // Frequency set to 50 Hz page 236: (31250/625) 625, being the PER value
    
    // CMP0 will be for trigger and CMP1 will be for the servo
    
    TCA0.SINGLE.CMP0 = 1;                          // Can't get to 10us at this TCA0 frequency, shortest is 1/31250 = 32us. should be fine

    PORTA.DIRSET = PIN0_bm;
    PORTD.DIRSET = PIN7_bm;
    PORTC.DIRCLR = PIN1_bm;
    PORTC.PIN1CTRL = PORT_ISC_BOTHEDGES_gc;
    
    /*
    EVSYS.CHANNEL0 = 0x41;                      // PORTA pin 1 is set to channel 0, will be the incoming echo signal
    EVSYS.USERTCB2CAPT = 0x01;                  // Select channel 0 for this user (1-1=0)
    
    // TCB2 will be used as the timer to process the echo signal from the ultra sonic sensor
    TCB2.INTCTRL = 0x01;                        // Capture interrupt enabled
    TCB2.EVCTRL = 0x01;                         // Enable input capture event
    TCB2.CTRLB = 0x04;                          // Input capture PW measurement mode
    TCB2.CTRLA = 0x01;                          // Uses clk with no division
    */
    
    EVSYS.CHANNEL2 = EVSYS_CHANNEL2_PORTC_PIN1_gc;  // Route PC1 to Channel 0
    EVSYS.USERTCD0INPUTA = EVSYS_USER_CHANNEL2_gc;
    
    TCD0.EVCTRLA = TCD_TRIGEI_bm | TCD_ACTION_bm; // Capture on Event A and enable it
    TCD0.EVCTRLB = TCD_TRIGEI_bm | TCD_ACTION_bm;
    TCD0.INPUTCTRLA = 0x08;       //edge triggered
    TCD0.INPUTCTRLB = 0x08;
    TCD0.INTCTRL = TCD_TRIGA_bm | TCD_TRIGB_bm;  // Enable interrupt on TRIGA (capture A)
    TCD0.CTRLA = TCD_ENABLE_bm;
    
    sei();                                      // Enable global interrupt
    
    while (1) { 
    }
}

ISR(TCD0_TRIG_vect) {
    static uint16_t start_time = 0;

    // Rising edge: echo pulse started
    if (TCD0.INTFLAGS & TCD_TRIGA_bm) {
        TCD0.INTFLAGS = TCD_TRIGA_bm;
        start_time = (TCD0.CAPTUREAH << 8) | TCD0.CAPTUREAL;
        PORTD.OUTSET = PIN7_bm; // Optional: turn LED on
    }

    // Falling edge: echo pulse ended
    if (TCD0.INTFLAGS & TCD_TRIGB_bm) {
        TCD0.INTFLAGS = TCD_TRIGB_bm;
        uint16_t end_time = (TCD0.CAPTUREBH << 8) | TCD0.CAPTUREBL;
        PORTD.OUTCLR = PIN7_bm; // Optional: turn LED off

        uint16_t pulse_width;
        if (end_time >= start_time) {
            pulse_width = end_time - start_time;
        } else {
            // Handle timer overflow (16-bit wraparound)
            pulse_width = (0xFFFF - start_time + end_time + 1);
        }

        // Convert to cm (pulse width in microseconds / 58 ? distance in cm)
        // Each tick = 1 / 8 MHz = 0.125 µs ? pulse_width * 0.125 = time in µs
        // distance = time / 58 = (pulse_width * 0.125) / 58
        float time_us = pulse_width * 0.125f;
        float distance_cm = time_us / 58.0f;

        // Optionally store the result
        value[count] = pulse_width;
        distance[count] = (uint16_t)distance_cm;

        count++;
        if (count >= 5) count = 0;
    }
}

