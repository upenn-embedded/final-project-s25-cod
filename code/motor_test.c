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

void motor_init() {
    // Set PWM outputs and direction pins as outputs
    DDRD |= (1 << R_MOTOR_PWM) | (1 << L_MOTOR_PWM) | (1 << R_MOTOR_DIR) | (1 << L_MOTOR_DIR) | (1 << SLEEP);

    // Wake up DRV8833
    PORTD |= (1 << SLEEP);

    // Setup Timer0 for Fast PWM
    TCCR0A |= (1 << COM0A1) | (1 << COM0B1); // Non-inverting for OC0A and OC0B
    TCCR0A |= (1 << WGM01) | (1 << WGM00);   // Fast PWM mode
    TCCR0B |= (1 << CS01) | (1 << CS00);     // Prescaler 64
    
    // Enable Timer0 Compare Match A interrupt
    TIMSK0 |= (1 << OCIE0A);
    
    // NOT USED ON ACTUAL PROJECT JUST BENCH DEBUGGING
    // Joystick inputs as inputs with pull-ups
    DDRC &= ~(1 << JOY);
    ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX0); // Set Vref = AVcc, Select ADC3 for PC3 (default)
    ADCSRA = (1 << ADEN)  | // Enable ADC
    (1 << ADATE) | // Auto-trigger (Free running mode)
    (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set prescaler to 128 (16MHz / 128 = 125kHz)
 
     ADCSRB = 0; // Free running mode (default)
     
     ADCSRA |= (1 << ADSC); // Start first conversion
    
    sei(); // Enable global interrupts
}

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

uint16_t getJoy() {
    return ADC;
}

ISR(TIMER0_COMPA_vect) {
    uint16_t joy = getJoy();
    int16_t speed = 0;

    if (joy > 510) {
        // Map [500-900] ? [0-255]
        speed = (uint16_t)((joy - 500) * 255L / (1023 - 500));
    } else if (joy < 490) {
        // Map [180-500] ? [-255-0]
        speed = -((joy - 180) * 255L / (500 - 180));
    } else {
        speed = 0; // Dead zone
    }
    printf("JOY: %d | SPEED: %d\n", joy, speed);
    driveMotor(speed, -speed);
}

int main(void) {
    motor_init();
    uart_init();
    printf("Test");

    while (1) {}
}
