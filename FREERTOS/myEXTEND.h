#ifndef myEXTENED_H
#define myEXTENED_H



extern volatile unsigned long long FreeRTOSRunTimeTicks;

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() configTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE()		 FreeRTOSRunTimeTicks


//configGENERATE_RUN_TIME_STATSΪ1�����ʵ�ֵĺ���
void configTimerForRunTimeStats(void);




#endif
