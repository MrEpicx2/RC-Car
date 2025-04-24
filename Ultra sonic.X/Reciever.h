/* 
 * File:   Reciever.h
 * Author: UOC
 *
 * Created on April 20, 2025, 4:14 PM
 */

typedef struct {
    volatile uint8_t* port1;
    uint8_t mask1;
    volatile uint8_t* port2;
    uint8_t mask2;
} SignalCombo;

typedef struct {
    SignalCombo combo;
    ActionFunction func;
    void* arg;
} SignalAction;


#define FORWARD          (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x40}  // PA6
#define BACKWARD         (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x20}  // PA5
#define LEFT             (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x60}  // PA5 | PA6
#define RIGHT            (SignalCombo) {.port2 = &PORTD.IN, .mask2 = 0x08}  // PD3
#define FORWARD_LEFT     (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x40, .port2 = &PORTD.IN, .mask2 = 0x08}   // PA6 | PD3
#define FORWARD_RIGHT    (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x20, .port2 = &PORTD.IN, .mask2 = 0x08}   // PA5 | PD3
#define BACKWARD_LEFT    (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x60, .port2 = &PORTD.IN, .mask2 = 0x08}   // PA5 | PA6 | PD3
#define BACKWARD_RIGHT   (SignalCombo) {.port2 = &PORTD.IN, .mask2 = 0x10,} // PD4
#define RIGHT_LIGHT      (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x40, .port2 = &PORTD.IN, .mask2 = 0x10}   // PA6 | PD4
#define LEFT_LIGHT       (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x20, .port2 = &PORTD.IN, .mask2 = 0x10}   // PA5 | PD4
#define TCB2_SWITCH      (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x60, .port2 = &PORTD.IN, .mask2 = 0x10}   // PA5 | PA6 | PD4