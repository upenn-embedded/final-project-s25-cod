// IMU Driver for ATmega328 using I2C - Adapted for LSM6DS0
// Author: COD
// Platform: AVR ATmega328

#include <avr/io.h>
#include "i2c.h"
#include "uart.h"

#define IMU_ADDR 0x6B // Default I2C address for LSM6DS0
#define IMU_REG_FIFO_CTRL4 0x0A
#define IMU_REG_CTRL1_XL 0x10
#define IMU_REG_CTRL2_G  0x11
#define IMU_REG_CTRL3_C 0x12
#define IMU_REG_CTRL4_C 0x13
#define IMU_REG_CTRL5_C 0x14
#define IMU_REG_CTRL6_C 0x15
#define IMU_REG_CTRL7_G 0x16
#define IMU_REG_CTRL8_XL 0x17
#define IMU_REG_CTRL9_XL 0x18

#define IMU_REG_STATUS_REG 0x1E //temp g xl [2:0]
#define IMU_REG_OUTX_G   0x22
#define IMU_REG_OUTY_G   0x24
#define IMU_REG_OUTZ_G   0x26

#define IMU_REG_OUTX_A 0x28
#define IMU_REG_OUTY_A 0x2A
#define IMU_REG_OUTZ_A 0x2C

#define X_ACC_OFFSET -557
#define Y_ACC_OFFSET -20
#define Z_ACC_OFFSET 402
#define X_GYRO_OFFSET 8
#define Y_GYRO_OFFSET -75
#define Z_GYRO_OFFSET -98



volatile uint8_t imu_addr;

void IMU_init(uint8_t addr) {
    imu_addr = addr;
    
    I2C_writeRegister(imu_addr, 0x00, IMU_REG_FIFO_CTRL4);
    printf("1st write works\n");

    // Accelerometer: 119 Hz, �2g, 50 Hz bandwidth
    I2C_writeRegister(imu_addr, 0x40, IMU_REG_CTRL1_XL); //0100 00 0 0 104Hz 2g low-res
    // Gyroscope: 119 Hz, 245 dps
    I2C_writeRegister(imu_addr, 0x40, IMU_REG_CTRL2_G); //0100 00 0 0
    I2C_writeRegister(imu_addr, 0x04, IMU_REG_CTRL3_C);
    I2C_writeRegister(imu_addr, 0x00, IMU_REG_CTRL4_C);
    I2C_writeRegister(imu_addr, 0x00, IMU_REG_CTRL5_C);
    I2C_writeRegister(imu_addr, 0x00, IMU_REG_CTRL6_C);
    I2C_writeRegister(imu_addr, 0x00, IMU_REG_CTRL7_G);
    
    I2C_writeRegister(imu_addr, 0x00, IMU_REG_CTRL8_XL);
    I2C_writeRegister(imu_addr, 0xE0, IMU_REG_CTRL9_XL); //I3C Disable
    
}

void IMU_getAll(int16_t *accel, int16_t *gyro, int16_t *temp) {
    uint8_t data[14];

    // Read Accel (6 bytes), Temp (2 bytes), Gyro (6 bytes)
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTX_G, 14);

    for (int i = 0; i < 3; i++) {
        accel[i] = (int16_t)(data[i*2+1] << 8 | data[i*2]);
        gyro[i]  = (int16_t)(data[i*2+9] << 8 | data[i*2+8]);
    }

    *temp = (int16_t)(data[7] << 8 | data[6]);
}

int16_t IMU_getXAcc() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTX_A, 2);
    return (int16_t)(data[1] << 8 | data[0]) - X_ACC_OFFSET;
}

int16_t IMU_getYAcc() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTY_A, 2);
    return (int16_t)(data[1] << 8 | data[0]) - Y_ACC_OFFSET;
}

int16_t IMU_getZAcc() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTZ_A, 2);
    return (int16_t)(data[1] << 8 | data[0]) - Z_ACC_OFFSET;
}

int16_t IMU_getXGyro() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTX_G, 2);
    return (int16_t)(data[1] << 8 | data[0]) - X_GYRO_OFFSET;
}

int16_t IMU_getYGyro() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTY_G, 2);
    return (int16_t)(data[1] << 8 | data[0]) - Y_GYRO_OFFSET;
}

int16_t IMU_getZGyro() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTZ_G, 2);
    return (int16_t)(data[1] << 8 | data[0]) - Z_GYRO_OFFSET;
}

int16_t IMU_getTemp() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTX_G, 2);
    return (int16_t)(data[1] << 8 | data[0]);
}

int IMU_checkNewData() {
    // For simplicity, return 1; LSM6DS0 supports status register at 0x1E
    return 1;
}
