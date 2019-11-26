//Aditya Rajguru

/*
 *
 * Hardware Target: TM-4C123GH6PM
 * Real Time Clock Example
 *
 *
 */

/**
 * main.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "rtc.h"
#include "system_utils.h"
#include "i2c0.h"
#include "gyroscope.h"
#include "flash.h"
#include <math.h>
#include <periodic.h>

float offset_accel[3];
float offset_gyro[3];
char str[20];
int periodic_time_value;
int num_samples;
uint32_t starting_block_address = 0x16000;   // for some reason this address works
int hysterisis_threshold = 0;
int gating_parameters[10];

void InitHW(){
    SystemClockInit();
    GPIOInit();
    TriggerInit();
    RTCModuleRCGCInit();
    UARTRCGCInit();
    UARTInit();
    RTCInit();
    initI2c0();
    SetGatingParameters(gating_parameters);
}

void Reset()
{
    NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
}


int16_t SingleSample(float *accel, float *gyroscope_readings, int16_t *magnetic_readings,float *offset_accel, float *offset_gyro)
{
           int16_t temperature_readings;
           float divider = 0;

           ReadAccelerometer(accel);
           ReadGyroscope(gyroscope_readings);
           ReadMagnetometer(magnetic_readings);
           temperature_readings  = ReadTemperature();
           int i =0;
           for(i=0;i<3;i++)
           {
               accel[i] = accel[i] - offset_accel[i];
               divider += accel[i]*accel[i];
               gyroscope_readings[i] = gyroscope_readings[i] - offset_gyro[i];
           }

           divider = sqrt(divider);
           accel[0] = accel[0] / divider;
           accel[1] = accel[1] / divider;
           accel[2] = accel[2] / divider;
       return temperature_readings;
}

void Burst()
{
    float acceleration[3];
    float gyroscope_readings[3];
    int16_t magnetic_readings[3];
    int16_t temperature_readings = SingleSample(acceleration, gyroscope_readings, magnetic_readings,offset_accel, offset_gyro);
    int i = 0;
    for(i=0;i<3;i++)
     {
         sprintf(str,"Accelerometer %d %lf\n",i+1,acceleration[i]);
         putsUart0(str);
     }
   for(i=0;i<3;i++)
      {
          sprintf(str,"Gyroscope %d %lf\n",i+1,gyroscope_readings[i]);
          putsUart0(str);
      }
   for(i=0;i<3;i++)
     {
         sprintf(str,"Magnetometer %d %d\n",i+1,magnetic_readings[i]);
         putsUart0(str);
     }
   sprintf(str,"Temperature %d\n",temperature_readings);
   putsUart0(str);
   float heading = 180*(atan2(magnetic_readings[1],magnetic_readings[0]))/3.14;
   sprintf(str, "Heading %lf\n",heading);
   putsUart0(str);
}

int main(void)

{
    InitHW();
    putsUart0("Data Logger v1.0 - Aditya Rajguru and Sadat Bin Hossain");
    uint32_t seconds = 0;
    char string[100];
    char *token;
    int i=0;

    EnableMagnetometer();
    Calibrate(offset_accel,offset_gyro);

    while(1){
    putsUart0("Enter Commands below. \r\n");
    getsUart0(string,100);
    token = strtok(string," ");
    if(strcmp(token,"poll") == 0){
        putsUart0("Devices found: ");
                    for (i = 4; i < 119; i++)
                    {
                        if (pollI2c0Address(i))
                        {
                            sprintf(str, "0x%02x ", i);
                            putsUart0(str);
                        }
                    }
                    putsUart0("\r");
    }
    else if(strcmp(token,"burst") == 0)
    {
        Burst();

    }
    else if(strcmp(token,"periodic") == 0 )
    {
        token = strtok(NULL," ");
        if(strcmp(token,"off") == 0)
            {
                // do something.
            }
        periodic_time_value = asciiToUint8(token);
        EnableNoHibWakeUpPeriodic(periodic_time_value);
    }

    else if(strcmp(token,"trigger") ==0)
    {

    }

    else if(strcmp(token,"samples") ==0)
        {
            token = strtok(NULL," ");
            num_samples = asciiToUint8(token);
        }
    else if(strcmp(token,"save") == 0)
    {
        uint32_t pageBuffer[256] = {232};
        WritePageToFlash(starting_block_address,pageBuffer);
    }
  }
    return 0;
}


