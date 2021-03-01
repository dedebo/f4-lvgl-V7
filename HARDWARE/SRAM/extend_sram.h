#ifndef _EXTEND_H
#define _EXTEND_H
#include "sys.h"


#define FSMC_BANK1_BLOCK3 ((u32)0x68000000) //使用的第三块内存，第四块由LCD使用了

void FSMC_SRAM_Init(void);
void FSMC_SRAM_WriteBuffer(u8* pBuffer,u32 WriteAddr,u32 NumHalfwordToWrite);
void FSMC_SRAM_ReadBuffer(u8* pBuffer,u32 ReadAddr,u32 NumHalfwordToRead);




#endif
