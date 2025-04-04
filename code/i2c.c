#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"

#define SCL_CLOCK 400000UL

void I2C_init() {
    TWSR0 = 0x00; // Prescaler = 1
    TWBR0 = ((F_CPU/SCL_CLOCK)-16)/2; // Bit rate
    TWCR0 = (1 << TWEN); // Enable TWI
}

void I2C_start() {
    TWCR0 = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    printf("start!\n");
    while (!(TWCR0 & (1 << TWINT)));
    printf("finish?\n");
}

void I2C_repStart() {
    I2C_start(); // Same as I2C_start()
}

void I2C_stop() {
    TWCR0 = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    while (TWCR0 & (1 << TWSTO));
}

void I2C_writeBegin(uint8_t addr) {
    I2C_start();
    TWDR0 = (addr << 1) | 0; // Write mode
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));
}

void I2C_readBegin(uint8_t addr) {
    I2C_start();
    TWDR0 = (addr << 1) | 1; // Read mode
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));
}

void I2C_writeRegister(uint8_t addr, uint8_t data, uint8_t reg) {
    I2C_writeBegin(addr);
    TWDR0 = reg;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));

    TWDR0 = data;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));

    I2C_stop();
}

void I2C_readRegister(uint8_t addr, uint8_t* data_addr, uint8_t reg) {
    I2C_writeBegin(addr);
    TWDR0 = reg;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));

    I2C_readBegin(addr);
    TWCR0 = (1 << TWINT) | (1 << TWEN); // NACK
    while (!(TWCR0 & (1 << TWINT)));
    *data_addr = TWDR0;

    I2C_stop();
}

void I2C_writeStream(uint8_t* data, int len) {
    for (int i = 0; i < len; i++) {
        TWDR0 = data[i];
        TWCR0 = (1 << TWINT) | (1 << TWEN);
        while (!(TWCR0 & (1 << TWINT)));
    }
}

void I2C_readStream(uint8_t* data_addr, int len) {
    for (int i = 0; i < len; i++) {
        if (i < len - 1) {
            TWCR0 = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); // ACK
        } else {
            TWCR0 = (1 << TWINT) | (1 << TWEN); // NACK
        }
        while (!(TWCR0 & (1 << TWINT)));
        data_addr[i] = TWDR0;
    }
}

void I2C_writeCompleteStream(uint8_t *dataArrPtr, uint8_t *addrArrPtr, int len, uint8_t addr) {
    I2C_writeBegin(addr);
    for (int i = 0; i < len; i++) {
        TWDR0 = addrArrPtr[i];
        TWCR0 = (1 << TWINT) | (1 << TWEN);
        while (!(TWCR0 & (1 << TWINT)));

        TWDR0 = dataArrPtr[i];
        TWCR0 = (1 << TWINT) | (1 << TWEN);
        while (!(TWCR0 & (1 << TWINT)));
    }
    I2C_stop();
}

void I2C_readCompleteStream(uint8_t* dataArrPtr, uint8_t addr, uint8_t reg, int len) {
    I2C_writeBegin(addr);
    TWDR0 = reg;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));

    I2C_readBegin(addr);
    I2C_readStream(dataArrPtr, len);
    I2C_stop();
}

void ERROR() {
    while (1) {
        // Loop forever or blink LED to indicate error
    }
}
