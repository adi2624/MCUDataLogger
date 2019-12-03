#include "data.h"
#include <stdint.h>
#include <string.h>
#include "uart0.h"
void ReadData(uint32_t address, uint32_t readBuffer[256])
{
    int i=0;
    for(i=0;i<256;i++)
    {
        readBuffer[i] = *(uint32_t*)(address);
        address += 4;
    }
}

void ParseData(uint32_t readBuffer[256])
{
    int counter = 0;
    uint8_t readConverted[1024];

    /*

    for(counter = 0; counter < 256;counter++)
    {
        /*
        uint32_t value_to_convert = readBuffer[counter];

        int l = 0;
        for(l=0; l<4;l++)
        {
            readConverted[ (counter*4) + l] = (value_to_convert >>= 8);
        }


    }

    */

    for(counter=0;counter< (PAGE_SIZE/RECORD_SIZE); counter++)  // this will print out contents extracted from a whole page
    {
        struct Record extracted;

        char string[100];

        extracted.day = (int)readBuffer[ (counter*RECORD_SIZE) + 0 ];
        extracted.month = (int)readBuffer[ (counter*RECORD_SIZE) + 1 ];
        extracted.hour = (int)readBuffer[ (counter*RECORD_SIZE) + 2 ];
        extracted.minute = (int)readBuffer[ (counter*RECORD_SIZE) + 3 ];
        extracted.second = (int)readBuffer[ (counter*RECORD_SIZE) + 4 ];
        extracted.accel_vector = *(float*)&readBuffer[ (counter*RECORD_SIZE) + 5];
        extracted.temp = (int)readBuffer[ (counter*RECORD_SIZE) + 6 ];
        extracted.mag_heading = *(float*)&readBuffer[ (counter*RECORD_SIZE) + 7 ];
        extracted.gyro_x = *(float*)&readBuffer[ (counter*RECORD_SIZE) + 8];
        extracted.gyro_y = *(float*)&readBuffer[ (counter*RECORD_SIZE) + 9 ];
        extracted.gyro_z = *(float*)&readBuffer[ (counter*RECORD_SIZE) + 10 ];

        sprintf(string,"\n H: %d M: %d S: %d D: %d M: %d \t",(int)extracted.hour, (int)extracted.minute, (int)extracted.second, (int)extracted.day, (int)extracted.month);
        putsUart0(string);
        sprintf(string,"Acceleration Vector: %d\t",(int)extracted.accel_vector);
        putsUart0(string);
        sprintf(string,"Magnetic Heading: %d\t", (int)extracted.mag_heading);
        putsUart0(string);
        sprintf(string, "Temperature: %d \t",(int)extracted.temp);
        putsUart0(string);
        sprintf(string, "Gyro X: %d Gyro Y: %d Gyro Z: %d \n",(int)extracted.gyro_x,(int)extracted.gyro_y,(int)extracted.gyro_z);
        putsUart0(string);

    }
}
