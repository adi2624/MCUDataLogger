#include "periodic.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "rtc.h"
#include "uart0.h"

void EnableNoHibWakeUpPeriodic(int time_value)
{
    RTCMatchSetupNoHib(time_value,0);
    StartRTCCounting();
    putsUart0("\nCounter has started!\r\n");

}
