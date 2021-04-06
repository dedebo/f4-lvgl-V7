#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sys.h"
#include "usart.h"	
///////////////////////////////一些全局变量的声明///////////////////////////////////////// 	 
extern SemaphoreHandle_t USART_Semaphore;

//////////////////////////////////////////////////////////////////
#if 1
#pragma import(__use_no_semihosting)             
                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
void _sys_exit(int x) 
{ 
	x = x; 
} 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);
	USART1->DR = (u8) ch;      
	return ch;
}
#endif
 
#if EN_USART1_RX   
	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲区


//bit15 ：收到了0x0a
//bit14 :收到了0x0b
//bit0-13 :计算接收到的字符的个数
u16 USART_RX_STA=0;       //先清空接收接收状态寄存器	

//串口初始化
//bound:波特率
void uart_init(u32 bound){
   
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	
	//时钟线使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
 
	//io口功能复用
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); 
	
	//GPIO配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;				
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 				
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 				
	GPIO_Init(GPIOA,&GPIO_InitStructure); 						

   //USART1 配置
	USART_InitStructure.USART_BaudRate = bound;										
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							
	USART_InitStructure.USART_Parity = USART_Parity_No;								
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					
	USART_Init(USART1, &USART_InitStructure);											
	
	USART_Cmd(USART1, ENABLE);  //串口使能 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if EN_USART1_RX	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//允许中断

	//Usart1 NVIC 
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=6;			//收到FreeRTOS控制
		//NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;			
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				
	NVIC_Init(&NVIC_InitStructure);					

#endif
	
}


//中断服务函数
void USART1_IRQHandler(void)                	
{
	u8 Res;
	static  BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)   //接收中断触发
	{
		Res =USART_ReceiveData(USART1);						//(USART1->DR);	读取接收到的内容
		
		if((USART_RX_STA&0x8000)==0)						//一次接收还没有结束
		{
			if(USART_RX_STA&0x4000)							//已经接收到0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//判断这次收到的数据是否是0x0a
				else USART_RX_STA|=0x8000;	//如果是 将bit15置1
			}
			else 	//还没有收到0x0d
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;	//如果收到0x0d,将bit14置1
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;	//每接收到一次数据，便加1
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;	//超出范围，无效
				}		 
			}
		}   		 
	} 
	if(USART_RX_STA&0x8000)	//一次接收结束
	{
		 xSemaphoreGiveFromISR( USART_Semaphore, &xHigherPriorityTaskWoken );
	}
	 portYIELD_FROM_ISR( xHigherPriorityTaskWoken );//如果高优先级的任务获得了信号量，则需要进行一次任务切换

} 
#endif	

 



