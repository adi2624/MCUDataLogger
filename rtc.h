// RTC Library
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

#ifndef RTC_H_
#define RTC_H_



//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void isHibWriteComplete();
int getSecondsValue();
void RTCModuleRCGCInit();
void LoadRTCValue(uint32_t value);
void RTCMatchSetup(uint32_t match_value, uint32_t load_value);
void HibernateMatchISR();
void StartRTCCounting();
void RTCInit();

#endif
