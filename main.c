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
#include "trigger.h"
#include "i2c0.h"
#include "gyroscope.h"
#include "flash.h"
#include "time.h"
#include "data.h"
#include <math.h>
#include <periodic.h>

float offset_accel[3];
float offset_gyro[3];
char str[20];
int periodic_time_value;
int gating_parameters[10];      // 0 - lowtemp, 1- hightemp, 2-lowaccel, 3-highaccel
int num_samples;
uint32_t starting_block_address = 0x10000;   // for some reason this address works
int nPages = 0;
int hysterisis_threshold = 0;
int gating_lower_acceleration = 0;
int gating_upper_acceleration = 0;
int gating_lower_temperature = 0;
int gating_upper_temperature = 0;
struct Time set_time = {0,0,0,0};
struct Date set_date = {0,0,0};
int log_mask[4] = {-1,-1,-1,-1};
int hysteresis_mask[4] = {-1,-1,-1,-1};
float acceleration_vector_prev_trigger = 0;
//float gyroscope_readings_prev_trigger[3]= {0,0,0};
float magnetic_heading_prev_trigger = 0;
int temperature_readings_prev_trigger = 0;
uint8_t page[1024];
int k=0; //counter for num elements in buffer
uint32_t current_address = 0x10000;


enum Log{Compass=0, Accel=1, Gyro=2, Temp=3};



void InitHW(){
    SystemClockInit();
    GPIOInit();
    TriggerInit();
    RTCModuleRCGCInit();
    UARTRCGCInit();
    UARTInit();
    RTCInit();
    initI2c0();
    StartRTCCounting();
}

/*
 *
 *
 * Utility Functions
 *
 *
 *
 */

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
    struct Record to_write;
    int i = 0;

    uint32_t difference_value = CalculateCurrentDifference(&set_time);
    UpdateDateandTimeValues(&set_date, &set_time, difference_value);

    to_write.day = set_date.day;
    to_write.month = set_date.month;

    to_write.hour = set_time.hour;
    to_write.minute = set_time.minute;
    to_write.second = set_time.second;

    if(log_mask[1] == 1)
    {

        for(i=0;i<3;i++)
         {
             sprintf(str,"Accelerometer %d %lf\n",i+1,acceleration[i]);
             putsUart0(str);
         }

        to_write.accel_vector = sqrt(acceleration[0]*acceleration[0] + acceleration[1]*acceleration[1] + acceleration[2]*acceleration[2]);
    }
    if(log_mask[2] == 1)
    {
       for(i=0;i<3;i++)
          {
              sprintf(str,"Gyroscope %d %lf\n",i+1,gyroscope_readings[i]);
              putsUart0(str);
          }

       to_write.gyro_x = gyroscope_readings[0];
       to_write.gyro_y = gyroscope_readings[1];
       to_write.gyro_z = gyroscope_readings[2];
    }

    if(log_mask[0] == 1)
    {
       for(i=0;i<3;i++)
         {
             sprintf(str,"Magnetometer %d %d\n",i+1,magnetic_readings[i]);
             putsUart0(str);
         }

       to_write.mag_heading = 180*(atan2(magnetic_readings[1],magnetic_readings[0]))/3.14;
    }
    if(log_mask[3] == 1)
    {
       sprintf(str,"Temperature %d\n",temperature_readings);
       putsUart0(str);
       float heading = 180*(atan2(magnetic_readings[1],magnetic_readings[0]))/3.14;
       sprintf(str, "Heading %lf\n",heading);
       putsUart0(str);
       to_write.temp = temperature_readings;
    }

    uint8_t  buffer[sizeof(struct Record)];
    memcpy(buffer, &to_write, sizeof(struct Record));
    if( (k+sizeof(buffer)) < 1023)
    {
        for(i=0;i<sizeof(buffer);i++)
        {
            page[k] = buffer[i];
            k++;
        }
    }

    else
    {
        uint32_t buffer_converted[256];

        for(i=0;i<1023;i++)
        {
            buffer_converted[(i/4)] >>= 8;
            buffer_converted[(i/4)] += buffer[i] << 24;
        }

        WritePageToFlash(current_address, buffer_converted);
        putsUart0("Page Fill Complete. Write to flash successful!\n");
        nPages++;
        k = 0;
        current_address += 1024;
    }




}

int CheckTempGating()
{
    if( (gating_parameters[0] ==0 ) && (gating_parameters[1] == 0))
    {
        return 0;
    }

    else
    {
        return 1;
    }

}

int CheckAccelGating()
{
    if( (gating_parameters[2]==0) && (gating_parameters[3] == 0))
        {
            return 0;
        }

        else
        {
            return 1;
        }

}

/*
 *
 *
 *
 * Interrupt Service Routines
 *
 *
 *
 *
 */


void TriggerISR()
{
    GPIO_PORTF_ICR_R |= PUSH_BUTTON_MASK;   //clear the interrupt

    float acceleration[3];
    float gyroscope_readings[3];
    int16_t magnetic_readings[3];
    float absolute_acceleration_cmp_value;
    int counter = 0;

    int16_t current_temp = SingleSample(acceleration, gyroscope_readings, magnetic_readings, offset_accel, offset_gyro);

    if(CheckAccelGating() && CheckTempGating() )  // gating values are not zero, therefore it has been activated
    {
        float absolute_acceleration_cmp_value = sqrt(acceleration[0]*acceleration[0] + acceleration[1]*acceleration[1] + acceleration[2]*acceleration[2]);
        if( (absolute_acceleration_cmp_value >= gating_parameters[2]) && (absolute_acceleration_cmp_value <= gating_parameters[3]) )
        {
            if( (current_temp >= gating_parameters[0]) && (current_temp <= gating_parameters[1]) )
            {
                if( (hysteresis_mask[1] == -1) || (hysteresis_mask[3] == -1))
                {
                    // hysterisis is turned off
                    for(counter =0; counter<num_samples;counter++)
                    {
                    Burst();
                    }
                }

                else if(((absolute_acceleration_cmp_value - acceleration_vector_prev_trigger) >= hysteresis_mask[1]) && ((current_temp - temperature_readings_prev_trigger) >= hysteresis_mask[3]))
                {

                int i = 0;
                    for(i=0; i< num_samples; i++)
                    {
                        Burst();
                    }
                }

            }

            else
            {
                // One of the parameters failed the gating condition.
            }

        }
    }

    else if (CheckAccelGating())
    {
        absolute_acceleration_cmp_value = sqrt(acceleration[0]*acceleration[0] + acceleration[1]*acceleration[1] + acceleration[2]*acceleration[2]);
        if( (absolute_acceleration_cmp_value >= gating_parameters[2]) && (absolute_acceleration_cmp_value <= gating_parameters[3]) )
        {
            if(hysteresis_mask[1] == -1)
            {
                //hysterisis is turned off
                for(counter =0; counter<num_samples;counter++)
                {
                Burst();
                }
            }

            else if( ((absolute_acceleration_cmp_value - acceleration_vector_prev_trigger) >=hysteresis_mask[1]))
            {
                for(counter =0; counter<num_samples;counter++)
                {
                    Burst();
                }
            }
        }

        else
        {
            // failed gating condition, no trigger
        }
    }

    else if (CheckTempGating())
    {
        if( (current_temp <= gating_parameters[1]) && (current_temp >= gating_parameters[0]) )
          {

            if(hysteresis_mask[3] == -1)
            {
                // hysterisis is turned off
                for(counter =0; counter<num_samples;counter++)
                {
                    Burst();
                }
            }

            else if(((current_temp - temperature_readings_prev_trigger) >= hysteresis_mask[3]))
            {
                for(counter =0; counter<num_samples;counter++)
                {
                    Burst();
                }
            }
          }

        else
        {
            // gating condition failed
        }
    }


    else
    {
        for(counter =0; counter<num_samples;counter++)
        {
            Burst();
        }
    }


    // Update previous values for hysterisis calculation

    acceleration_vector_prev_trigger = absolute_acceleration_cmp_value;
    magnetic_heading_prev_trigger = 180*(atan2(magnetic_readings[1],magnetic_readings[0]))/3.14;
    temperature_readings_prev_trigger = current_temp;


}


/*
 *
 *
 * Main
 *
 *
 *
 */
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
    if(strcmp(token,"reset") == 0)
    {
        Reset();
    }
    else if(strcmp(token, "temp") == 0)
    {
        int temperature = ReadTemperature();
        sprintf(str,"The current temperature is %d \n", temperature);
        putsUart0(str);
    }
    else if(strcmp(token,"time") == 0)
    {
        token = strtok(NULL, " ");

        if(token == NULL)
        {
            // Here you just have to display the current time values

            uint32_t difference_value = CalculateCurrentDifference(&set_time);
            UpdateDateandTimeValues(&set_date, &set_time, difference_value);
            sprintf(str,"The time is H:%d M:%d S:%d \n", set_time.hour, set_time.minute, set_time.second);
            putsUart0(str);
        }

        else
        {
            int value_time[3];
            int i=0;

            //Set the time

            while(token != NULL)
            {
                value_time[i] = asciiToUint8(token);
                //sprintf(str,"Value %d",value);
                token = strtok(NULL, " ");
                //putsUart0(str);
                i++;
            }

            InitializeTimeStructure(&set_time, value_time[0], value_time[1], value_time[2]);
        }
    }

    else if(strcmp(token, "date") == 0)
    {
        token = strtok(NULL, " ");

        if(token == NULL)
        {
            uint32_t difference_value = CalculateCurrentDifference(&set_time);
            UpdateDateandTimeValues(&set_date, &set_time, difference_value);
            sprintf(str,"The date is D:%d M:%d Y:%d \n", set_date.day, set_date.month, set_date.year);
            putsUart0(str);
        }

        else
        {
            int value_date[3];  // Day Month Year
            int i = 0;

            //Set the date

            while(token!=NULL)
            {
                value_date[i] = asciiToUint8(token);
                token = strtok(NULL, " ");
                i++;
            }

            InitializeDateStructure(&set_date, value_date[0], value_date[1], value_date[2]);
        }
    }

    else if(strcmp(token,"poll") == 0){
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
        int current_readings = (int)getSecondsValue();
        RTCMatchSetupNoHib(current_readings + periodic_time_value, periodic_time_value);
    }

    else if(strcmp(token, "hysteresis") == 0)
    {
        token = strtok(NULL, " ");

        if(strcmp(token,"accel") == 0)
        {
            token = strtok(NULL, " ");
            hysteresis_mask[1] = asciiToUint8(token);
        }

        else if(strcmp(token, "temp") == 0)
        {
            token = strtok(NULL, " ");
            hysteresis_mask[3] = asciiToUint8(token);
        }
    }

    else if(strcmp(token,"trigger") ==0)
    {
        GPIO_PORTF_IM_R |= 1;   // enable interrupts for Push Button 2
    }

    else if(strcmp(token, "gating") == 0)
    {
        token = strtok(NULL, " ");

        if(strcmp(token,"temp") == 0)
        {
            int i = 0;

            while(token!=NULL)
            {
                token = strtok(NULL, " ");
                if(i == 0)
                {
                    //gating_upper_temperature = asciiToUint8(token);
                    gating_parameters[1] = asciiToUint8(token);
                }

                else if(i == 1)
                {
                   // gating_lower_temperature = asciiToUint8(token);
                    gating_parameters[0] = asciiToUint8(token);
                }

                i++;
            }
        }

        else if(strcmp(token, "accel") == 0)
        {
            token = strtok(NULL, " ");

            int i = 0;

            while(token!=NULL)
            {
                if(i == 0)
               {
                   //gating_upper_acceleration = asciiToUint8(token);
                    gating_parameters[3] = asciiToUint8(token);
               }

               else if(i == 1)
               {
                   //gating_lower_acceleration = asciiToUint8(token);
                   gating_parameters[2] = asciiToUint8(token);
               }

               token = strtok(NULL, " ");

               i++;
            }
        }
    }

    else if(strcmp(token,"stop") == 0)
    {
        GPIO_PORTF_IM_R &= ~1;
        periodic_time_value = 0;
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

    else if(strcmp(token,"log") == 0)
    {
        token = strtok(NULL, " ");

        if(strcmp(token,"compass") == 0)
        {
            log_mask[0] = 1;
        }
        else if(strcmp(token,"accel") ==0 )
        {
            log_mask[1] = 1;
        }
        else if(strcmp(token,"gyro") == 0)
        {
            log_mask[2] = 1;
        }
        else if(strcmp(token, "temp") == 0)
        {
            log_mask[3] = 1;
        }
    }

    else if(strcmp(token, "test") == 0)
    {
        int i=0;
        for(i=0;i<23;i++)
        {
            Burst();
        }
    }

    else if(strcmp(token,"data") == 0)
    {
        int i =0;
        for(i=0; i<nPages;i++)
        {
            uint32_t address_to_check = starting_block_address + (i)*(1024);
            uint32_t readData[256];
            ReadData(address_to_check,readData);
            ParseData(readData);
        }

    }

  }
    return 0;
}


