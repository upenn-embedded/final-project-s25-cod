
#define ADDRESS  0x6B //replace with imu hex found in Part A

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"
#include "imu.h"
#include "uart.h"
#include <util/twi.h>
#include <math.h>

#define IMU_REG_STATUS_REG 0x1E //temp g xl [2:0]
#define SCL_CLOCK 400000UL
/*
 * main function 
 */
volatile long xaccoffset = 0;
volatile long yaccoffset = 0;
volatile long zaccoffset = 0;

volatile long xgyrooffset = 0;
volatile long ygyrooffset = 0;
volatile long zgyrooffset = 0;

volatile uint32_t millis_counter = 0;
volatile float gyroAngle=0;
volatile unsigned long currTime, prevTime=0, loopTime;

void calibrate() {
    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < 200; i++) {
            xaccoffset += IMU_getXAcc();
            yaccoffset += IMU_getYAcc();
            zaccoffset += IMU_getZAcc();

            xgyrooffset += IMU_getXGyro();
            ygyrooffset += IMU_getYGyro();
            zgyrooffset += IMU_getZGyro();
        }
    
        xaccoffset = xaccoffset / 200;
        yaccoffset = yaccoffset / 200;
        zaccoffset = zaccoffset / 200;

        xgyrooffset = xgyrooffset / 200;
        ygyrooffset = ygyrooffset / 200;
        zgyrooffset = zgyrooffset / 200;

        printf("XAcc Offset: %l\n", xaccoffset);
        printf("YAcc Offset: %l\n", yaccoffset);
        printf("ZAcc Offset: %l\n", zaccoffset);

        printf("XGyro Offset: %l\n", xgyrooffset);
        printf("YGyro Offset: %l\n", ygyrooffset);
        printf("ZGyro Offset: %l\n", zgyrooffset);
        
        xaccoffset = 0;
        yaccoffset = 0;
        zaccoffset = 0;
        
        xgyrooffset = 0;
        ygyrooffset = 0;
        zgyrooffset = 0;    
    }
}

double getAccAngle() {
    float yacc = (int) (IMU_getYAcc()) * 2 / 32768.0;
    float zacc = (int) (IMU_getZAcc()) * 2 / 32768.0;

    return atan2(yacc, zacc) * (180 / M_PI);
}

void init_millis_timer() {
    TCCR1A = 0;                 // Normal mode
    TCCR1B = (1 << CS11) | (1 << CS10); // Prescaler 64
    TIMSK1 = (1 << TOIE1);      // Enable overflow interrupt
    TCNT1 = 0;
    sei();                      // Enable global interrupts
}

uint32_t millis() {
    uint32_t ms;
    cli();             // Disable interrupts for atomic read
    ms = millis_counter;
    sei();             // Re-enable interrupts
    return ms;
}

int main() {
    uart_init();
    cli();
    I2C_init();
    sei();
    
    init_millis_timer();
    
    IMU_init(ADDRESS); //initialize imu using the found I2C address from the 
                       // previous task
    
    while (1) {
        currTime = millis_counter;
        loopTime = currTime - prevTime;
        prevTime = currTime;

        float gyroX = IMU_getXGyro() * 250 / 32728.0;
        gyroAngle = gyroAngle + (float)gyroX*loopTime/1024;
        printf("%d\n", currTime);
        printf("Gyro Angle: %.2f\n", gyroAngle);
    }
    
    return 0;
}

ISR(TIMER1_OVF_vect) {
    millis_counter++;
}
