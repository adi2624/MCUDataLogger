#include "trigger.h"
#include "tm4c123gh6pm.h"
#include "gyroscope.h"
#include <stdint.h>
#include<math.h>
#include "uart0.h"


extern int hysteris_threshold;
extern int gating_lower_acceleration;
extern int gating_upper_acceleration;
extern int gating_lower_temperature;
extern int gating_upper_temperature;
extern int num_samples;
extern void Burst();

float offset_gyroscope[3] = {0,0,0};
float offset_acceleration[3] = {0,0,0};





