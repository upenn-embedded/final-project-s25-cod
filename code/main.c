
#define ADDRESS  0x6B //replace with imu hex found in Part A

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"
#include "imu.h"
#include "i2c.c"
#include "imu.c"


/*
 * main function 
 */
int main() {
    I2C_init(1);  //Initialize I2C, input 0 used to ignore the ERROR() function
    cli();
    IMU_init(ADDRESS); //initialize imu using the found I2C address from the 
                       // previous task
    sei();
    
    _delay_ms(500);
    
    IMU_getAll();
    //Initialization
    
    while(1);
    
    return 0;
}
