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


//LITTLEVGL
#include "lvgl.h"
#include "lv_port_disp.h"		//显示驱动
#include "lv_port_indev.h"		//触摸驱动
#include "label_test/label_test.h"


int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  			//初始化延时函数
	uart_init(115200);			//初始化串口波特率为115200
	TIM3_Init(1000-1,84-1);		//定时一个毫秒
	LED_Init();					//初始化LED 
	
 	LCD_Init(1);
	
	KEY_Init(); 				//按键初始化  
	tp_dev.init();				//触摸屏初始化
	
	FSMC_SRAM_Init();
	
	
	
	
	lv_init();					//lvgl系统初始化
	lv_port_disp_init();		//lvgl显示接口初始化,放在lv_init()的后面
	lv_port_indev_init();		//lvgl输入接口初始化,放在lv_init()的后面
	

	
	label_test_start();
	
	while(1)
	{
		tp_dev.scan(0);
		lv_task_handler();
	}
}
