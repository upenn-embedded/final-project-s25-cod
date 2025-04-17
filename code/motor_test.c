#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

// Motor A Pins (Timer0)
#define L_MOTOR_DIR PD3 // High is forward, low is backward
#define L_MOTOR_PWM PD6 // OC0A
#define SLEEP PD4       // Sleep control (write high to enable motor driver)

// Motor B Pins (Timer0)
#define R_MOTOR_DIR PD2      
#define R_MOTOR_PWM PD5 // 0C0B

// Bench joystock debugging
#define JOY PC3

void driveMotor(int16_t lSpeed, int16_t rSpeed) {
    // Clamp speed values to +/-255
    if (lSpeed > 255) lSpeed = 255;
    if (lSpeed < -255) lSpeed = -255;
    if (rSpeed > 255) rSpeed = 255;
    if (rSpeed < -255) rSpeed = -255;
    
    if (lSpeed >= 0) {
        PORTD &= ~(1 << L_MOTOR_DIR);
        OCR0A = lSpeed;
    } else {
        PORTD |= (1 << L_MOTOR_DIR);
        OCR0A = lSpeed + 255;
    }
    
    if (rSpeed >= 0) {
        PORTD &= ~(1 << R_MOTOR_DIR);
        OCR0B = rSpeed;
    } else {
        PORTD |= (1 << R_MOTOR_DIR);
        OCR0B = rSpeed + 255;
    }
}
