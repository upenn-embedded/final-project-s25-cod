
#include <stdint.h>

#ifndef I2C_H
#define I2C_H

/* Function Declarations */

void I2C_init();

void I2C_start();

void I2C_repStart();

void I2C_stop();

void I2C_writeBegin(uint8_t addr);

void I2C_readBegin(uint8_t addr);

void I2C_writeRegister(uint8_t addr, uint8_t data, uint8_t reg);

void I2C_readRegister(uint8_t addr, uint8_t* data_addr, uint8_t reg);

void I2C_writeStream(uint8_t* data, int len);

void I2C_readStream(uint8_t* data_addr, int len);

void I2C_writeCompleteStream(uint8_t *dataArrPtr, uint8_t *addrArrPtr, int len, uint8_t addr);

void I2C_readCompleteStream(uint8_t* dataArrPtr, uint8_t addr, uint8_t reg, int len);

void ERROR();

#endif
