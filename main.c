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
}
int main(void)
{
    InitHW();
    putsUart0("Data Logger v1.0 - Aditya Rajguru and Sadat Bin Hossain");
    uint32_t seconds = 0;
    char string[100];

    RTCMatchSetup(20,0);  // Give me an alert in 20 seconds.
    StartRTCCounting();


    while(1){
    seconds = getSecondsValue();
    itoA(seconds, string);
    putsUart0(string);
    //putsUart0("\r\n");
    }


    return 0;
}


