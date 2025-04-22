#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

// Motor A Pins (Timer0)
//#define L_MOTOR_DIR PD3 // High is forward, low is backward
//#define L_MOTOR_PWM PD6 // OC0A
//#define SLEEP PD4       // Sleep control (write high to enable motor driver)
//
//// Motor B Pins (Timer0)
//#define R_MOTOR_DIR PD2      
//#define R_MOTOR_PWM PD5 // 0C0B
//
//// Bench joystock debugging

#define MAX_SPEED 255

void driveMotor(int16_t speed) {
    if (speed > MAX_SPEED) speed = MAX_SPEED;
    if (speed < -MAX_SPEED) speed = -MAX_SPEED;
    
    if (speed >= 0) {
        // We are operating in slow decay
        
        // Disable PWM on PD5 (OC0B)
        TCCR0A &= ~(1 << COM0B1);
        PORTD &= ~(1 << PD5);
        
        TCCR0A |= (1 << COM0A1);
        OCR0A = speed;
        
        // AIN1 = PWM, AIN2 = 1, Reverse PWM, slow decay
        // BIN1 = 1, BIN2 = PWM, Forward PWM, slow decay
    } else {
        TCCR0A &= ~(1 << COM0A1);
        PORTD &= ~(1 << PD6);
        
        TCCR0A |= (1 << COM0B1);
        OCR0B = -speed;
        
        // AIN1 = 1, AIN2 = PWM, Forward PWM, slow decay
        // BIN1 = PWM, BIN2 = 1, Reverse PWM, slow decay
    }
}

//void driveMotor(int16_t lSpeed, int16_t rSpeed) {
//    // Clamp speed values to +/-255
//    if (lSpeed > 255) lSpeed = 255;
//    if (lSpeed < -255) lSpeed = -255;
//    if (rSpeed > 255) rSpeed = 255;
//    if (rSpeed < -255) rSpeed = -255;
//    
//    if (lSpeed >= 0) {
//        PORTD &= ~(1 << L_MOTOR_DIR);
//        OCR0A = lSpeed;
//    } else {
//        PORTD |= (1 << L_MOTOR_DIR);
//        OCR0A = -lSpeed;
//    }
//    
//    if (rSpeed >= 0) {
//        PORTD &= ~(1 << R_MOTOR_DIR);
//        OCR0B = rSpeed;
//    } else {
//        PORTD |= (1 << R_MOTOR_DIR);
//        OCR0B = -rSpeed;
//    }
//}
