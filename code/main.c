
#define ADDRESS  0x6B //replace with imu hex found in Part A

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"
#include "imu.h"
#include "uart.h"
#include <util/twi.h>

#define IMU_REG_STATUS_REG 0x1E //temp g xl [2:0]
#define SCL_CLOCK 400000UL

/*
 * main function 
 */
int main() {
    uart_init();
    cli();
    I2C_init();
    sei();
    
    IMU_init(ADDRESS); //initialize imu using the found I2C address from the 
                       // previous task
    while (1) {
        float ygyro = (int) (IMU_getYGyro()) * 250 / 32768.0;
        float yacc = (int) (IMU_getYAcc()) * 2 / 32768.0;
        printf("XGyro: %.3f\n", xgyro);
        printf("XAcc: %.3f\n", xacc);
        
        _delay_ms(50);
    }
    
    return 0;
}
