
#include <stdint.h>

#ifndef I2C_H
#define I2C_H

/* Function Declarations */

void I2C_init();

uint8_t I2C_start();

uint8_t I2C_repStart();

void I2C_stop();

uint8_t I2C_write(uint8_t data);

void I2C_writeRegister(uint8_t addr, uint8_t data, uint8_t reg);

void I2C_readRegister(uint8_t addr, uint8_t* data_addr, uint8_t reg);

void I2C_readCompleteStream(uint8_t* dataArrPtr, uint8_t addr, uint8_t reg, int len);

void ERROR();

#endif
