#include "data.h"
#include<stdint.h>


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

    for(counter = 0; counter < 256;counter++)
    {
        uint32_t value_to_convert = readBuffer[counter];

        int l = 0;
        for(l=0; l<4;l++)
        {
            readConverted[ (counter*4) + l] = (value_to_convert >>= 8);
        }

    }



    for(counter=0;counter< (PAGE_SIZE/RECORD_SIZE); counter++)
    {
        struct Record extracted;

        uint8_t byte_array[RECORD_SIZE];

        char string[100];

        int i =0;

        for(i=0;i<RECORD_SIZE;i++)
        {
            byte_array[i] = readConverted[ (counter*RECORD_SIZE) + i ];
        }

        // at this point byte_array is full, copy into structure

        memcpy(&extracted, byte_array, RECORD_SIZE);
        sprintf(string,"%lf",extracted.mag_heading);
        putsUart0(string);


    }
}
