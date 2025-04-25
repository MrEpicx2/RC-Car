#include "i2c_avr128db28.h"


void i2c_init(void) {
    
    // Set baud rate for 100kHz I2C communication
    TWI0.MBAUD = ((24000000 / (2 * 100000)) - 5); // Should be around 115
    
    // Enable TWI Master Mode
    TWI0.MCTRLA = 0b00000001; // Enable master mode with timeout
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc; // Force the bus into an idle state

    
    // Set SDA and SCL as outputs
    PORTMUX.TWIROUTEA = 0x0;
    PORTA.DIRSET = PIN2_bm | PIN3_bm;
    //WILL NOT WORK WITH ULTRASONIC ON THE SAME PORT DUE TO SOME UNKNOWN INTERFERNENCE
    //PORTMUX.TWIROUTEA = 0x2;                //*********************PORT TO PC2 AND PC3******************************
    //PORTC.DIRSET = PIN2_bm | PIN3_bm;       // page 164 of data sheet (ALT TWI0 pins PC2, PC3) 
}


void i2c_start(uint8_t address) {
    // Send start condition and address with write bit (0)
    TWI0.MADDR = address & 0xFE;
    while (!(TWI0.MSTATUS & TWI_WIF_bm)); // Wait for write flag (bit 6 of MSTATUS register)
}

void i2c_stop(void) {
    // Send stop condition
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;       // execute acknowledge and issue stop condition
}

uint8_t i2c_byte(uint8_t byte) {
    TWI0.MDATA = byte;                      // Load byte into data register
    while (!(TWI0.MSTATUS & TWI_WIF_bm));   // Wait for write flag
    
    if (TWI0.MSTATUS & TWI_RXACK_bm) {      // Check for NACK (error)
        return 1; // Indicate failure
    }
    return 0; // Indicate success
}


void i2c_write(uint8_t data) {
    TWI0.MDATA = data;
    while (!(TWI0.MSTATUS & TWI_WIF_bm)); // Wait for write flag
}

uint8_t i2c_read_ack(void) {
    TWI0.MCTRLB = TWI_ACKACT_ACK_gc | TWI_MCMD_RECVTRANS_gc;
    while (!(TWI0.MSTATUS & TWI_RIF_bm)); // Wait for read flag
    return TWI0.MDATA;
}

uint8_t i2c_read_nack(void) {
    TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_RECVTRANS_gc;
    while (!(TWI0.MSTATUS & TWI_RIF_bm)); // Wait for read flag
    return TWI0.MDATA;
}

uint8_t i2c_scan_device(uint8_t address) {
    i2c_start(address << 1);
    
    if (TWI0.MSTATUS & TWI_RXACK_bm) { // No device found
        i2c_stop();
        return 0;
    }
    
    i2c_stop();
    return 1;
}
