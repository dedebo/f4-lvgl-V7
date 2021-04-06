#ifndef myEXTENED_H
#define myEXTENED_H



extern volatile unsigned long long FreeRTOSRunTimeTicks;

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() configTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE()		 FreeRTOSRunTimeTicks


//configGENERATE_RUN_TIME_STATS为1后必须实现的函数
void configTimerForRunTimeStats(void);




#endif
