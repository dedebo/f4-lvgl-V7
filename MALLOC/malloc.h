#ifndef __MALLOC_H
#define __MALLOC_H
#include "stm32f4xx.h"
 
 
#ifndef NULL
#define NULL 0
#endif

//定义三个内存池
#define SRAMIN	 0		//内部RAM
#define SRAMEX   1		//外部RAM
#define SRAMCCM  2		//CCM(只能CPU使用)


#define SRAMBANK 	3	//定义支持SRAM的块数.	

//内部SRAM设置
#define MEM1_BLOCK_SIZE			32  	  						//内存块大小
#define MEM1_MAX_SIZE			32//0*1024  						//管理内存池大小
#define MEM1_ALLOC_TABLE_SIZE	MEM1_MAX_SIZE/MEM1_BLOCK_SIZE 	//内存块数量

//外部SRAM设置
#define MEM2_BLOCK_SIZE			32  	  						//�ڴ���СΪ32�ֽ�
#define MEM2_MAX_SIZE			400 *1024  						//管理内存池大小
#define MEM2_ALLOC_TABLE_SIZE	MEM2_MAX_SIZE/MEM2_BLOCK_SIZE 	//�ڴ���С
		 
//CCM设置
#define MEM3_BLOCK_SIZE			32  	  						//�ڴ���СΪ32�ֽ�
#define MEM3_MAX_SIZE			32//0 *1024  						//管理内存池大小
#define MEM3_ALLOC_TABLE_SIZE	MEM3_MAX_SIZE/MEM3_BLOCK_SIZE 	//�ڴ���С
		 


//�ڴ���������
struct _m_mallco_dev
{
	void (*init)(u8);					//��ʼ��
	u8 (*perused)(u8);		  	    	//�ڴ�ʹ����
	u8 	*membase[SRAMBANK];				//�ڴ�� ����SRAMBANK��������ڴ�
	u16 *memmap[SRAMBANK]; 				//�ڴ����״̬��
	u8  memrdy[SRAMBANK]; 				//�ڴ�����Ƿ����
};
extern struct _m_mallco_dev mallco_dev;	 //��mallco.c���涨��

void mymemset(void *s,u8 c,u32 count);	//�����ڴ�
void mymemcpy(void *des,void *src,u32 n);//�����ڴ�     
void my_mem_init(u8 memx);				//�ڴ�����ʼ������(��/�ڲ�����)
u32 my_mem_malloc(u8 memx,u32 size);	//�ڴ����(�ڲ�����)
void *my_mem_free(u8 memx,u32 offset);		//�ڴ��ͷ�(�ڲ�����)
u8 my_mem_perused(u8 memx);				//����ڴ�ʹ����(��/�ڲ�����) 
////////////////////////////////////////////////////////////////////////////////
//外部调用的函数
void *myfree(u8 memx,void *ptr);  			//释放
void *mymalloc(u8 memx,u32 size);			//分配
void *myrealloc(u8 memx,void *ptr,u32 size);//重分配
#endif













