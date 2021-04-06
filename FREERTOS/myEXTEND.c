#include "myEXTEND.h"
#include "tim3.h"

/////任务运行时间统计函数
volatile unsigned long long FreeRTOSRunTimeTicks;


void configTimerForRunTimeStats(void)
{
	FreeRTOSRunTimeTicks = 0;
	TIM3_Init(50-1,84-1);
}





//静态内存分配所需要实现的函数
#if configSUPPORT_STATIC_ALLOCATION				//使用静态内存分配

	//静态分配中空闲任务所需的内存，任务控制块
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

	//静态分配中定时器任务所需的内存，任务控制块
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
