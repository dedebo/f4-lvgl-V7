#ifndef _MY_LCD_H
#define _MY_LCD_H
#include "sys.h"


//LCD重要参数集
typedef struct  
{										    
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				//LCD ID
	u8  dir;			//横屏还是竖屏控制：1，竖屏；0，横屏。	
	u16	wramcmd;		//开始写gram指令
	u16  setxcmd;		//设置x坐标指令
	u16  setycmd;		//设置y坐标指令 
}_lcd_dev; 	  

//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数
//LCD的画笔颜色和背景色	   
extern u16  POINT_COLOR;//默认红色    
extern u16  BACK_COLOR; //背景颜色.默认为白色


//////////////////////////////////////////////////////////////////////////////////	 
//-----------------LCD端口定义---------------- 
#define	LCD_LED PBout(15)  		//LCD背光    		 PB15 	    
//LCD地址结构体
typedef struct
{
	vu16 LCD_REG;
	vu16 LCD_RAM;
} LCD_TypeDef;
//使用NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 A6作为数据命令区分线 
//注意设置时STM32内部会右移一位对其! 111 1110=0X7E			    
#define LCD_BASE        ((u32)(0x6C000000 | 0x0000007E))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////
	 
//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左	 

#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
//#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)
////////////////////////////////////////////////////////

void LCD_WR_REG(vu16 regval);//写寄存器
void LCD_WR_DATA(vu16 data);//写数据
u16 LCD_RD_DATA(void);//读数据
void LCD_WriteReg(u16 LCD_Reg,u16 LCD_RegValue);//写寄存器和数据
u16 LCD_ReadReg(u16 LCD_Reg);//读寄存器
void LCD_WriteRAM_Prepare(void);//准备写GRAM
void LCD_WriteRAM(u16 RGB_Code);//写GRAM

/////////////////////////////////////////////////////////
void lcddev_init(u8 lcd_dir);//lcddev内容初始化函数
void LCD_Init(u8 dir);//初始化函数， 竖屏1，横屏0
void LCD_Display_Dir(u8 dir); //设置显示方式
void LCD_Scan_Dir(u8 dir);	//设置扫描方式
void LCD_Clear(u16 color); //清屏函数
void LCD_SetCursor(u16 Xpos, u16 Ypos);//设置坐标函数
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);//局部填充
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color);				//填充指定颜色
void LCD_DrawPoint(u16 x,u16 y);//慢速画点
void LCD_Fast_DrawPoint(u16 x,u16 y,u16 color);//快速画点
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);//画线
void LCD_Draw_Circle(u16 x0,u16 y0,u8 r,u16 color);//画圆
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);//画矩形
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode);//写一个字符
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p,u8 mode);//写一串字符
u32 LCD_Pow(u8 m,u8 n);//求 m 的 n次方
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size);
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode);
void LCD_DisplayOn(void); //显示开
void LCD_DisplayOff(void);//显示关
void LCD_ShowChineseChar(u16 x,u16 y,u8 num,u8 size, u8 mode);//取模字符
void LCD_ShowChineseString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p);//取模字符串



#endif
