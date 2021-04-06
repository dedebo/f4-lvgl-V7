/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "sys.h"
#include "usart.h"
#include "myEXTEND.h"

#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
    #include <stdint.h>
    extern uint32_t SystemCoreClock;
#endif

//æ–­è¨€
#define vAssertCalled(char,int) printf("Error:%s,%d\r\n",char,int)
#define configASSERT(x) if((x)==0) vAssertCalled(__FILE__,__LINE__)

/***************************************************************************************************************/
/*                                        FreeRTOS»ù´¡ÅäÖÃÅäÖÃÑ¡Ïî                                              */
/***************************************************************************************************************/
#define configUSE_PREEMPTION					1                       //1Ê¹ÓÃÇÀÕ¼Ê½ÄÚºË£¬0Ê¹ÓÃĞ­³Ì
#define configUSE_TIME_SLICING					1						//1Ê¹ÄÜÊ±¼äÆ¬µ÷¶È(Ä¬ÈÏÊ½Ê¹ÄÜµÄ)
#define configUSE_PORT_OPTIMISED_TASK_SELECTION	1                       //1ÆôÓÃÌØÊâ·½·¨À´Ñ¡ÔñÏÂÒ»¸öÒªÔËĞĞµÄÈÎÎñ
                                                                        //Ò»°ãÊÇÓ²¼ş¼ÆËãÇ°µ¼ÁãÖ¸Áî£¬Èç¹ûËùÊ¹ÓÃµÄ
                                                                        //MCUÃ»ÓĞÕâĞ©Ó²¼şÖ¸ÁîµÄ»°´ËºêÓ¦¸ÃÉèÖÃÎª0£¡
#define configUSE_TICKLESS_IDLE					0                       //1ÆôÓÃµÍ¹¦ºÄticklessÄ£Ê½
#define configUSE_QUEUE_SETS					1                       //Îª1Ê±ÆôÓÃ¶ÓÁĞ
#define configCPU_CLOCK_HZ						(SystemCoreClock)       //CPUÆµÂÊ
#define configTICK_RATE_HZ						(1000)                  //Ê±ÖÓ½ÚÅÄÆµÂÊ£¬ÕâÀïÉèÖÃÎª1000£¬ÖÜÆÚ¾ÍÊÇ1ms
#define configMAX_PRIORITIES					(32)                    //¿ÉÊ¹ÓÃµÄ×î´óÓÅÏÈ¼¶(ÓÖ32¸öÓÅÏÈ¼¶£¬Êı×ÖÔ½´óÓÅÏÈ¼¶Ô½¸ß)
#define configMINIMAL_STACK_SIZE				((unsigned short)130)   //¿ÕÏĞÈÎÎñÊ¹ÓÃµÄ¶ÑÕ»´óĞ¡
#define configMAX_TASK_NAME_LEN					(20)                    //ÈÎÎñÃû×Ö×Ö·û´®³¤¶È

#define configUSE_16_BIT_TICKS					0                       //ÏµÍ³½ÚÅÄ¼ÆÊıÆ÷±äÁ¿Êı¾İÀàĞÍ£¬
                                                                        //1±íÊ¾Îª16Î»ÎŞ·ûºÅÕûĞÎ£¬0±íÊ¾Îª32Î»ÎŞ·ûºÅÕûĞÎ
#define configIDLE_SHOULD_YIELD					1                       //Îª1Ê±¿ÕÏĞÈÎÎñ·ÅÆúCPUÊ¹ÓÃÈ¨¸øÆäËûÍ¬ÓÅÏÈ¼¶µÄÓÃ»§ÈÎÎñ
#define configUSE_TASK_NOTIFICATIONS            1                       //Îª1Ê±¿ªÆôÈÎÎñÍ¨Öª¹¦ÄÜ£¬Ä¬ÈÏ¿ªÆô
#define configUSE_MUTEXES						1                       //Îª1Ê±Ê¹ÓÃ»¥³âĞÅºÅÁ¿
#define configQUEUE_REGISTRY_SIZE				8                       //²»Îª0Ê±±íÊ¾ÆôÓÃ¶ÓÁĞ¼ÇÂ¼£¬¾ßÌåµÄÖµÊÇ¿ÉÒÔ
                                                                        //¼ÇÂ¼µÄ¶ÓÁĞºÍĞÅºÅÁ¿×î´óÊıÄ¿¡£
#define configCHECK_FOR_STACK_OVERFLOW			0                       //´óÓÚ0Ê±ÆôÓÃ¶ÑÕ»Òç³ö¼ì²â¹¦ÄÜ£¬Èç¹ûÊ¹ÓÃ´Ë¹¦ÄÜ
                                                                        //ÓÃ»§±ØĞëÌá¹©Ò»¸öÕ»Òç³ö¹³×Óº¯Êı£¬Èç¹ûÊ¹ÓÃµÄ»°
                                                                        //´ËÖµ¿ÉÒÔÎª1»òÕß2£¬ÒòÎªÓĞÁ½ÖÖÕ»Òç³ö¼ì²â·½·¨¡£
#define configUSE_RECURSIVE_MUTEXES				1                       //Îª1Ê±Ê¹ÓÃµİ¹é»¥³âĞÅºÅÁ¿
#define configUSE_MALLOC_FAILED_HOOK			0                       //1Ê¹ÓÃÄÚ´æÉêÇëÊ§°Ü¹³×Óº¯Êı
#define configUSE_APPLICATION_TASK_TAG			0                       
#define configUSE_COUNTING_SEMAPHORES			1                       //Îª1Ê±Ê¹ÓÃ¼ÆÊıĞÅºÅÁ¿

/***************************************************************************************************************/
/*                                FreeRTOSÓëÄÚ´æÉêÇëÓĞ¹ØÅäÖÃÑ¡Ïî                                                */
/***************************************************************************************************************/
#define configSUPPORT_DYNAMIC_ALLOCATION        1                       //ä½¿ç”¨åŠ¨æ€å†…å­˜åˆ†é…
#define configSUPPORT_STATIC_ALLOCATION 		0						//é™æ€å†…å­˜åˆ†é…ï¼ˆè‡ªå·±åˆ†é…ï¼‰
#define configTOTAL_HEAP_SIZE					((size_t)(60*1024))     //å †çš„å¤§å°

/***************************************************************************************************************/
/*                                FreeRTOSÓë¹³×Óº¯ÊıÓĞ¹ØµÄÅäÖÃÑ¡Ïî                                              */
/***************************************************************************************************************/
#define configUSE_IDLE_HOOK						0                       //ç©ºé—²ä»»åŠ¡é’©å­å‡½æ•°
#define configUSE_TICK_HOOK						0                       //

/***************************************************************************************************************/
/*                                FreeRTOSÓëÔËĞĞÊ±¼äºÍÈÎÎñ×´Ì¬ÊÕ¼¯ÓĞ¹ØµÄÅäÖÃÑ¡Ïî                                 */
/***************************************************************************************************************/
#define configGENERATE_RUN_TIME_STATS	        1                       //Îª1Ê±ÆôÓÃÔËĞĞÊ±¼äÍ³¼Æ¹¦ÄÜ
#define configUSE_TRACE_FACILITY				1                       //Îª1ÆôÓÃ¿ÉÊÓ»¯¸ú×Ùµ÷ÊÔ
#define configUSE_STATS_FORMATTING_FUNCTIONS	1                       //ÓëºêconfigUSE_TRACE_FACILITYÍ¬Ê±Îª1Ê±»á±àÒëÏÂÃæ3¸öº¯Êı
                                                                        //prvWriteNameToBuffer(),vTaskList(),
                                                                        //vTaskGetRunTimeStats()
                                                                        
/***************************************************************************************************************/
/*                                FreeRTOSÓëĞ­³ÌÓĞ¹ØµÄÅäÖÃÑ¡Ïî                                                  */
/***************************************************************************************************************/
#define configUSE_CO_ROUTINES 			        0                       //Îª1Ê±ÆôÓÃĞ­³Ì£¬ÆôÓÃĞ­³ÌÒÔºó±ØĞëÌí¼ÓÎÄ¼şcroutine.c
#define configMAX_CO_ROUTINE_PRIORITIES         ( 2 )                   //Ğ­³ÌµÄÓĞĞ§ÓÅÏÈ¼¶ÊıÄ¿

/***************************************************************************************************************/
/*                                FreeRTOSÓëÈí¼ş¶¨Ê±Æ÷ÓĞ¹ØµÄÅäÖÃÑ¡Ïî                                            */
/***************************************************************************************************************/
#define configUSE_TIMERS				        1                               //Îª1Ê±ÆôÓÃÈí¼ş¶¨Ê±Æ÷
#define configTIMER_TASK_PRIORITY		        (configMAX_PRIORITIES-1)        //Èí¼ş¶¨Ê±Æ÷ÓÅÏÈ¼¶
#define configTIMER_QUEUE_LENGTH		        5                               //Èí¼ş¶¨Ê±Æ÷¶ÓÁĞ³¤¶È
#define configTIMER_TASK_STACK_DEPTH	        (configMINIMAL_STACK_SIZE*2)    //Èí¼ş¶¨Ê±Æ÷ÈÎÎñ¶ÑÕ»´óĞ¡

/***************************************************************************************************************/
/*                                FreeRTOS¿ÉÑ¡º¯ÊıÅäÖÃÑ¡Ïî                                                      */
/***************************************************************************************************************/
#define INCLUDE_xTaskGetSchedulerState          1                       
#define INCLUDE_vTaskPrioritySet		        1
#define INCLUDE_uxTaskPriorityGet		        1
#define INCLUDE_vTaskDelete				        1
#define INCLUDE_vTaskCleanUpResources	        1
#define INCLUDE_vTaskSuspend			        1
#define INCLUDE_vTaskDelayUntil			        1
#define INCLUDE_vTaskDelay				        1
#define INCLUDE_eTaskGetState			        1
#define INCLUDE_xTimerPendFunctionCall	        1

/***************************************************************************************************************/
/*                                FreeRTOSÓëÖĞ¶ÏÓĞ¹ØµÄÅäÖÃÑ¡Ïî                                                  */
/***************************************************************************************************************/
#ifdef __NVIC_PRIO_BITS
	#define configPRIO_BITS       		__NVIC_PRIO_BITS
#else
	#define configPRIO_BITS       		4                  
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY			15                      //
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	5                       //
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/***************************************************************************************************************/
/*                                FreeRTOSÓëÖĞ¶Ï·şÎñº¯ÊıÓĞ¹ØµÄÅäÖÃÑ¡Ïî                                          */
/***************************************************************************************************************/
#define xPortPendSVHandler 	PendSV_Handler
#define vPortSVCHandler 	SVC_Handler

#endif /* FREERTOS_CONFIG_H */

