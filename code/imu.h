/* 
 * File:   imu.h
 *  Supported IMU(s): LSM6DSO
 *  Note: only init(), getAll(), and checkNewData() have been implemented
 * 
 * Author: James Steeman
 *
 * Created on February 22, 2025, 2:20 AM
 */

#include <stdint.h>

#ifndef IMU_H
#define	IMU_H

void IMU_init(uint8_t addr);

void IMU_getAll(int16_t *accel, int16_t *gyro, int16_t *temp);

int16_t IMU_getXAcc();

int16_t IMU_getYAcc();

int16_t IMU_getZAcc();

int16_t IMU_getXGyro();

int16_t IMU_getYGyro();

int16_t IMU_getZGyro();

int16_t IMU_getTemp();

int IMU_checkNewData();

#endif	/* LSM6DS0_H */

