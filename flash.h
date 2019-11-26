#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"

void WritePageToFlashLevelling(uint32_t key);
void WritePageToFlash(uint32_t address,uint32_t pageBuffer[256]);
