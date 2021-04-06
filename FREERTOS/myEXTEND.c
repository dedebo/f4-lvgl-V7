#include "myEXTEND.h"
#include "tim3.h"

/////��������ʱ��ͳ�ƺ���
volatile unsigned long long FreeRTOSRunTimeTicks;


void configTimerForRunTimeStats(void)
{
	FreeRTOSRunTimeTicks = 0;
	TIM3_Init(50-1,84-1);
}





//��̬�ڴ��������Ҫʵ�ֵĺ���
#if configSUPPORT_STATIC_ALLOCATION				//ʹ�þ�̬�ڴ����

	//��̬�����п�������������ڴ棬������ƿ�
	static StaticTask_t IdleTaskTCB;
	static StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];
	void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, 
										StackType_t **ppxIdleTaskStackBuffer, 
										uint32_t *pulIdleTaskStackSize )
	{
		*ppxIdleTaskTCBBuffer = &IdleTaskTCB;
		*ppxIdleTaskStackBuffer = IdleTaskStack;
		*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	}

	//��̬�����ж�ʱ������������ڴ棬������ƿ�
	static StaticTask_t TimerTaskTCB;
	static StackType_t  TimerTaskStack[configTIMER_TASK_STACK_DEPTH];
	void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
										 StackType_t **ppxTimerTaskStackBuffer, 
										 uint32_t *pulTimerTaskStackSize )
	{
		*ppxTimerTaskTCBBuffer = &TimerTaskTCB;
		*ppxTimerTaskStackBuffer = TimerTaskStack;
		*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
	}

#endif
