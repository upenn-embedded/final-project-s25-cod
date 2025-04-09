#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <util/delay.h>
#include "uart.h"

#define SCL_CLOCK 100000UL

volatile int startCount = 0;

void I2C_init() {
    TWSR0 &= 0x00;
    TWBR0 = ((F_CPU / SCL_CLOCK) - 16) / 2;
}

void ERROR() {
    printf("ERROR!");
    while(1) {
        
    }
}

uint8_t I2C_start(uint8_t address) {
    uint8_t status;
    TWCR0 = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); //enable TWI, generate START
    while (!(TWCR0 & (1 << TWINT))); //Wait for TWI to finish
    status = TWSR0 & 0xF8; //read TWI status register
    if (status != 0x08) {
        return 0; //start condition failed
    }
    TWDR0 = (address << 1) | 0;
    TWCR0 = (1 << TWEN) | (1 << TWINT); //enable TWI, clear interrupt
    while (!(TWCR0 & (1 << TWINT))); //Wait for TWI to finish
    status = TWSR0 & 0xF8;
    if (status == 0x18) {
        return 1; //ACK received
    } else if (status == 0x20) {
        return 2; //NACK received
    } else {
        return 3; //SLA+W failed
    }
}

uint8_t I2C_repStart(uint8_t address) {
    uint8_t status;		/* Declare variable */
    TWCR0=(1<<TWSTA)|(1<<TWEN)|(1<<TWINT);/* Enable TWI, generate start */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR0&0xF8;		/* Read TWI status register */
    if(status!=0x10) {
        return 0;			/* Return 0 for repeated start condition fail */
    }		/* Check for repeated start transmitted */
    TWDR0 = (address << 1) | 1;		/* Write SLA+R in TWI data register */
    TWCR0=(1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR0&0xF8;		/* Read TWI status register */
    if(status==0x40) {
        return 1;			/* Return 1 to indicate ack received */
    }		/* Check for SLA+R transmitted &ack received */
    if(status==0x48) {		/* Check for SLA+R transmitted &nack received */
        return 2;
    }			/* Return 2 to indicate nack received */
    else {
       return 3;			/* Else return 3 to indicate SLA+W failed */ 
    }
}

void I2C_stop() {
    TWCR0 = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    while (TWCR0 & (1 << TWSTO));
}

uint8_t I2C_write(uint8_t data)	/* I2C write function */
{
    uint8_t status;		/* Declare variable */
    TWDR0=data;			/* Copy data in TWI data register */
    TWCR0=(1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR0&0xF8;		/* Read TWI status register */
    if(status==0x28)		/* Check for data transmitted &ack received */
    return 0;			/* Return 0 to indicate ack received */
    if(status==0x30)		/* Check for data transmitted &nack received */
    return 1;			/* Return 1 to indicate nack received */
    else
    return 2;			/* Else return 2 for data transmission failure */
}

uint8_t I2C_read_Ack()		/* I2C read ack function */
{
    TWCR0=(1<<TWEN)|(1<<TWINT)|(1<<TWEA); /* Enable TWI, generation of ack */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    return TWDR0;			/* Return received data */
}

uint8_t I2C_read_Nack()		/* I2C read nack function */
{
    TWCR0=(1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    return TWDR0;		/* Return received data */
}

void I2C_writeRegister(uint8_t addr, uint8_t data, uint8_t reg) {
    I2C_start(addr);/* Start I2C with device write address */
    I2C_write(reg);		/* Write start memory address for data write */
    I2C_write(data);
    I2C_stop();			/* Stop I2C */
}

void I2C_readRegister(uint8_t addr, uint8_t* data_addr, uint8_t reg) {
    I2C_start(addr);/* Start I2C with device write address */
    I2C_write(reg);		/* Write start memory address */
    I2C_repStart(addr);/* Repeat start I2C SLA+R */
    data_addr[0] = I2C_read_Ack();
    I2C_read_Nack();		/* Read flush data with nack */
    I2C_stop();			/* Stop I2C */
}

void I2C_readCompleteStream(uint8_t* dataArrPtr, uint8_t addr, uint8_t reg, int len) {
    I2C_start(addr);/* Start I2C with device write address */
    I2C_write(reg);		/* Write start memory address */
    I2C_repStart(addr);/* Repeat start I2C SLA+R */
    for (int i = 0; i<len; i++)/* Read data with acknowledgment */
	{
        if (i < len - 1) {
            dataArrPtr[i] = I2C_read_Ack();
        } else {
            dataArrPtr[i] = I2C_read_Nack();
        }
	}
    I2C_stop();			/* Stop I2C */
}
