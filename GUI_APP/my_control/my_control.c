#include "lvgl.h"
#include "my_control.h"
#include "usart.h"
#include "led.h"



//图标图像声明
LV_IMG_DECLARE(LED_ICON);

//函数声明
static void switch_event_cb_handler(lv_obj_t* switchx, lv_event_t event);
static void led_img_event_cb_handler(lv_obj_t* obj, lv_event_t event);

//图标
static lv_obj_t* led_img;		//led图标


//style
static lv_style_t samlpe_style;
//开关
static lv_obj_t* switch0;				//开关1
static lv_obj_t* switch1;				//开关2
//led
static lv_obj_t* led0;					//开关1对应的LED灯
static lv_obj_t* led1;					//开关2对应的LED灯

//label
static lv_obj_t* label_par;
static lv_obj_t* label;

//messagebox
static lv_obj_t* messagebox;

//图标设置
static void img_set(void)
{
	led_img = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(led_img, &LED_ICON);
	lv_obj_align_origo(led_img,NULL,LV_ALIGN_IN_TOP_LEFT,20,50);
	lv_obj_set_click(led_img,true);
	lv_obj_set_event_cb(led_img, led_img_event_cb_handler);
}


//style设置
static void style_set(void)
{
	lv_style_init(&samlpe_style);
	lv_style_set_bg_color(&samlpe_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
	lv_style_set_bg_grad_color(&samlpe_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
	lv_style_set_bg_grad_dir(&samlpe_style, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
	lv_style_set_bg_main_stop(&samlpe_style, LV_STATE_DEFAULT, 0);
	lv_style_set_bg_grad_stop(&samlpe_style, LV_STATE_DEFAULT, 255);
}


//设置label
static void label_set(void)
{
	label_par = lv_obj_create(lv_scr_act(),NULL);
	lv_obj_set_size(label_par,200,60);
	lv_obj_align(label_par,led_img,LV_ALIGN_OUT_RIGHT_MID,20,0);
	lv_obj_set_style_local_bg_color(label_par,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT, LV_COLOR_SILVER);

	label = lv_label_create(label_par,NULL);
	lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);
	
	lv_obj_align(label,NULL,LV_ALIGN_IN_LEFT_MID,0,0);
	lv_label_set_recolor(label,true);
	lv_label_set_text(label,"linxingyu 2021-3-21");
}


static void button_set(lv_obj_t* par)
{
	static lv_obj_t* label1;
	static lv_obj_t* label2;

	//设置两个开关
	switch0 = lv_switch_create(par,NULL);
	lv_obj_set_size(switch0,100,50);
	lv_obj_align_origo(switch0,NULL,LV_ALIGN_IN_RIGHT_MID,-100,-30);	
	
	switch1 = lv_switch_create(par, NULL);
	lv_obj_set_size(switch1, 100, 50);
	lv_obj_align_origo(switch1, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 30);
	
	
	
	//设置开关边上的文字
	label1 = lv_label_create(par,NULL);
	lv_label_set_text(label1,"OFF");
	lv_obj_align(label1,switch0,LV_ALIGN_OUT_LEFT_MID,-5,0);
	label1 = lv_label_create(par, NULL);
	lv_label_set_text(label1, "ON");
	lv_obj_align(label1, switch0, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	label2 = lv_label_create(par, NULL);
	lv_label_set_text(label2, "OFF");
	lv_obj_align(label2, switch1, LV_ALIGN_OUT_LEFT_MID, -5, 0);
	label2 = lv_label_create(par, NULL);
	lv_label_set_text(label2, "ON");
	lv_obj_align(label2, switch1, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	//为开关设置回调函数
	lv_obj_set_event_cb(switch0, switch_event_cb_handler);
	lv_obj_set_event_cb(switch1, switch_event_cb_handler);

	//创建两个LED灯
	led0 = lv_led_create(par,NULL);
	lv_obj_set_style_local_bg_color(led0, LV_LED_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_RED);
	lv_led_set_bright(led0,LV_LED_BRIGHT_MAX);
	lv_obj_align_origo(led0,switch0,LV_ALIGN_OUT_LEFT_MID,-100,0);
	lv_led_off(led0);
	led1 = lv_led_create(par, NULL);
	lv_obj_set_style_local_bg_color(led1, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
	lv_led_set_bright(led1, LV_LED_BRIGHT_MAX);
	lv_obj_align_origo(led1, switch1, LV_ALIGN_OUT_LEFT_MID, -100, 0);
	lv_led_off(led1);


	//更新状态
	if(LED0 == 1)
	{
		lv_switch_off(switch0,LV_ANIM_OFF);
		lv_led_off(led0);
	}
	else
	{
		lv_switch_on(switch0,LV_ANIM_OFF);
		lv_led_on(led0);
		
	}
	if(LED1 == 1)
	{
		lv_switch_off(switch1,LV_ANIM_OFF);
		lv_led_off(led1);
	}
	else
	{
		lv_switch_on(switch1,LV_ANIM_OFF);
		lv_led_on(led1);
	}
}

//messagebox设置
static void messagebox_set(lv_obj_t* par)
{
	messagebox = lv_msgbox_create(par,NULL);
	lv_obj_set_size(messagebox, LV_HOR_RES_MAX - 10, 400);
	lv_obj_align(messagebox,NULL,LV_ALIGN_IN_BOTTOM_MID,0,-20);
	lv_msgbox_set_text(messagebox,"linxingyu\n2021-3-21");
}

////////////////////////////对外部开放的函数///////////////////////////////////////////

void my_control_test(void)
{
	lv_obj_clean(lv_scr_act());	
	style_set();
	lv_obj_add_style(lv_scr_act(), LV_OBJ_PART_MAIN, &samlpe_style);
	img_set();
	messagebox_set(lv_scr_act());
	label_set();
}

void my_set_label_text(void* buf)
{
	lv_label_set_text(label,buf);
}



//开关事件回调函数
static void switch_event_cb_handler(lv_obj_t* switchx, lv_event_t event)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		if (switchx == switch0)
		{
			lv_led_toggle(led0);
			LED0 = !LED0;
		}
		else if (switchx == switch1)
		{
			lv_led_toggle(led1);
			LED1 = !LED1;
		}
	}
}

//led图标事件回调函数
static void led_img_event_cb_handler(lv_obj_t* obj, lv_event_t event)
{
	static lv_obj_t* led_win;			//led窗口
	lv_obj_t* led_close_btn;
	if (event == LV_EVENT_CLICKED)
	{
		led_win = lv_win_create(lv_scr_act(), NULL);
		lv_win_set_title(led_win, "LED");
		lv_obj_set_size(led_win,320,460);
		lv_obj_align(led_win,NULL,LV_ALIGN_IN_BOTTOM_MID,0,0);
		led_close_btn = lv_win_add_btn_right(led_win, LV_SYMBOL_CLOSE);
		lv_obj_set_event_cb(led_close_btn, lv_win_close_event_cb);

		button_set(led_win);
	}
}
