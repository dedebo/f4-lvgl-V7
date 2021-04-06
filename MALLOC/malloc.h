#ifndef __MALLOC_H
#define __MALLOC_H
#include "stm32f4xx.h"
 
 
#ifndef NULL
#define NULL 0
#endif

//å®šä¹‰ä¸‰ä¸ªå†…å­˜æ± 
#define SRAMIN	 0		//å†…éƒ¨RAM
#define SRAMEX   1		//å¤–éƒ¨RAM
#define SRAMCCM  2		//CCM(åªèƒ½CPUä½¿ç”¨)


#define SRAMBANK 	3	//å®šä¹‰æ”¯æŒSRAMçš„å—æ•°.	

//å†…éƒ¨SRAMè®¾ç½®
#define MEM1_BLOCK_SIZE			32  	  						//å†…å­˜å—å¤§å°
#define MEM1_MAX_SIZE			32//0*1024  						//ç®¡ç†å†…å­˜æ± å¤§å°
#define MEM1_ALLOC_TABLE_SIZE	MEM1_MAX_SIZE/MEM1_BLOCK_SIZE 	//å†…å­˜å—æ•°é‡

//å¤–éƒ¨SRAMè®¾ç½®
#define MEM2_BLOCK_SIZE			32  	  						//ÄÚ´æ¿é´óĞ¡Îª32×Ö½Ú
#define MEM2_MAX_SIZE			400 *1024  						//ç®¡ç†å†…å­˜æ± å¤§å°
#define MEM2_ALLOC_TABLE_SIZE	MEM2_MAX_SIZE/MEM2_BLOCK_SIZE 	//ÄÚ´æ±í´óĞ¡
		 
//CCMè®¾ç½®
#define MEM3_BLOCK_SIZE			32  	  						//ÄÚ´æ¿é´óĞ¡Îª32×Ö½Ú
#define MEM3_MAX_SIZE			32//0 *1024  						//ç®¡ç†å†…å­˜æ± å¤§å°
#define MEM3_ALLOC_TABLE_SIZE	MEM3_MAX_SIZE/MEM3_BLOCK_SIZE 	//ÄÚ´æ±í´óĞ¡
		 


//ÄÚ´æ¹ÜÀí¿ØÖÆÆ÷
struct _m_mallco_dev
{
	void (*init)(u8);					//³õÊ¼»¯
	u8 (*perused)(u8);		  	    	//ÄÚ´æÊ¹ÓÃÂÊ
	u8 	*membase[SRAMBANK];				//ÄÚ´æ³Ø ¹ÜÀíSRAMBANK¸öÇøÓòµÄÄÚ´æ
	u16 *memmap[SRAMBANK]; 				//ÄÚ´æ¹ÜÀí×´Ì¬±í
	u8  memrdy[SRAMBANK]; 				//ÄÚ´æ¹ÜÀíÊÇ·ñ¾ÍĞ÷
};
extern struct _m_mallco_dev mallco_dev;	 //ÔÚmallco.cÀïÃæ¶¨Òå

void mymemset(void *s,u8 c,u32 count);	//ÉèÖÃÄÚ´æ
void mymemcpy(void *des,void *src,u32 n);//¸´ÖÆÄÚ´æ     
void my_mem_init(u8 memx);				//ÄÚ´æ¹ÜÀí³õÊ¼»¯º¯Êı(Íâ/ÄÚ²¿µ÷ÓÃ)
u32 my_mem_malloc(u8 memx,u32 size);	//ÄÚ´æ·ÖÅä(ÄÚ²¿µ÷ÓÃ)
void *my_mem_free(u8 memx,u32 offset);		//ÄÚ´æÊÍ·Å(ÄÚ²¿µ÷ÓÃ)
u8 my_mem_perused(u8 memx);				//»ñµÃÄÚ´æÊ¹ÓÃÂÊ(Íâ/ÄÚ²¿µ÷ÓÃ) 
////////////////////////////////////////////////////////////////////////////////
//å¤–éƒ¨è°ƒç”¨çš„å‡½æ•°
void *myfree(u8 memx,void *ptr);  			//é‡Šæ”¾
void *mymalloc(u8 memx,u32 size);			//åˆ†é…
void *myrealloc(u8 memx,void *ptr,u32 size);//é‡åˆ†é…
#endif













