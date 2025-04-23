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
#include <string.h>

#include "font.h"
#include "lcd.h"
#include "i2c_avr128db28.h"
#include "Reciever.h"

//***********************************************GLOBALS******************************************************** 

bool fwd_on = false;
bool bwd_on = false;

bool right_light = false;
bool left_light = false;

bool tcb2_switch = false;

volatile uint16_t value[5];
volatile uint8_t count = 0;
volatile uint16_t distance[5];

ActionFunction output = NULL;
void* output_arg = NULL;

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

void R_light(void* unused) {
    if (right_light == false) {
        right_light = true;
        PORTD.OUTSET = PIN6_bm;
    }
    
    else if (right_light == true) {
        right_light = false;
        PORTD.OUTCLR = PIN6_bm;
    }
}

void L_light(void* unused) {
    if (left_light == false) {
        left_light = true;
        PORTD.OUTSET = PIN7_bm;
    }
    
    else if (left_light == true) {
        left_light = false;
        PORTD.OUTCLR = PIN7_bm;
    }
}

void tcb_sw(void* unused) {
    if (tcb2_switch == false) {
        TCB2.INTCTRL = 0x01;                        // Capture interrupt enabled
        TCB2.EVCTRL = 0x01;                         // Enable input capture event
        TCB2.CTRLB = 0x04;                          // Input capture PW measurement mode
        TCB2.CTRLA = 0x01;                          // Uses clk with no division
        PORTD.OUTSET = PIN6_bm;
        PORTD.OUTSET = PIN7_bm;
        tcb2_switch = true;
    }
    
    else if (tcb2_switch == true) {
        TCB2.CTRLB = (TCB_CNTMODE_PWM8_gc) | (TCB_CCMPEN_bm);   // PWM mode and enable WO on PC0
        TCB2.CCMPL = 31;    // 31250Hz / 10 = 10,000 Hz      
        TCB2.CCMPH = 0;    // duty
        TCB2.CTRLA = (TCB_CLKSEL_TCA0_gc ) | (TCB_ENABLE_bm);       // 31250 Hz from TCA
        PORTD.OUTCLR = PIN6_bm;
        PORTD.OUTCLR = PIN7_bm;
        tcb2_switch = false;
    }
}

uint8_t NumberToAscii(uint32_t u32Number_, uint8_t* pu8AsciiString_)
{
  bool bFoundDigit = false;
  uint8_t au8AsciiNumber[11];
  uint8_t u8CharCount = 0;
  uint32_t u32Divider = 1000000000;
  uint8_t u8Temp;
  
  /* Parse out all the digits, start counting after leading zeros */
  for(uint8_t i = 0; i < 10; i++)
  {
    u8Temp = (u32Number_ / u32Divider);
    
    /* Check for first non-leading zero */
    if(u8Temp != 0)
    {
      bFoundDigit = true;
    }

    /* As long as a non-leading zero has been found, add the ASCII char */
    if(bFoundDigit)
    {
      au8AsciiNumber[u8CharCount] = u8Temp + 0x30;
      u8CharCount++;
    }
    
    /* Update for next iteration */
    u32Number_ %= u32Divider;
    u32Divider /= 10;
  }
  
  /* Handle special case where u32Number == 0 */
  if(!bFoundDigit)
  {
    u8CharCount = 1;
    au8AsciiNumber[0] = '0';
  }
  
  /* Add the null and copy to destination */
  au8AsciiNumber[u8CharCount] = '\0';
  strcpy((char *)pu8AsciiString_, (const char*)au8AsciiNumber);
  
  return(u8CharCount);
}

bool is_signal_combo_active(SignalCombo sig) {
    bool port1_check_1 = sig.port1 ? ((*sig.port1 & sig.mask1) == sig.mask1) : true;
    bool port2_check_1 = sig.port2 ? ((*sig.port2 & sig.mask2) == sig.mask2) : true;

    bool port1_check_2 = sig.port1 ? ((*sig.port1 & sig.mask1) == sig.mask1) : true;
    bool port2_check_2 = sig.port2 ? ((*sig.port2 & sig.mask2) == sig.mask2) : true;

    return (port1_check_1 && port2_check_1 && port1_check_2 && port2_check_2);
}

void signal_check(void) {
    
        output = NULL;
        output_arg = NULL;
        
        if (is_signal_combo_active(TCB2_SWITCH)) {
            PORTA.OUTSET = PIN7_bm;
            output = tcb_sw;
            output_arg = NULL;
        }
        
        else if (is_signal_combo_active(BACKWARD_LEFT)) {
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
            output = R_light;
            output_arg = NULL;
        }
        
        else if (is_signal_combo_active(LEFT_LIGHT)) {
            PORTA.OUTSET = PIN7_bm;
            output = L_light;
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
    //TCB2.CTRLB = (TCB_CNTMODE_PWM8_gc) | (TCB_CCMPEN_bm);   // PWM mode and enable WO on PC0
    
    // CCMP High byte is the duty and low is the period
    
    TCB0.CCMPL = 255;   // 4MHz / 255 = 15686 Hz      // LOW MUST BE SET FIRST
    TCB0.CCMPH = 0;     // duty

    TCB1.CCMPL = 255;    // 4MHz / 255 = 15686 Hz      
    TCB1.CCMPH = 0;      // duty
    
    //TCB2.CCMPL = 31;    // 31250Hz / 10 = 10,000 Hz      
    //TCB2.CCMPH = 0;    // duty

    
    TCB0.CTRLA = (TCB_CLKSEL_DIV2_gc ) | (TCB_ENABLE_bm);       // 4MHz clk and enable TCB
    TCB1.CTRLA = (TCB_CLKSEL_DIV2_gc ) | (TCB_ENABLE_bm);
    //TCB2.CTRLA = (TCB_CLKSEL_TCA0_gc ) | (TCB_ENABLE_bm);       // 31250 Hz from TCA
    
    
    PORTA.DIRSET = PIN2_bm;
    PORTA.DIRSET = PIN3_bm;
    PORTC.DIRSET = PIN0_bm;
    
    
    TCB2.INTCTRL = 0x01;                        // Capture interrupt enabled
    TCB2.EVCTRL = 0x01;                         // Enable input capture event
    TCB2.CTRLB = 0x04;                          // Input capture PW measurement mode
    TCB2.CTRLA = 0x01;
    
    //*********************************************Screen SDA PC2 and SCL PC3 in i2c_avr128db28.c driver*************************************************************************
    
    
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
     
    
    //**************************************************************************ADC for metal dector****************************************************************************************
    
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
    
    PORTD.DIRCLR = PIN2_bm;
    
    //****************************************************************Receiver set up***********************************************************************************
    
    PORTA.PIN5CTRL |= PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL |= PORT_PULLUPEN_bm;
    
    PORTD.PIN4CTRL |= PORT_PULLUPEN_bm;
    PORTD.PIN5CTRL |= PORT_PULLUPEN_bm;
    
    PORTA.DIRCLR = PIN5_bm | PIN6_bm;       // PA5 and PA6 as incoming receiver signal D1 and D0 respectivley
    PORTD.DIRCLR = PIN3_bm | PIN4_bm;       // PD4 and PD5 as incoming receiver signal D3 and D2 respectivley
    
    //Miscellaneous
    
    PORTD.DIRSET = PIN6_bm | PIN7_bm;        // Right headlight PD6 and left is PD7
    
    //*****************************************************************Ultrasonic setup*********************************************************************************
    
    PORTC.DIRCLR = PIN1_bm;
    EVSYS.CHANNEL2 = 0x41;                      // PORTC pin 1 is set to channel 0, will be the incoming echo signal
    EVSYS.USERTCB2CAPT = 0x03;                  // Select channel 2 for this user (3-1=2)
    sei();                                      // Enable global interrupt
    
    //**********************************************************************LOCALS**************************************************************************************
    
    PORTA.DIRSET = PIN7_bm;
    PORTD.DIRSET = PIN5_bm;
    
    uint16_t adc_val = 0;
    uint8_t number[5] = {9, 8, 7, 6, 5};
    uint8_t string[4];
    while (1) {
        signal_check();
        
        if (ADC0.INTFLAGS & ADC_RESRDY_bm) {
            ADC0.INTFLAGS = 1;
            adc_val = ADC0.RES;        
        }
        
        if (adc_val < 1205) {
                PORTD.OUTSET = PIN5_bm;
                TCB2.CCMPH = 1;
                TCB2.CCMPL = (adc_val / 38.5) + 1;
        }
        
        else {
            TCB2.CCMPH = 0;
            PORTD.OUTCLR = PIN5_bm;
        }
        
        for (uint8_t i = 0; i < 5; i++) {
            NumberToAscii(distance[i], string);
            lcd_puts(string);
            lcd_puts(" CM");
            _delay_ms(100);
            lcd_clrscr();
        }
    }
}

ISR(TCB2_INT_vect) {
    TCB2.INTFLAGS = 0x01;
    value[count] = TCB2.CCMP;
    distance[count] = (value[count] * 0.125) / 58; // Convert to cm times value by 0.125 to account for sys clk, I think 
    count++;
    if (count >= 5) {
        count = 0;
    }
}