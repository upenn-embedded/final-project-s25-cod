
#define ADDRESS  0x6A //replace with imu hex found in Part A

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
    PORTC |= (1<<PC4);
    PORTC |= (1<<PC5);
    uart_init();
    cli();
    I2C_init();
    sei();
    IMU_init(ADDRESS); //initialize imu using the found I2C address from the 
                       // previous task
    printf("Init done\n");
    _delay_ms(500);
//    
    while(1) {
        printf("XGyro: %d\n", IMU_getXGyro());
        printf("x done\n");
        printf("YGyro: %d\n", IMU_getYGyro());
        printf("y done\n");
        printf("ZGyro: %d\n", IMU_getZGyro());
//        uint8_t data[2];
//        for (int i = 0; i < 8; i++) {
//            I2C_readCompleteStream(data, ADDRESS, 0x1E + 2 * i, 2);
//            printf("REG %X: %X\n", 0x20 + 2 * i, (int16_t)(int16_t)(data[1] << 8 | data[0]));
//        }
//        _delay_ms(1000);
    }
    
    return 0;
}
