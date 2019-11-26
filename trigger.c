#include "trigger.h"
#include "tm4c123gh6pm.h"
#include "gyroscope.h"
#include <stdint.h>
#include "uart0.h"

#define PUSH_BUTTON_MASK 1

extern int hysteris_threshold;
extern int* gating_parameters;

void TriggerISR()
{
    GPIO_PORTF_ICR_R |= PUSH_BUTTON_MASK;   //clear the interrupt
    int current_temp = ReadTemperature();
    if( (current_temp >= gating_parameters[0]) && (current_temp <= gating_parameters[1]) ){     // extern int* gating parameters not updating correctly
        putsUart0("Trigger detected!\n");
    }

    else
    {
        putsUart0("Trigger was detected but the gating failed!\n");
    }
}
