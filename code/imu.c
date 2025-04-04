#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define IMU_ADDR 0x68 // Default I2C address for MPU6050
#define IMU_REG_ACCEL_XOUT_H 0x3B
#define IMU_REG_GYRO_XOUT_H 0x43
#define IMU_REG_TEMP_OUT_H 0x41
#define IMU_REG_PWR_MGMT_1 0x6B

uint8_t imu_addr;

void IMU_init(uint8_t addr) {
    imu_addr = addr;
    I2C_init();
    I2C_writeRegister(imu_addr, 0x00, IMU_REG_PWR_MGMT_1); // Wake up IMU
}

void IMU_getAll(int16_t *accel, int16_t *gyro, int16_t *temp) {
    uint8_t data[14];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_ACCEL_XOUT_H, 14);
    
    for (int i = 0; i < 3; i++) {
        accel[i] = (data[i * 2] << 8) | data[i * 2 + 1];
        gyro[i] = (data[8 + i * 2] << 8) | data[9 + i * 2];
    }
    *temp = (data[6] << 8) | data[7];
}

int16_t IMU_getXAcc() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_ACCEL_XOUT_H, 2);
    return (data[0] << 8) | data[1];
}

int16_t IMU_getYAcc() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_ACCEL_XOUT_H + 2, 2);
    return (data[0] << 8) | data[1];
}

int16_t IMU_getZAcc() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_ACCEL_XOUT_H + 4, 2);
    return (data[0] << 8) | data[1];
}

int16_t IMU_getXGyro() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_GYRO_XOUT_H, 2);
    return (data[0] << 8) | data[1];
}

int16_t IMU_getYGyro() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_GYRO_XOUT_H + 2, 2);
    return (data[0] << 8) | data[1];
}

int16_t IMU_getZGyro() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_GYRO_XOUT_H + 4, 2);
    return (data[0] << 8) | data[1];
}

int16_t IMU_getTemp() {
    uint8_t data[2];
    I2C_readCompleteStream(data, imu_addr, IMU_REG_TEMP_OUT_H, 2);
    return (data[0] << 8) | data[1];
}

int IMU_checkNewData() {
    // Assuming data ready bit is in status register (not implemented for MPU6050)
    return 1; // Always returning new data available for now
}
