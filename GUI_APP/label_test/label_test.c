#include "label_test.h"


void label_test_start(void)
{
	static lv_obj_t* label1;
	static lv_obj_t* label2;
	
	
	
	label1 = lv_label_create(lv_scr_act(), NULL);
	label2 = lv_label_create(lv_scr_act(), NULL);
	
	lv_obj_set_pos(label1,0,0);
	lv_label_set_long_mode(label1,LV_LABEL_LONG_SROLL_CIRC);
	lv_label_set_recolor(label1,true);
	lv_obj_set_width(label1,50);
	lv_label_set_text(label1,"#ff0000 color# #00ff00 color# #0000ff color#");
	
	lv_obj_set_pos(label2,0,100);
	lv_label_set_long_mode(label2,LV_LABEL_LONG_BREAK);
	lv_label_set_recolor(label2,true);
	lv_obj_set_width(label1,100);
	lv_label_set_text(label2,"I am linxingyu,i am a good man");
}

