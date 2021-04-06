#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "my_lcd.h"
#include "key.h"  
#include "touch.h" 
#include "touch_test.h"
#include "tim3.h"
#include "extend_sram.h"
#include "sdio_sdcard.h"


/////////////////////MALLOC//////////////////
#include "malloc.h"





/////////////FATFS///////////
#include "ff.h"
	//static FATFS fs1;
	//static FIL fp;
	//static FRESULT res;
	//static UINT bw;


//LITTLEVGL
#include "lvgl.h"
#include "lv_port_disp.h"			//显示驱动
#include "lv_port_indev.h"			//触摸屏驱动
#include "lv_port_fs.h"
#include "my_control/my_control.h"	//自定义GUI

static lv_fs_file_t f;
static lv_fs_res_t res;
static uint32_t bw;

/////////////FREERTOS/////////////////
#include "FreeRTOS.h"
#include "task.h"
#include "myEXTEND.h"
#include "semphr.h"			//使用信号量必须引入这个头文件

/////////////////////信号量////////////////////////////
SemaphoreHandle_t USART_Semaphore;


//////////////////任务/////////////////////////////////////////

//开始任务				
#define START_TASK_PRIO 31						//任务优先级
#define START_TASK_STACK_SIZE 128				//任务堆栈大小
TaskHandle_t StartTaskHandler;					//任务句柄
void start_task(void *pvParameter);				//任务函数
				


//lvgl周期性任务
#define LVGL_PERIODIC_TASK_PRIO 29
#define LVGL_PERIODIC_TASK_STACK_SIZE 512
TaskHandle_t LvglPeriodicTaskHandler;
void lvglPeriodic_task(void* pvParamter);


//任务统计
static char taskListBuf[400];
#define STATISTIC_TASK_PRIO 1
#define STATISTIC_TASK_STACK_SIZE 256
TaskHandle_t statisticTaskHandler;
void statistic_task(void* pvParameter);


//lvgl自定义GUI
#define MY_CONTROL_TASK_PRIO 15
#define MY_CONTROL_TASK_STACK_SIZE 512
TaskHandle_t myControlTaskHandler;
void my_control_task(void* pvParameter);


//lvgl USART显示任务
#define MY_LVGL_USART_TASK_PRIO 14
#define MY_LVGL_USART_TASK_STACK_SIZE 256
TaskHandle_t myLvglUsartTaskHandler;
void my_lvgl_usart_task(void* pvParameter);


int main(void)
{ 

	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//FreeRTOS只采用4
	delay_init(168);  			//延时函数初始化
	uart_init(115200);			//串口初始化
	TIM3_Init(1000-1,84-1);		//定时器3初始化，给LVGL提供心跳时钟
	LED_Init();					//LED初始化 
 	LCD_Init(1);				//LCD初始化
	KEY_Init(); 				//按键初始化  
	tp_dev.init();				//触摸屏初始化
	FSMC_SRAM_Init();			//外部RAM初始化
	mallco_dev.init(SRAMEX); 	//初始化外部内存池
	//SD开初始化
	while(SD_Init())	  
	{
		LCD_ShowString(30,150,200,16,16,(u8*)"SD Card Error!",0);
		delay_ms(500);					
		LCD_ShowString(30,150,200,16,16,(u8*)"Please Check! ",0);
		delay_ms(500);
	}
	LCD_Clear(BACK_COLOR);		//清屏
	
	
	
	lv_init();					//LVGL初始化
	lv_port_disp_init();		//lvgl显示初始化 必须放在lv_init()之后
	lv_port_indev_init();		//lvgl触摸初始化 必须放在lv_init()之后
	lv_port_fs_init();			//lvgl文件系统
	
	
	
	
	
	res = lv_fs_open(&f, "S:LVGL", LV_FS_MODE_WR);
	lv_fs_write(&f,"lxy888",6,&bw);
	lv_fs_close(&f);
	
	
	
//	f_mount(&fs1,"SD:",1);		//Register/Unregister the work area of the volume
//	
//	//test
//	res=f_open(&fp,"SD:lvl2.txt",FA_WRITE|FA_OPEN_ALWAYS);
//	f_write(&fp,"lvgl333",4,&bw);
//	f_close(&fp);
//	printf("res:%d\r\n",res);
	
	
	
	//创建开始任务
	xTaskCreate(	(TaskFunction_t)	start_task,						//任务函数入口地址
					(char * )			"start_task",					//任务名
					(uint16_t)			START_TASK_STACK_SIZE,			//任务堆栈大小
					(void *)			NULL,							//任务带的参数
					(UBaseType_t)		START_TASK_PRIO,				//任务优先级
					(TaskHandle_t *)	&StartTaskHandler				//任务句柄
					);
	
	vTaskStartScheduler();	//开启任务调度
	
	

}


//开始任务
void start_task(void *pvParameter)
{
	//进入临界区
	taskENTER_CRITICAL();
	
	//创建信号量
	USART_Semaphore = xSemaphoreCreateBinary();
	if(USART_Semaphore == NULL)
	{
		printf("fail to create Semaphore");		//不能使用中文
	}
	
	
	
	
													
	//创建任务													
	xTaskCreate(	(TaskFunction_t)	lvglPeriodic_task,				
					(char * )			"lvglPeridic_task",				
					(uint16_t)			LVGL_PERIODIC_TASK_STACK_SIZE,		
					(void *)			NULL,							
					(UBaseType_t)		LVGL_PERIODIC_TASK_PRIO,		
					(TaskHandle_t *)	&LvglPeriodicTaskHandler		
					);
					

	xTaskCreate(	(TaskFunction_t)	statistic_task,					
					(char * )			"statistic_task",				
					(uint16_t)			STATISTIC_TASK_STACK_SIZE,		
					(void *)			NULL,							
					(UBaseType_t)		STATISTIC_TASK_PRIO,			
					(TaskHandle_t *)	&statisticTaskHandler			
					);					

	xTaskCreate(	(TaskFunction_t)	my_control_task,					
					(char * )			"my_control_task",				
					(uint16_t)			MY_CONTROL_TASK_STACK_SIZE,		
					(void *)			NULL,							
					(UBaseType_t)		MY_CONTROL_TASK_PRIO,			
					(TaskHandle_t *)	&myControlTaskHandler			
					);
					
	xTaskCreate(	(TaskFunction_t)	my_lvgl_usart_task,					
					(char * )			"my_lvgl_usart_task",				
					(uint16_t)			MY_LVGL_USART_TASK_STACK_SIZE,		
					(void *)			NULL,							
					(UBaseType_t)		MY_LVGL_USART_TASK_PRIO,			
					(TaskHandle_t *)	&myLvglUsartTaskHandler			
					);

	//printf("lv_color_t:%d\r\n",sizeof(lv_color_t)); //得到结果为2
	
	//开始任务必须删除自己
	 vTaskDelete(StartTaskHandler );
	//退出临界区
	taskEXIT_CRITICAL();				
}



//LVGL周期性任务
void lvglPeriodic_task(void* pvParamter)
{
	
	while(1)
	{
		tp_dev.scan(0);			//扫描触摸屏
		lv_task_handler();		//lvgl任务处理
		vTaskDelay(30);			
	}
	
}	


//统计任务
void statistic_task(void* pvParameter)
{
	while(1)
	{
		vTaskList(taskListBuf);
		printf("%s\r\n\r\n",taskListBuf);
		vTaskDelay(2000);
	}
}

//自定义GUI任务
void my_control_task(void* pvParameter)
{
	
	taskENTER_CRITICAL();
	my_control_test();
	taskEXIT_CRITICAL();
	
	vTaskDelete(myControlTaskHandler);

}

//lvgl_usart任务
void my_lvgl_usart_task(void* pvParameter)
{
	BaseType_t err;
	
	while(1)
	{
		err = xSemaphoreTake(USART_Semaphore,portMAX_DELAY );
		if(err == pdTRUE)
		{
			
			my_set_label_text(USART_RX_BUF);	//将USART1接收缓冲区的内容传入label中
			memset(USART_RX_BUF,0,USART_REC_LEN);
			USART_RX_STA = 0;
			taskYIELD();
		}
		else
		{
			taskYIELD();
		}
	}
	
}

