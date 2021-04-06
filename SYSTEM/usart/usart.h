#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 


////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//接收缓冲区大小 200
#define EN_USART1_RX 			1		//使能接收
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲区
extern u16 USART_RX_STA;         		//状态	
//初始化函数
void uart_init(u32 bound);
#endif


