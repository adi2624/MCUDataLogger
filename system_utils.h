// Util Init Library
// Aditya Rajguru

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL with MPU 9250
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz
// Hibernation Module Clock : 32768 Hz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef UTIL_H_
#define UTIL_H_

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"


void UARTRCGCInit();
void UARTInit();
void SystemClockInit();
void GPIOInit();
void itoA(uint32_t number,char* value);
uint8_t asciiToUint8(const char str[]);

#endif
