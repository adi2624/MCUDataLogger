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
    int i=0;

    RTCMatchSetup(20,0);  // Give me an alert in 20 seconds.
    StartRTCCounting();
    putsUart0("\nCounter has started!\r\n");


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
  }
    return 0;
}


