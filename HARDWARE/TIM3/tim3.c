#include "tim3.h"
#include "usart.h"




//定时器3，用于给GUI提供心跳时钟
void TIM3_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef Tim_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	Tim_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	Tim_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	Tim_TimeBaseInitStruct.TIM_Period = arr;
	Tim_TimeBaseInitStruct.TIM_Prescaler = psc;
	//Tim_TimeBaseInitStruct.TIM_RepetitionCounter = 
	
	
	TIM_TimeBaseInit(TIM3,&Tim_TimeBaseInitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);					//开启中断
	
	TIM_Cmd(TIM3,ENABLE);
}



#include "lvgl.h"
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
	{
		
		lv_tick_inc(1);	//一毫秒
	}
	
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	
}

