/*
 * File:   avr-main.c
 * Author: UOC
 *
 * Created on March 31, 2025, 2:50 PM
 */


#include <avr/io.h>
#include "avr-main.h"
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

uint16_t value[5];
uint8_t count = 0;
uint16_t distance[5];


void bwd(void* mode) {
    if ((uint8_t)(uintptr_t) mode == 1) {
        if (bwd_on == false) {
            for (uint8_t duty = 102; duty <= 254; duty++) {
                TCB0.CCMPH = duty;
                _delay_ms(10);
            }
            bwd_on = true;
        }
        else if (bwd_on == true)   TCB0.CCMPH = 255; 
    }
    
    else if ((uint8_t)(uintptr_t) mode == 0){
        TCB0.CCMPH = 0;
        bwd_on = false;
    }
}

void fwd(void* mode) {
    if ((uint8_t)(uintptr_t) mode == 1) {
        if (fwd_on == false) {
            for (uint8_t duty = 102; duty <= 254; duty++) {
                TCB1.CCMPH = duty;
                _delay_ms(10);
            }
            fwd_on = true;
        }
        else if (fwd_on == true)   TCB1.CCMPH = 255; 
    }
    
    else if ((uint8_t)(uintptr_t) mode == 0){
        TCB1.CCMPH = 0;
        fwd_on = false;
    }
}

void servo_right(void* unused){
    TCA0.SINGLE.CMP1 = 1719 / 100;
    /*
    for (uint16_t servo_pos = 4800; servo_pos >= 1719; servo_pos--) {
        TCA0.SINGLE.CMP1 = servo_pos / 100;
        _delay_ms(1);
    }*/
}

void servo_left(void* unused) {
    TCA0.SINGLE.CMP1 = 8124 / 100;
    /*
    for (uint16_t servo_pos = 4800; servo_pos <= 8124; servo_pos++) {
        TCA0.SINGLE.CMP1 = servo_pos / 100;
        _delay_ms(1);
    }*/
}

void fwd_left(void* mode) {
    servo_left(NULL);
    fwd(mode);
    
}

void fwd_right(void* mode) {
    servo_right(NULL);
    fwd(mode);
}

void bwd_left(void* mode) {
    servo_left(NULL);
    bwd(mode);
}

void bwd_right(void* mode) {
    servo_right(NULL);
    bwd(mode);
}

bool is_signal_combo_active(SignalCombo sig) {
    bool port1_check_1 = sig.port1 ? ((*sig.port1 & sig.mask1) == sig.mask1) : true;
    bool port2_check_1 = sig.port2 ? ((*sig.port2 & sig.mask2) == sig.mask2) : true;

    bool port1_check_2 = sig.port1 ? ((*sig.port1 & sig.mask1) == sig.mask1) : true;
    bool port2_check_2 = sig.port2 ? ((*sig.port2 & sig.mask2) == sig.mask2) : true;

    return (port1_check_1 && port2_check_1 && port1_check_2 && port2_check_2);
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
    
    
    //*********************************************Screen SDA PC2 and SCL PC3 in i2c_avr128db28.c driver*************************************************************************
    
    /*
    i2c_init(); // Initialize I2C

    _delay_ms(500); // Give I2C some time before checking
    
    lcd_init();
    lcd_clrscr();  // **Clear screen before writing**
    lcd_gotoxy(0, 0);  // **Ensure cursor is at the start**
    
    lcd_puts("Hello World1");  // Put string from RAM
    lcd_gotoxy(0, 2);         // Move cursor to line 3
    lcd_puts_p(PSTR("String from flash")); 
    _delay_ms(3000);
    lcd_clrscr();
     */
    
    //**************************************************************************ADC set up****************************************************************************************
    
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
    
    //Ultrasonic set up
    
    //Receiver set up
    
    PORTA.PIN5CTRL |= PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL |= PORT_PULLUPEN_bm;
    
    PORTD.PIN4CTRL |= PORT_PULLUPEN_bm;
    PORTD.PIN5CTRL |= PORT_PULLUPEN_bm;
    
    PORTA.DIRCLR = PIN5_bm | PIN6_bm;      // PA5 and PA6 as incoming receiver signal D1 and D0 respectivley
    PORTD.DIRCLR = PIN3_bm | PIN4_bm;      // PD4 and PD5 as incoming receiver signal D3 and D2 respectivley
    
    //**********************************************************************LOCALS**************************************************************************************
    
    PORTA.DIRSET = PIN7_bm;
    
    ActionFunction output = NULL;
    void* output_arg = NULL;
    
    while (1) {
        output = NULL;
        output_arg = NULL;
        
        if (is_signal_combo_active(BACKWARD_LEFT)) {
            PORTA.OUTSET = PIN7_bm;
            output = bwd_left;
            output_arg = (void*)1;
        }
        
        else if (is_signal_combo_active(FORWARD_RIGHT)) {
            PORTA.OUTSET = PIN7_bm;
            output = fwd_right;
            output_arg = (void*)1;
        }
        
        else if (is_signal_combo_active(FORWARD_LEFT)) {
            PORTA.OUTSET = PIN7_bm;
            output = fwd_left;
            output_arg = (void*)1;
        }
        
        else if (is_signal_combo_active(LEFT)) {
            PORTA.OUTSET = PIN7_bm;
            output = servo_left;
            output_arg = NULL;
        }
        
        else if (is_signal_combo_active(RIGHT_LIGHT)) {
            PORTA.OUTSET = PIN7_bm;
            output = NULL;
        }
        
        else if (is_signal_combo_active(LEFT_LIGHT)) {
            PORTA.OUTSET = PIN7_bm;
            output = NULL;
            output_arg = NULL;
        }
        
        else if (is_signal_combo_active(FORWARD)) {
            PORTA.OUTSET = PIN7_bm;
            output = fwd;
            output_arg = (void*)1;
        }
        
        else if (is_signal_combo_active(BACKWARD)) {
            PORTA.OUTSET = PIN7_bm;
            output = bwd;
            output_arg = (void*)1;
        }
        
        
        else if (is_signal_combo_active(RIGHT)) {
            PORTA.OUTSET = PIN7_bm;
            output = servo_right;
            output_arg = NULL;
        }
        

        else if (is_signal_combo_active(BACKWARD_RIGHT)) {
            PORTA.OUTSET = PIN7_bm;
            output = bwd_right;
            output_arg = (void*)1;
        }
        
        if (output != NULL) output(output_arg);     // Call whatever function the signal is
        
        
        else if (output == NULL) {
            PORTA.OUTCLR = PIN7_bm;
            output_arg = NULL;
            output = fwd;
            output(output_arg);
            output = bwd;
            output(output_arg);
            TCA0.SINGLE.CMP1 = 4800 / 100;          // reset servo
            output = NULL;      
        }
        _delay_ms(10);  // debounce delay
    }
}