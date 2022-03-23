#include "includes.h"
#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "touch.h"	 	
#include "malloc.h" 
#include "sram.h"  
#include "usmart.h"  
#include "sdio_sdcard.h"    
#include "malloc.h" 
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"    
#include "fontupd.h"
#include "text.h"	
#include "piclib.h"		
#include "math.h"
#include "audioplay.h"
#include "edit_map.h" 
#include "guix.h"
#include "select_stage.h"
#include "game.h"
#include "remote.h"
#include "rng.h"


void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color);
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color); 

u16 colorlist[15]={BLACK,BLUE,BROWN,WHITE,RED,YELLOW,GREEN,BRED,GBLUE,MAGENTA,CYAN,BRRED,GRED,GRAY,DGRAY};
/////////////////// 0     1     2     3    4    5      6    7     8     9      10    11   12   13    14

u16 print_ui_tank[5][5]=	//UI标识用
		{
			{BLACK,RED,BLACK,RED,BLACK},
			{RED,RED,RED,BLACK,BLACK},
			{BLACK,RED,RED,RED,RED},
			{RED,RED,RED,BLACK,BLACK},
			{BLACK,RED,BLACK,RED,BLACK}
		};

mybutton btn_start_game={23,163,70,17};
mybutton btn_edit_map={110,163,70,17};
mybutton btn_select_stage={196,163,70,17};
mybutton btn_power_off={16,209,20,20};
mybutton btn_help={60,209,20,20};
mybutton exit_help={130,207,50,25};

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   

//主任务
//设置任务优先级
#define MAIN_TASK_PRIO       			3 
//设置任务堆栈大小
#define MAIN_STK_SIZE  					2048
//任务堆栈	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//任务函数
void main_task(void *pdata);


//////////////////////////////////////////////////////////////////////////////

void init(void);	//初始化单片机
u32 touch_scan(void);	//触控检测


//程序入口
int main(void)
{ 
  init();			//初始化
  OSInit();  	 				//初始化UCOSII
  OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	
	
}

void init(void){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	usmart_dev.init(84);		//初始化USMART
	LED_Init();					//初始化LED 
	KEY_Init(); 				//按键初始化  
 	LCD_Init();					//LCD初始化 
	tp_dev.init();				//触摸屏初始化
	Remote_Init();				//红外接收初始化
	W25QXX_Init();				//初始化W25Q128
/*	WM8978_Init();				//初始化WM8978
	WM8978_HPvol_Set(40,40);	//耳机音量设置
	WM8978_SPKvol_Set(25);		//喇叭音量设置*/
	my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM);		//初始化CCM内存池 
	exfuns_init();			//为fatfs相关变量申请内存  
	f_mount(fs[0],"0:",1); 	//挂载SD卡  
	int key;
	while(font_init()) 			//检查字库
	{   
		LCD_Clear(WHITE);		   	//清屏
 		POINT_COLOR=RED;			//设置字体为红色	   	   	  
		LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");
		while(SD_Init())			//检测SD卡
		{
			LCD_ShowString(30,70,200,16,16,"SD Card Failed!");
			delay_ms(200);
			LCD_Fill(30,70,200+30,70+16,WHITE);
			delay_ms(200);		    
		}								 						    
		LCD_ShowString(30,70,200,16,16,"SD Card OK");
		LCD_ShowString(30,90,200,16,16,"Font Updating...");
		key=update_font(20,110,16,"0:");//更新字库
		while(key)//更新失败		
		{			 		  
			LCD_ShowString(30,110,200,16,16,"Font Update Failed!");
			delay_ms(200);
			LCD_Fill(20,110,200+20,110+16,WHITE);
			delay_ms(200);		       
		} 		  
		LCD_ShowString(30,110,200,16,16,"Font Update Success!   ");  
	}
		while(RNG_Init())	 		//初始化随机数发生器
	{
		LCD_ShowString(30,130,200,16,16,"  RNG Error! ");	 
		delay_ms(200);
		LCD_ShowString(30,130,200,16,16,"RNG Trying...");	 
	}
}

//extern OS_EVENT * audiombox;//事件控制块

//开始任务
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 		  
	//audiombox=OSMboxCreate((void*) 0);	//创建音频播放任务邮箱  
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)
	//audioplay_task_creat();
  OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);	 				   	 				   
 	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}	  

//加载主界面   
void load_main_ui(void)
{
	
	LCD_Clear(BLACK);
	POINT_COLOR=RED;
	while(ai_load_picfile("0:/PICTURE/主界面.bmp",0,0,lcddev.width,lcddev.height,1)); //显示背景图片
	Show_Str(btn_start_game.x,btn_start_game.y,btn_start_game.width,btn_start_game.height,"进入游戏",16,1);
	Show_Str(btn_edit_map.x,btn_edit_map.y,btn_edit_map.width,btn_edit_map.height,"编辑地图",16,1);
	Show_Str(btn_select_stage.x,btn_select_stage.y,btn_select_stage.width,btn_select_stage.height,"选择关卡",16,1);
	POINT_COLOR=RED;
	LCD_Draw_Circle(26,219,10);	
	POINT_COLOR=WHITE;
	LCD_Draw_Circle(70,219,10);
	POINT_COLOR=RED;
	LCD_ShowChar(67,211,'?',16,1);
}	

//触控检测
u32 touch_scan(void)
{	  	
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)		//触摸屏被按下
		{	
		 	if(tp_dev.x[0]> btn_start_game.x && tp_dev.x[0]< btn_start_game.x + btn_start_game.width 
			&& tp_dev.y[0]> btn_start_game.y && tp_dev.y[0]< btn_start_game.y + btn_start_game.height)
			{			
					return 1;
			}
			else if(tp_dev.x[0]> btn_edit_map.x && tp_dev.x[0]< btn_edit_map.x + btn_edit_map.width 
				   && tp_dev.y[0]> btn_edit_map.y && tp_dev.y[0]< btn_edit_map.y + btn_edit_map.height)
			{			
					return 2;
			}
			else if(tp_dev.x[0]> btn_select_stage.x && tp_dev.x[0]< btn_select_stage.x + btn_select_stage.width 
				   && tp_dev.y[0]> btn_select_stage.y && tp_dev.y[0]< btn_select_stage.y + btn_select_stage.height)
			{			
					return 3;
			}
			else if(tp_dev.x[0]> btn_power_off.x && tp_dev.x[0]< btn_power_off.x + btn_power_off.width 
				   && tp_dev.y[0]> btn_power_off.y && tp_dev.y[0]< btn_power_off.y + btn_power_off.height)
			{			
					return 4;
			}
			else if(tp_dev.x[0]> btn_help.x && tp_dev.x[0]< btn_help.x + btn_help.width 
				   && tp_dev.y[0]> btn_help.y && tp_dev.y[0]< btn_help.y +btn_help.height)
			{			
					return 5;
			}
		}
		return 0;
}

//帮助help
void help(void){
	int key;
	while(ai_load_picfile("0:/PICTURE/帮助.bmp",0,0,lcddev.width,lcddev.height,1)); //显示帮助
	POINT_COLOR=RED;
	Show_Str(exit_help.x,exit_help.y,48,24,"确定",24,1);
	while(1){
		key=KEY_Scan(0);
		tp_dev.scan(0);
		if(key==5 || key==6) {return;}	//按了“确定”或“取消”键
		if(tp_dev.sta&TP_PRES_DOWN)		//触摸屏被按下
		{	//触摸了“确认”按钮
		 	if(tp_dev.x[0]> exit_help.x-3 && tp_dev.x[0]< exit_help.x + exit_help.width 
			&& tp_dev.y[0]> exit_help.y-3 && tp_dev.y[0]< exit_help.y + exit_help.height)
			{	//			
					return;
			}
		}
	}
}


//主任务
void main_task(void *pdata)
{	
	int flag = 1;		//当前选定的按钮
	int pre = 5;   	//前一个选定的按钮
	int tmp;
	int update=1;  	//标记是否切换了按钮
	int stage=1;    //当前选定关卡，缺省为第一关
	
	u32 key;
	u32 touch=0;
	piclib_init();	//加载图片需要初始化图片库
	load_main_ui();	//加载主界面
	while(1){
		touch=touch_scan();
		key=KEY_Scan(0);
		POINT_COLOR=BLACK;
		/*确定选择的按钮，进入相应功能模块*/
		if(touch == flag || key==5){
			switch(flag){
				case 1:{
					delay_ms(50);
					pregame(stage);	//游戏
					load_main_ui();
					update=1;
					break;
				}
				case 2:{
					edit();					//编辑地图
					load_main_ui();
					update=1;
					break;
				}
				case 3:{
					stage=select();	//选择关卡
					load_main_ui();
					update=1;
					break;
				}
				case 4:{
					PWR_EnterSTANDBYMode();	//待机
					break;
				//待机
				}
				case 5:{
					help();					//帮助
					load_main_ui();	
					update=1;
					break;
				//帮助说明
				}
			}
		}
		/*更新当前选定按钮*/
		else if(touch) {
			flag = touch;
			update = 1;
		}
		else if( key==3 ){
			flag = (--flag) ? flag : 5;
			update = 1;
		}
		else if(key == 4){
			flag = (flag++)%5 ? flag : 1 ;
			update = 1;
		}
		else if(key == 1 || key == 2){
			if(pre<4&&flag<4) {pre = flag; flag = 5; }
			else if(pre>=4&&flag>=4) {pre = flag; flag = 1;}
			else{
				tmp = pre;
				pre = flag;
				flag = tmp;
			}
			update = 1;
		}
		/*更新界面*/
		else if(update){
			switch(flag){
				case 1:{
					//LCD_Fill(btn_start_game.x - 20,btn_start_game.y-2,btn_start_game.x,btn_start_game.y+btn_start_game.height+1,WHITE);
					LCD_Fill(btn_edit_map.x - 20,btn_edit_map.y-2,btn_edit_map.x,btn_edit_map.y+btn_edit_map.height+1,BLACK);
					LCD_Fill(btn_select_stage.x - 20,btn_select_stage.y-2,btn_select_stage.x,btn_select_stage.y+btn_select_stage.height+1,BLACK);
					for(int i=0;i<5;i++){
						for(int j=0;j<5;j++){
							LCD_Fill(btn_start_game.x - 20+j*4,btn_start_game.y-2+i*4,btn_start_game.x - 20+(j*4+3),btn_start_game.y-2+(i*4+3),print_ui_tank[i][j]);
						}
					}
					POINT_COLOR=RED;
					LCD_Draw_Circle(26,219,10);	
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(70,219,10);
					POINT_COLOR=RED;
					LCD_ShowChar(67,211,'?',16,1);
					update=0;
					break;
				}
				case 2:{
					LCD_Fill(btn_start_game.x - 20,btn_start_game.y-2,btn_start_game.x,btn_start_game.y+btn_start_game.height+1,BLACK);
					//LCD_Fill(btn_edit_map.x - 20,btn_edit_map.y-2,btn_edit_map.x,btn_edit_map.y+btn_edit_map.height+1,WHITE);
					LCD_Fill(btn_select_stage.x - 20,btn_select_stage.y-2,btn_select_stage.x,btn_select_stage.y+btn_select_stage.height+1,BLACK);
					for(int i=0;i<5;i++){
						for(int j=0;j<5;j++){
							LCD_Fill(btn_edit_map.x - 20+j*4,btn_edit_map.y-2+i*4,btn_edit_map.x - 20+(j*4+3),btn_edit_map.y-2+(i*4+3),print_ui_tank[i][j]);
						}
					}
					POINT_COLOR=RED;
					LCD_Draw_Circle(26,219,10);	
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(70,219,10);
					POINT_COLOR=RED;
					LCD_ShowChar(67,211,'?',16,1);				
					update=0;
					break;
				}
				case 3:{
					LCD_Fill(btn_start_game.x - 20,btn_start_game.y-2,btn_start_game.x,btn_start_game.y+btn_start_game.height+1,BLACK);
					LCD_Fill(btn_edit_map.x - 20,btn_edit_map.y-2,btn_edit_map.x,btn_edit_map.y+btn_edit_map.height+1,BLACK);
					//LCD_Fill(btn_select_stage.x - 20,btn_select_stage.y-2,btn_select_stage.x,btn_select_stage.y+btn_select_stage.height+1,WHITE);
					for(int i=0;i<5;i++){
						for(int j=0;j<5;j++){
							LCD_Fill(btn_select_stage.x - 20+j*4,btn_select_stage.y-2+i*4,btn_select_stage.x - 20+(j*4+3),btn_select_stage.y-2+(i*4+3),print_ui_tank[i][j]);
						}
					}
					POINT_COLOR=RED;
					LCD_Draw_Circle(26,219,10);	
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(70,219,10);
					POINT_COLOR=RED;
					LCD_ShowChar(67,211,'?',16,1);
					update=0;
					break;
				//选择关卡
				}
				case 4:{
					LCD_Fill(btn_start_game.x - 20,btn_start_game.y-2,btn_start_game.x,btn_start_game.y+btn_start_game.height+1,BLACK);
					LCD_Fill(btn_edit_map.x - 20,btn_edit_map.y-2,btn_edit_map.x,btn_edit_map.y+btn_edit_map.height+1,BLACK);
					LCD_Fill(btn_select_stage.x - 20,btn_select_stage.y-2,btn_select_stage.x,btn_select_stage.y+btn_select_stage.height+1,BLACK);
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(26,219,10);	
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(70,219,10);
					POINT_COLOR=RED;
					LCD_ShowChar(67,211,'?',16,1);
					update=0;
					break;
				}
				case 5:{
					LCD_Fill(btn_start_game.x - 20,btn_start_game.y-2,btn_start_game.x,btn_start_game.y+btn_start_game.height+1,BLACK);
					LCD_Fill(btn_edit_map.x - 20,btn_edit_map.y-2,btn_edit_map.x,btn_edit_map.y+btn_edit_map.height+1,BLACK);
					LCD_Fill(btn_select_stage.x - 20,btn_select_stage.y-2,btn_select_stage.x,btn_select_stage.y+btn_select_stage.height+1,BLACK);
					POINT_COLOR=RED;
					LCD_Draw_Circle(26,219,10);	
					POINT_COLOR=RED;
					LCD_Draw_Circle(70,219,10);
					POINT_COLOR=WHITE;
					LCD_ShowChar(67,211,'?',16,1);
					update=0;
					break;
				}
			}
			
		}
		delay_ms(10);
	}
}




