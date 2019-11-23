// Init Utils Library
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

#include "system_utils.h"
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

#define UART_TX_MASK 2
#define UART_RX_MASK 1


void UARTRCGCInit(){
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;
}

void UARTInit(){
   GPIO_PORTA_DIR_R |= UART_TX_MASK;                   // enable output on UART0 TX pin
   GPIO_PORTA_DIR_R &= ~UART_RX_MASK;                   // enable input on UART0 RX pin
   GPIO_PORTA_DR2R_R |= UART_TX_MASK;                  // set drive strength to 2mA (not needed since default configuration -- for clarity)
   GPIO_PORTA_DEN_R |= UART_TX_MASK | UART_RX_MASK;    // enable digital on UART0 pins
   GPIO_PORTA_AFSEL_R |= UART_TX_MASK | UART_RX_MASK;  // use peripheral to drive PA0, PA1
   GPIO_PORTA_PCTL_R &= ~(GPIO_PCTL_PA1_M | GPIO_PCTL_PA0_M); // clear bits 0-7
   GPIO_PORTA_PCTL_R |= GPIO_PCTL_PA1_U0TX | GPIO_PCTL_PA0_U0RX;
                                                       // select UART0 to drive pins PA0 and PA1: default, added for clarity

   // Configure UART0 to 115200 baud, 8N1 format
   UART0_CTL_R = 0;                                    // turn-off UART0 to allow safe programming
   UART0_CC_R = UART_CC_CS_SYSCLK;                     // use system clock (40 MHz)
   UART0_IBRD_R = 21;                                  // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
   UART0_FBRD_R = 45;                                  // round(fract(r)*64)=45
   UART0_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;    // configure for 8N1 w/ 16-level FIFO
   UART0_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN;
                                                       // enable TX, RX, and module
}

void SystemClockInit(){

    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);
}

void GPIOInit(){

    SYSCTL_GPIOHBCTL_R = 0; // Set to use Advanced Peripheral Bus

}

void itoA(uint32_t number,char* value)
{
    uint16_t digit = 0;
    uint8_t char_counter = 0;

    if(number == 0)
    {
        value[0] = digit + 48;
        value[1] = '\0';                    //return '0\0' if number is 0
        return;

    }
    while(number>0)
    {
        digit = number%10;
        number = number/10;                     //extract the number into value array.
        value[char_counter] = digit + 48;
        char_counter++;
    }
    char_counter--;                             //set char counter to index of last digit

    uint8_t start_counter = 0;
    uint8_t end_counter = char_counter;
    while(end_counter > char_counter/2 )
        {
                    char temp = value[end_counter];
                    value[end_counter] = value[start_counter];
                    value[start_counter] = temp;                    // reverse string to get digits in order.
                    start_counter++;
                    end_counter--;
        }
    value[char_counter+1] = '\0';       //place null character after the last digit
    return;
}

uint8_t asciiToUint8(const char str[])
{
    uint8_t data;
    if (str[0] == '0' && tolower(str[1]) == 'x')
        sscanf(str, "%hhx", &data);
    else
        sscanf(str, "%hhu", &data);
    return data;
}




