#ifndef __FLASH_H__
#define __FLASH_H__

#include "main.h"

void FLASH_Inside_Wr(uint32_t addr, uint32_t Pdata);
void FLASH_Inside_Wr_Multi(uint32_t addr, uint32_t* Pdata, uint32_t len);
uint32_t FLASH_Inside_Rd(uint32_t addr);

#endif
