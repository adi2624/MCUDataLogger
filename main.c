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
#include <math.h>

void InitHW(){
    SystemClockInit();
    GPIOInit();
    RTCModuleRCGCInit();
    UARTRCGCInit();
    UARTInit();
    RTCInit();
    initI2c0();
}



int main(void)
{
    InitHW();
    putsUart0("Data Logger v1.0 - Aditya Rajguru and Sadat Bin Hossain");
    uint32_t seconds = 0;
    char string[100];
    char str[20];
    float offset_accel[3];
    float offset_gyro[3];
    int i=0;

    RTCMatchSetup(20,0);  // Give me an alert in 20 seconds.
    StartRTCCounting();
    putsUart0("\nCounter has started!\r\n");

    EnableMagnetometer();
    Calibrate(offset_accel,offset_gyro);

    while(1){
    putsUart0("Enter Commands below. \r\n");
    getsUart0(string,100);
    if(strcmp(string,"poll") == 0){
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
    else if(strcmp(string,"burst") == 0){
        float accel[3];
        float gyroscope_readings[3];
        int16_t temperature_readings;
        int16_t magnetic_readings[3];
        ReadAccelerometer(accel);
        for(i=0;i<3;i++)
        {
            sprintf(str,"Accelerometer %d %lf\n",i+1,accel[i] - offset_accel[i]);
            putsUart0(str);
        }
        ReadGyroscope(gyroscope_readings);
        for(i=0;i<3;i++)
        {
            sprintf(str,"Gyroscope %d %lf\n",i+1,gyroscope_readings[i] - offset_gyro[i]);
            putsUart0(str);
        }
        temperature_readings  = ReadTemperature();
        sprintf(str,"Temperature %d\n",temperature_readings);
        putsUart0(str);
        ReadMagnetometer(magnetic_readings);
        for(i=0;i<3;i++)
        {
            sprintf(str,"Magnetometer %d %d\n",i+1,magnetic_readings[i]);
            putsUart0(str);
        }

        float heading = 180*(atan2(magnetic_readings[1],magnetic_readings[0]))/3.14;
        sprintf(str, "Heading %lf\n",heading);
        putsUart0(str);

    }
  }
    return 0;
}


