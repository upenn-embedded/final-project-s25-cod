
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

#define Kp  60
#define Kd  0.06
#define Ki  75

// Motor A Pins (Timer0)
//#define L_MOTOR_DIR PD3 // High is forward, low is backward
//#define L_MOTOR_PWM PD6 // OC0A
#define SLEEP PD4       // Sleep control (write high to enable motor driver)
//
//// Motor B Pins (Timer0)
//#define R_MOTOR_DIR PD2      
//#define R_MOTOR_PWM PD5 // 0C0B

#define PWM_1 PD5
#define PWM_2 PD6
/*
 * main function 
 */

int xaccoffset = 0;
int yaccoffset = 0;
int zaccoffset = 0;

int xgyrooffset = 0;
int ygyrooffset = 0;
int zgyrooffset = 0;

volatile int count = 0;

volatile uint32_t millis_counter = 0;
volatile float gyroAngle=0;
volatile unsigned long currTime, prevTime=0, loopTime;

volatile int motorPower, gyroX;
volatile float accAngle, gyroRate, gyroAngle, currentAngle, prevAngle=0, error, prevError=0, errorSum=0;


// used to set the offsets for each of the 6 axes
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

//get angle of inclination based off of y and z axis accelerometer 
//subject to distortion by sudden movements
float getAccAngle() {
    float yacc = (int) (IMU_getYAcc()) * 2 / 32768.0;
    float zacc = (int) (IMU_getZAcc()) * 2 / 32768.0;
    
    float angle = atan2(yacc, zacc) * (180 / M_PI);
    if (angle > 90) {
        angle -= 180;
    } else if (angle < -90) {
        angle += 180;
    }
    return angle;
}

//get angle of inclination based off x axis gyroscope
//subject to drift over time
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

//sets timer 1 to count milliseconds (for testing)
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

//sets timer 1 to count 5ms (for actual operation)
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

//initializes motor
void motor_init() {
    cli();
    // Set PWM outputs and direction pins as outputs
    //DDRD |= (1 << R_MOTOR_PWM) | (1 << L_MOTOR_PWM) | (1 << R_MOTOR_DIR) | (1 << L_MOTOR_DIR) | (1 << SLEEP);
    DDRD |= (1 << PWM_1) | (1 << PWM_2) | (1 << SLEEP);
    // Wake up DRV8833
    PORTD |= (1 << SLEEP);

    // Setup Timer0 for Fast PWM
    TCCR0A |= (1 << COM0A1) | (1 << COM0B1); // Non-inverting for OC0A and OC0B
    TCCR0A |= (1 << WGM01) | (1 << WGM00);   // Fast PWM mode
    TCCR0B |= (1 << CS01) | (1 << CS00);     // Prescaler 64
    
    // Enable Timer0 Compare Match A interrupt
    //TIMSK0 |= (1 << OCIE0A);
    
//    // NOT USED ON ACTUAL PROJECT JUST BENCH DEBUGGING
//    // Joystick inputs as inputs with pull-ups
//    DDRC &= ~(1 << JOY);
//    ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX0); // Set Vref = AVcc, Select ADC3 for PC3 (default)
//    ADCSRA = (1 << ADEN)  | // Enable ADC
//    (1 << ADATE) | // Auto-trigger (Free running mode)
//    (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set prescaler to 128 (16MHz / 128 = 125kHz)
// 
//     ADCSRB = 0; // Free running mode (default)
//     
//     ADCSRA |= (1 << ADSC); // Start first conversion
    
    sei(); // Enable global interrupts
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
    //getGyroAngle(); //have to initialize 
}

int main() {
    
    Initialize();
    printf("Initialize Done\n");
    
    while (1) {
        accAngle = getAccAngle(); 
        //gyroAngle = getGyroAngle();
        
//        if (count > 1000) {
//            printf("Acceleration Angle: %.2f\n", accAngle);
//            printf("Gyro Angle: %.2f\n\n", gyroAngle);
//            count = 0;
//        }
        
        gyroX = IMU_getXGyro();
        //set motor power after constraining it
        //driveMotor(motorPower, -motorPower);
        driveMotor(motorPower);
        //measure distance every 100 milliseconds
    }
    
    return 0;
}

//ISR(TIMER1_COMPA_vect) {
//    TCNT1 = 0;
//    OCR1A = 124; //1kHz
//    millis_counter++;
//    count++;
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
    if (count > 500) {
        printf("Motor Power: %d     ", -motorPower);
        printf("Error: %.2f     ", error);
        printf("Gyro Angle: %.5f    ", gyroAngle);
        printf("Gyro Rate: %.2f     ", gyroRate);
        printf("Gyro X: %d      ", gyroX);
        printf("Acc Angle: %.2f\n", accAngle);
        count = 0;
    }
    prevAngle = currentAngle;
    count++;
}
