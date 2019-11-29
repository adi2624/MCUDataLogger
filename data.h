#include<stdint.h>

#define RECORD_SIZE 44
#define PAGE_SIZE 1024

struct Record{

    int day;
    int month;
    int hour;
    int minute;
    int second;
    float accel_vector;
    int temp;
    float mag_heading;
    float gyro_x;
    float gyro_y;
    float gyro_z;
};

void ReadData(uint32_t address, uint32_t readBuffer[256]);
void ParseData(uint32_t readBuffer[256]);
