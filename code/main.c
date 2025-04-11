
#define ADDRESS  0x6B //replace with imu hex found in Part A

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"
#include "imu.h"
#include "uart.h"
#include "motor_test.h"
#include <util/twi.h>
#include <math.h>

#define IMU_REG_STATUS_REG 0x1E //temp g xl [2:0]
#define SCL_CLOCK 400000UL

#define SAMPLE_TIME 0.005
#define TARGET_ANGLE 0.0

#define Kp  40
#define Kd  0.05
#define Ki  40
/*
 * main function 
 */

int xaccoffset = 0;
int yaccoffset = 0;
int zaccoffset = 0;

int xgyrooffset = 0;
int ygyrooffset = 0;
int zgyrooffset = 0;

volatile uint32_t millis_counter = 0;
volatile float gyroAngle=0;
volatile unsigned long currTime, prevTime=0, loopTime;

volatile int motorPower, gyroRate, gyroX;
volatile float accAngle, gyroAngle, currentAngle, prevAngle=0, error, prevError=0, errorSum=0;

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

float getAccAngle() {
    float yacc = (int) (IMU_getYAcc()) * 2 / 32768.0;
    float zacc = (int) (IMU_getZAcc()) * 2 / 32768.0;

    return atan2(yacc, zacc) * (180 / M_PI);
}

double getGyroAngle() { //must call once at the beginning to initialize
    currTime = millis_counter;
    if ((int) currTime < (int) prevTime) {
        prevTime = 0;
        currTime = loopTime;
        millis_counter = 0;
    }
    loopTime = currTime - prevTime;
    prevTime = currTime;
    float gyroX = IMU_getXGyro() * 250 / 32728.0;
    return gyroAngle + (float)gyroX*loopTime/1000;
    
}

void init_millis_timer() {
    cli();
    TCCR1A |= (1<<COM1A1);
    TCCR1A &= ~(1<<COM1A0);
    // CTC Mode
    TCCR1B |= (1<<WGM12);
    TCCR1B |= (1 << CS11);
    TCCR1B |= (1 << CS10); // Prescaler 64
    TIMSK1 |= (1<<OCIE1A);
    OCR1A = 124;
    TCNT1 = 0;
    sei();                      // Enable global interrupts
}

void init_200hz_timer() {
    cli();
    TCCR1A |= (1<<COM1A1);
    TCCR1A &= ~(1<<COM1A0);
    // CTC Mode
    TCCR1B |= (1<<WGM22);
    TCCR1B |= (1 << CS12); // Prescaler 128
    TIMSK1 |= (1<<OCIE1A);
    OCR1A = 155;
    TCNT1 = 0;
    sei();   
}

uint32_t millis() {
    uint32_t ms;
    cli();             // Disable interrupts for atomic read
    ms = millis_counter;
    sei();             // Re-enable interrupts
    return ms;
}

void Initialize() {
    uart_init();
    cli();
    I2C_init();
    sei();
    
    //init_millis_timer();
    init_200hz_timer();
    motor_init();
    
    IMU_init(ADDRESS); //initialize imu using the found I2C address from the 
                       // previous task
    getGyroAngle();
}

int main() {
    printf("before initialize");
    Initialize();
    printf("Test 1");
    while (1) {
        printf("Serial test");
        calibrate();
        accAngle = getAccAngle(); 
        gyroX = IMU_getXGyro();
        // set motor power after constraining it
        driveMotor(motorPower, motorPower);
        // measure distance every 100 milliseconds
    }
    
    return 0;
}

//ISR(TIMER1_COMPA_vect) {
//    TCNT1 = 0;
//    OCR1A = 124; //1kHz
//    millis_counter++;
//}

ISR(TIMER1_COMPA_vect) {
    TCNT1 = 0;
    OCR1A = 155; //200 Hz
    gyroRate = gyroX * 250 / 32728.0;
    gyroAngle = (float) gyroRate * SAMPLE_TIME;
    currentAngle = 0.9934*(prevAngle + gyroAngle) + 0.0066*(accAngle);
  
    error = currentAngle - TARGET_ANGLE;
    errorSum = errorSum + error;  
    if (errorSum < -300) {
        errorSum = -300;
    } else if (errorSum > 300) {
        errorSum = 300;
    }
    //calculate output from P, I and D values
    motorPower = Kp*(error) + Ki*(errorSum)*SAMPLE_TIME - Kd*(currentAngle-prevAngle)/SAMPLE_TIME;
    prevAngle = currentAngle;
}
