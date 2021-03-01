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
#include "lv_port_disp.h"		//��ʾ����
#include "lv_port_indev.h"		//��������
#include "label_test/label_test.h"


int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  			//��ʼ����ʱ����
	uart_init(115200);			//��ʼ�����ڲ�����Ϊ115200
	TIM3_Init(1000-1,84-1);		//��ʱһ������
	LED_Init();					//��ʼ��LED 
	
 	LCD_Init(1);
	
	KEY_Init(); 				//������ʼ��  
	tp_dev.init();				//��������ʼ��
	
	FSMC_SRAM_Init();
	
	
	
	
	lv_init();					//lvglϵͳ��ʼ��
	lv_port_disp_init();		//lvgl��ʾ�ӿڳ�ʼ��,����lv_init()�ĺ���
	lv_port_indev_init();		//lvgl����ӿڳ�ʼ��,����lv_init()�ĺ���
	

	
	label_test_start();
	
	while(1)
	{
		tp_dev.scan(0);
		lv_task_handler();
	}
}
