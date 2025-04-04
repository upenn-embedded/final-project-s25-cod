// IMU Driver for ATmega328 using I2C - Adapted for LSM6DS0
// Author: COD
// Platform: AVR ATmega328

#include <avr/io.h>
#include "i2c.h"
#include "uart.h"

#define IMU_ADDR 0x6B // Default I2C address for LSM6DS0
#define IMU_REG_CTRL1_XL 0x10
#define IMU_REG_CTRL2_G  0x11
#define IMU_REG_OUT_TEMP_L 0x15
#define IMU_REG_OUTX_G   0x18
#define IMU_REG_OUTY_G   0x1A
#define IMU_REG_OUTZ_G   0x1C
#define IMU_REG_OUTX_L_XL  0x28

uint8_t imu_addr;

void IMU_init(uint8_t addr) {
    imu_addr = addr;
    I2C_init();

    // Accelerometer: 119 Hz, ±2g, 50 Hz bandwidth
    I2C_writeRegister(imu_addr, 0x60, IMU_REG_CTRL1_XL);
    // Gyroscope: 119 Hz, 245 dps
    I2C_writeRegister(imu_addr, 0x60, IMU_REG_CTRL2_G);
}

void IMU_getAll(int16_t *accel, int16_t *gyro, int16_t *temp) {
    uint8_t data[14];

    // Read Accel (6 bytes), Temp (2 bytes), Gyro (6 bytes)
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTX_L_XL, 14);

    for (int i = 0; i < 3; i++) {
        accel[i] = (int16_t)(data[i*2+1] << 8 | data[i*2]);
        gyro[i]  = (int16_t)(data[i*2+9] << 8 | data[i*2+8]);
    }

    *temp = (int16_t)(data[7] << 8 | data[6]);
}

int16_t IMU_getXAcc() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTX_L_XL, 2);
    return (int16_t)(data[1] << 8 | data[0]);
}

int16_t IMU_getYAcc() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTX_L_XL + 2, 2);
    return (int16_t)(data[1] << 8 | data[0]);
}

int16_t IMU_getZAcc() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTX_L_XL + 4, 2);
    return (int16_t)(data[1] << 8 | data[0]);
}

int16_t IMU_getXGyro() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTX_G, 2);
    printf("%d %d \n", data[1], data[0]);
    return (int16_t)(data[1] << 8 | data[0]);
}

int16_t IMU_getYGyro() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTY_G, 2);
    return (int16_t)(data[1] << 8 | data[0]);
}

int16_t IMU_getZGyro() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUTZ_G, 2);
    return (int16_t)(data[1] << 8 | data[0]);
}

int16_t IMU_getTemp() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_OUT_TEMP_L, 2);
    return (int16_t)(data[1] << 8 | data[0]);
}

int IMU_checkNewData() {
    // For simplicity, return 1; LSM6DS0 supports status register at 0x1E
    return 1;
}
