/*
 * File:   newavr-main.c
 * Author: Khai
 *
 * Created on April 8, 2025, 7:33 PM
 */


#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include "Controller.h"

typedef struct {
    bool current;
    bool previous;
} ButtonState;

//***********************************************GLOBALS******************************************************** 

ButtonState buttonForward = {0};        // PA1      Forward
ButtonState buttonBackward = {0};       // PA2      Backwards
ButtonState buttonLeft = {0};           // PA5      Left
ButtonState buttonRight = {0};          // PA6      Right
ButtonState buttonLight_L = {0};        // PA3      Left light
ButtonState buttonLight_R = {0};        // PA4      Right light
uint8_t combo = 0b0000; 

bool is_button_held(ButtonState *btn) {
    return btn->current && btn->previous;
}

void update_button_state(ButtonState *btn, volatile uint8_t *port_in, uint8_t pin_mask) {
    btn->previous = btn->current;
    btn->current = (*port_in & pin_mask);
}

uint8_t combo_logic(void) {
    combo = 0b0000;
    update_button_state(&buttonForward, &PORTA.IN, FORWARD_BUTTON);  
    update_button_state(&buttonBackward, &PORTA.IN, BACK_BUTTON);  
    update_button_state(&buttonLeft, &PORTA.IN, LEFT_BUTTON);  
    update_button_state(&buttonRight, &PORTA.IN, RIGHT_BUTTON);
    update_button_state(&buttonLight_L, &PORTA.IN, LIGHT_L_BUTTON);
    update_button_state(&buttonLight_R, &PORTA.IN, LIGHT_R_BUTTON);
    
    if (is_button_held(&buttonForward) && is_button_held(&buttonLeft)) {
    combo = FORWARD_LEFT;
    }
    else if (is_button_held(&buttonForward) && is_button_held(&buttonRight)) {
        combo = FORWARD_RIGHT;
    }
    else if (is_button_held(&buttonBackward) && is_button_held(&buttonLeft)) {
        combo = BACKWARD_LEFT;
    }
    else if (is_button_held(&buttonBackward) && is_button_held(&buttonRight)) {
        combo = BACKWARD_RIGHT;
    }
    else if (is_button_held(&buttonForward)) {
        combo = FORWARD;
    }
    else if (is_button_held(&buttonBackward)) {
        combo = BACKWARD;
    }
    else if (is_button_held(&buttonLeft)) {
        combo = LEFT;
    }
    else if (is_button_held(&buttonRight)) {
        combo = RIGHT;
    }
    else if (is_button_held(&buttonLight_R)) {
        combo = RIGHT_LIGHT;
    }
    else if (is_button_held(&buttonLight_L)) {
        combo = LEFT_LIGHT;
    }
    else {
        combo = IDLE;
    }
    return combo;
}

int main(void) {
    
    PORTA.DIRCLR = 0xFF;
    PORTD.DIRSET = 0xFF;
    
    while (1) {
        combo = combo_logic();
        PORTD.OUT = 0xFF;           // Set all lines HIGH to reset the encoder
        _delay_ms(450);               // Brief delay for it to register release
        PORTD.OUT = combo;
        _delay_ms(200);
            
    }         
}
