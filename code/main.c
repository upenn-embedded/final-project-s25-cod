
#define ADDRESS  0x6B //replace with imu hex found in Part A

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"
#include "imu.h"
#include "uart.h"

/*
 * main function 
 */
int main() {
    uart_init();
    cli();
    IMU_init(ADDRESS); //initialize imu using the found I2C address from the 
                       // previous task
    sei();
    
    _delay_ms(500);
    
    while(1) {
        printf("XGyro: %d\n", (int) IMU_getXGyro());
        printf("YGyro: %d\n", (int) IMU_getYGyro());
        printf("ZGyro: %d\n", (int) IMU_getZGyro());
        _delay_ms(1000);
    }
    
    return 0;
}
