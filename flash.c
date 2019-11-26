#include "flash.h"

#define PAGE_SIZE 1024
#define WORD_SIZE 128

void WritePageToFlashLevelling(uint32_t key)
{
    //we need to determine the address of the starting block for the page

}

void WritePageToFlash(uint32_t address, uint32_t pageBuffer[256])   // writing 1024 bytes as 256, 4 byte integers
{
    FLASH_FMA_R = address;
    FLASH_FMC_R = FLASH_FMC_WRKEY | FLASH_FMC_ERASE;    //erase 1k page at address
    while(FLASH_FMC_R & FLASH_FMC_ERASE);   // wait until the board clears the bit and signals op complete.

    uint32_t *buffer = (uint32_t*)&FLASH_FWBN_R;
    uint32_t k = 0;
    int block = 0;
    int j = 0;

    for(block=0; block < PAGE_SIZE/WORD_SIZE; block++)
    {
        FLASH_FMA_R = address;

        // copy 32 4 byte integers into the 32 FWBN registers

        for(j=0; j< 32; j++)
        {
            buffer[j] = pageBuffer[k++];        // Each integer is 4 bytes, 32 of them get copied for each iteration of the loop as a 128 byte word
        }

        FLASH_FMC2_R = FLASH_FMC_WRKEY | FLASH_FMC2_WRBUF;  // the WRBUF bit tells to copy data from FWBn registers to FMA address
        while(FLASH_FMC2_R & FLASH_FMC2_WRBUF);  // wait for operation to complete

        address += 32 * sizeof(uint32_t); // 32*4bytes

    }

}
