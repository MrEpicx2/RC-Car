/* 
 * File:   Reciever.h
 * Author: UOC
 *
 * Created on April 20, 2025, 4:14 PM
 */

#define NULL ((void*)0)

typedef void (*ActionFunction)(void);

typedef struct {
    volatile uint8_t* port1;
    uint8_t mask1;
    volatile uint8_t* port2;
    uint8_t mask2;
} SignalCombo;


#define FORWARD          (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x40}  // PA6
#define BACKWARD         (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x20}  // PA5
#define LEFT             (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x60}  // PA5 | PA6
#define RIGHT            (SignalCombo) {.port2 = &PORTD.IN, .mask2 = 0x10}  // PD4
#define FORWARD_LEFT     (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x40, .port2 = &PORTD.IN, .mask2 = 0x10}   // PA6 | PD4
#define FORWARD_RIGHT    (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x20, .port2 = &PORTD.IN, .mask2 = 0x10}   // PA5 | PD4
#define BACKWARD_LEFT    (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x60, .port2 = &PORTD.IN, .mask2 = 0x10}   // PA5 | PA6 | PD4
#define BACKWARD_RIGHT   (SignalCombo) {.port2 = &PORTD.IN, .mask2 = 0x20,} // PD5
#define RIGHT_LIGHT      (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x40, .port2 = &PORTD.IN, .mask2 = 0x20}   // PA6 | PD5
#define LEFT_LIGHT       (SignalCombo) {.port1 = &PORTA.IN, .mask1 = 0x20, .port2 = &PORTD.IN, .mask2 = 0x20}   // PA5 | PD5