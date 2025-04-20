/* 
 * File:   Controller.h
 * Author: UOC
 *
 * Created on April 20, 2025, 12:01 PM
 */

/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/

#define RIGHT_BUTTON       (uint8_t) 0x40    // PA6
#define LEFT_BUTTON        (uint8_t) 0x20    // PA5
#define BACK_BUTTON        (uint8_t) 0x04    // PA2
#define FORWARD_BUTTON     (uint8_t) 0x02    // PA1
#define LIGHT_R_BUTTON     (uint8_t) 0x10    // PA4
#define LIGHT_L_BUTTON     (uint8_t) 0x08    // PA3


#define IDLE            (uint8_t) ~0x00
#define FORWARD         (uint8_t) ~0x02                 // PD1 low
#define BACKWARD        (uint8_t) ~0x04                 // PD2 low
#define LEFT            (uint8_t) ~(0x02 | 0x04)        // PD1 and PD2 low
#define RIGHT           (uint8_t) ~0x08                 // PD3 low
#define FORWARD_LEFT    (uint8_t) ~(0x02 | 0x08)        // PD1 and PD3 low
#define FORWARD_RIGHT   (uint8_t) ~(0x04 | 0x08)        // PD2 and PD3 low
#define BACKWARD_LEFT   (uint8_t) ~(0x02 | 0x04 | 0x08) // PD1, PD2, and PD3 low
#define BACKWARD_RIGHT  (uint8_t) ~(0x10)               // PD4 low
#define RIGHT_LIGHT     (uint8_t) ~(0x02 | 0x10)        // PD1 and PD4 low
#define LEFT_LIGHT      (uint8_t) ~(0x04 | 0x10)        // PD2 and PD4 low