#include"gyroscope.h"
#include "i2c0.h"
#include "system_utils.h"
#include <math.h>
// Code understood from : https://github.com/amartinezacosta/Tiva-C-MPU9250/blob/master/mpu9250.c


void Calibrate(float* offset_accel, float* offset_gyro)
{
    int num_tries = 1024;

    float calculated_accel[3];
    float calculated_gyro[3];

    int16_t accel_bias[3];
    int16_t deg_bias[3];

    int64_t accel_sum[3] = {0,0,0};         // Mav value of int16_t can be 32,768, so for 512 tries we need a max value of at least 2^32
    int64_t gyro_sum[3] = {0,0,0};

    int i = 0;

    for(i=0;i<num_tries;i++)
    {
        ReadRawAccelerometer(calculated_accel);
        ReadRawGyroscope(calculated_gyro);

        accel_sum[0] += calculated_accel[0];
        accel_sum[1] += calculated_accel[1];
        accel_sum[2] += calculated_accel[2];

        gyro_sum[0] += calculated_gyro[0];
        gyro_sum[1] += calculated_gyro[1];
        gyro_sum[2] += calculated_gyro[2];
    }

    accel_bias[0] = accel_sum[0] / num_tries;
    accel_bias[1] = accel_sum[1] / num_tries;
    accel_bias[2] = accel_sum[2] / num_tries;

    deg_bias[0] = gyro_sum[0] / num_tries;
    deg_bias[1] = gyro_sum[1] / num_tries;
    deg_bias[2] = gyro_sum[2] / num_tries;

    offset_accel[0] = (float)(2 * accel_bias[0])/32768;
    offset_accel[1] = (float)(2 * accel_bias[1])/32768;
    offset_accel[2] = (float)(2 * accel_bias[2])/32768;

    offset_gyro[0] = (float)(2 * deg_bias[0])/32768;
    offset_gyro[1] = (float)(2 * deg_bias[1])/32768;
    offset_gyro[2] = (float)(2 * deg_bias[2])/32768;

}

void ReadRawAccelerometer(int16_t *accel)
{
    uint8_t data[6];
    readI2c0Registers(MPU_ADDRESS,X_OUT_H,DATA_LENGTH,data);
    accel[0] = (data[0] << 8) | data[1];
    accel[1] = (data[2] << 8) | data[3];
    accel[2] = (data[4] << 8) | data[5];
}

void ReadAccelerometer(float *accel)
{
    uint8_t data[6];
    int16_t accel_temp[3];
    readI2c0Registers(MPU_ADDRESS,X_OUT_H,DATA_LENGTH,data);
    accel_temp[0] = (data[0] << 8) | data[1];
    accel_temp[1] = (data[2] << 8) | data[3];
    accel_temp[2] = (data[4] << 8) | data[5];

    accel[0] = (float)(2*accel_temp[0])/32768;
    accel[1] = (float)(2*accel_temp[1])/32768;
    accel[2] = (float)(2*accel_temp[2])/32768;
}

void ReadRawGyroscope(int16_t *gyroscope_readings)
{
   uint8_t data[6];
   readI2c0Registers(MPU_ADDRESS,GYRO_X_OUT_H,DATA_LENGTH,data);
   gyroscope_readings[0] = (data[0] << 8) | data[1];
   gyroscope_readings[1] = (data[2] << 8) | data[3];
   gyroscope_readings[2] = (data[4] << 8) | data[5];

}

void ReadGyroscope(float *gyroscope_readings)
{
   uint8_t data[6];
   int16_t gyro_temp[3];
   readI2c0Registers(MPU_ADDRESS,GYRO_X_OUT_H,DATA_LENGTH,data);
   gyro_temp[0] = (data[0] << 8) | data[1];
   gyro_temp[1] = (data[2] << 8) | data[3];
   gyro_temp[2] = (data[4] << 8) | data[5];

   gyroscope_readings[0] = (float)(250 * gyro_temp[0])/32768;
   gyroscope_readings[1] = (float)(250 * gyro_temp[1])/32768;
   gyroscope_readings[2] = (float)(250 * gyro_temp[2])/32768;

}

int16_t ReadTemperature()
{
    uint8_t data[2];
    int16_t temp_readings;
    readI2c0Registers(MPU_ADDRESS,TEMP_OUT_H,TEMPERATURE_LENGTH,data);
    temp_readings = (int16_t)(data[0] << 8) | data[1];
    temp_readings =  temp_readings/333.87 + 21;

    return temp_readings;
}

void EnableMagnetometer()
{
    writeI2c0Register(MPU_ADDRESS, MAGNETOMETER_ENABLE_ADDRESS, asciiToUint8("0x02"));              // enable i2c mag bypass
    writeI2c0Register(MAGNETOMETER_ADDRESS, MAGNETOMETER_CONTROL_REG, asciiToUint8("0x16")); // enable 16-bit continuous output
}

void ReadMagnetometer(int16_t *magnetic_readings)
{
    uint8_t data[6];
    while(! (readI2c0Register(MAGNETOMETER_ADDRESS, MAGNETOMETER_STATUS_ONE_REG) & 0x01) ); //wait until data is ready
    readI2c0Registers(MAGNETOMETER_ADDRESS, MAGNETOMETER_X_L_OUT, DATA_LENGTH, data);
    magnetic_readings[0] = (int16_t)(data[1] << 8) | data[0];
    magnetic_readings[1] = (int16_t)(data[3] << 8) | data[2];
    magnetic_readings[2] = (int16_t)(data[5] << 8) | data[4];

    magnetic_readings[0] = (float)(magnetic_readings[0]*4912*10)/32760;
    magnetic_readings[1] = (float)(magnetic_readings[1]*4912*10)/32760;
    magnetic_readings[2] = (float)(magnetic_readings[2]*4912*10)/32760;

    readI2c0Register(MAGNETOMETER_ADDRESS,MAGNETOMETER_STATUS_TWO_REG);

}

void CalculateAngles(float *calibrated_acceleration, float *calibrated_gyroscope_readings, float *pitch, float *roll, float *yaw)
{

    *(pitch) += calibrated_gyroscope_readings[0] * 0.01;
    *(roll) += calibrated_gyroscope_readings[1] * 0.01;
    *(yaw) += calibrated_gyroscope_readings[2] * 0.01;

    float accel_pitch = atan2f(calibrated_acceleration[1],sqrtf(calibrated_acceleration[0]*calibrated_acceleration[0] + calibrated_acceleration[2]*calibrated_acceleration[2]) )*180 / 3.141592;
    float accel_roll =  atan2f(calibrated_acceleration[0],sqrtf(calibrated_acceleration[1]*calibrated_acceleration[1] + calibrated_acceleration[2]*calibrated_acceleration[2]) )*180 / 3.141592;
    float accel_yaw = atan2f(calibrated_acceleration[2],sqrtf(calibrated_acceleration[0]*calibrated_acceleration[0] + calibrated_acceleration[1]*calibrated_acceleration[1]) )*180 / 3.141592;

    *(pitch) = 0.94*(*pitch) + 0.06*(accel_pitch);
    *(roll) = 0.94*(*roll) + 0.06*accel_roll;
    *(yaw) = 0.98*(*yaw) + 0.02*accel_yaw;

}
