#ifndef __GYROSCOPE_H
#define __GYROSCOPE_H


#define MPU_ADDRESS 104
#define X_OUT_H 59
#define DATA_LENGTH 6

#define GYRO_X_OUT_H 67

#define TEMP_OUT_H 65
#define TEMPERATURE_LENGTH 2

#define MAGNETOMETER_ENABLE_ADDRESS 55
#define MAGNETOMETER_ADDRESS 12
#define MAGNETOMETER_X_L_OUT 3
#define MAGNETOMETER_CONTROL_REG 10



#endif

#include <stdint.h>

void MPU_WritePowerManagement(uint8_t data);
void MPU_WriteConfiguration(uint8_t data);
void MPU_WriteSampleRateDivider(uint8_t data);
void MPU_WritegyroConfiguration(uint8_t data);
void MPU_WriteGyroConfiguration(uint8_t data);
void MPU_WriteAccelerationConfiguration(uint8_t data);
void MPU_WriteAccelerationConfiguration2(uint8_t data);
void Calibrate(float* offset_accel, float* offset_gyro);
void ReadRawAccelerometer(int16_t *accel);
void ReadRawGyroscope(int16_t *gyroscope_readings);
void ReadAccelerometer(float *data);
void ReadGyroscope(float *gyroscope_readings);
void ReadMagnetometer(int16_t *magnetic_readings);
void EnableMagnetometer();
int16_t ReadTemperature();
