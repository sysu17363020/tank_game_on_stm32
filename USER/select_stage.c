#include "sys.h"
#include "delay.h"  
#include "usart.h" 
#include "lcd.h"
#include "touch.h"
#include "key.h"
#include "malloc.h" 
#include "sdio_sdcard.h"
#include "guix.h"
#include "text.h"
#include "edit_map.h"
#include "data_addr.h"
#include "select_stage.h"

//u16 colorlist[15]={BLACK,BLUE,BROWN,WHITE,RED,YELLOW,GREEN,BRED,GBLUE,MAGENTA,CYAN,BRRED,GRED,GRAY,DGRAY};
///////////////////// 0     1     2     3    4    5      6    7     8     9      10    11   12   13   14
extern u16 print_my_home[8][8];

myframe frame_delet={30,80,180,80};

mybutton confirm_select={263,192,32,16};
mybutton delet_stage={263,219,32,16};

mybutton confirm_delet={70,135,32,16};
mybutton quit_delet={138,135,32,16};

void load_select_ui(u8 mapnum){
	int i,x,y;
	POINT_COLOR = BLUE;
	x=258;
	y=8;
	for(i=1;i<=mapnum;i++){
		Show_Str(x,y,48,16,"第 关",16,1);
		LCD_ShowxNum(x+16,y,i,1,16,1);
		y+=20;
	}
}


//删除关卡并恢复画面
//返回值：0，删除失败；1，成功删除
int delet(u8* pmapnum,u8 tg){
	u8 *buf1,*buf2;
	int i,j,flag=1;	//用一个flag标记删除是否成功
	buf1=mymalloc(0,512*8);
	POINT_COLOR=YELLOW;
	Show_Str(confirm_delet.x,confirm_delet.y,32,16,"确认",16,1);
	//读写SD卡，使所删关卡后的所有关卡全部往前移1个关卡的空间
	for(i=0;i<*pmapnum-tg;i++){
		if(SD_ReadDisk(buf1,MapAddr+(tg+i)*8,8)) {flag=0; break; } //读SD卡出错
		if(SD_WriteDisk(buf1,MapAddr+(tg+i-1)*8,8)) {flag=0; break; } //写SD卡出错
	}
	POINT_COLOR=RED;
	if(flag){	//成功删除，更改总关卡数
						buf2=mymalloc(0,1);
						(*pmapnum)--;
						*buf2=*pmapnum;
						i=10;
						while(i--) {
							if(SD_WriteDisk(buf2,MapNumAddr,1)) flag=0;
							else {flag=1; break;}
						}
						if(flag) Show_Str(frame_delet.x+50,frame_delet.y+35,80,16,"成功删除！",16,1);
						myfree(0,buf2);
					}
	else Show_Str(frame_delet.x+50,frame_delet.y+35,80,16,"删除异常！",16,1);//删除失败，弹出提示信息
	delay_ms(700);
	//更新侧边栏				
	LCD_Fill(240,0,319,239,GRAY);
	load_select_ui(*pmapnum);
	POINT_COLOR=YELLOW;
	if(tg>*pmapnum) tg=*pmapnum;
	Show_Str(258,8+20*(tg-1),48,16,"第 关",16,1);
	LCD_ShowxNum(258+16,8+20*(tg-1),tg,1,16,1);
	POINT_COLOR=RED;
	Show_Str(delet_stage.x,delet_stage.y,delet_stage.width,delet_stage.height,"删除",16,1);
	Show_Str(confirm_select.x,confirm_select.y,confirm_select.width,confirm_select.height,"确认",16,1);
	//恢复画面
	if(SD_ReadDisk(buf1,MapAddr+(tg-1)*8,8)) Show_Str(65,105,128,16,"读取SD卡失败!",16,1);
	for(i=0;i<60;i++){
		for(j=0;j<60;j++){
			if(i<26||i>33||j<51||j>58)
				LCD_Fill(i*4,j*4,i*4+3,j*4+3,colorlist[*(buf1+j*60+i)]);
		}
	}
	for(i=26;i<=33;i++){	//基地
		for(j=51;j<=58;j++){
			LCD_Fill(i*4,j*4,i*4+3,j*4+3,print_my_home[j-51][i-26]);
		}
	}
	myfree(0,buf1);
	if(flag) return 1;
	else return 0;	
}

//删除对话框函数
//返回值：1，成功删除；0，未删除
int predelet(u8* pmapnum,u8 tg){
	u8 *buf1;
	int i,j;
	u8 key;
	buf1=mymalloc(0,512*8);
	LCD_Fill(frame_delet.x,frame_delet.y,frame_delet.x+frame_delet.width,frame_delet.y+frame_delet.height,GRAY);
	POINT_COLOR=BLACK;
	Show_Str(frame_delet.x+35,frame_delet.y+16,128,16,"确认删除第 关？",16,1);
	LCD_ShowxNum(frame_delet.x+35+16*5,frame_delet.y+16,tg,1,16,1);
	POINT_COLOR=RED;
	Show_Str(confirm_delet.x,confirm_delet.y,32,16,"确认",16,1);
	Show_Str(quit_delet.x,quit_delet.y,32,16,"取消",16,1);
	while(1){
		tp_dev.scan(0);
		key=KEY_Scan(0);
		if(key==5) {	myfree(0,buf1);	return delet(pmapnum,tg);}//确认删除，执行delet，返回delet的返回值
		else if(key==6)	//取消删除，恢复画面后返回0
		{
			if(SD_ReadDisk(buf1,MapAddr+(tg-1)*8,8)) Show_Str(65,105,128,16,"读取SD卡失败!",16,1);
			for(i=0;i<60;i++){
				for(j=0;j<60;j++){
					if(i<26||i>33||j<51||j>58)
					LCD_Fill(i*4,j*4,i*4+3,j*4+3,colorlist[*(buf1+j*60+i)]);
				}
			}
					for(i=26;i<=33;i++){	//家
							for(j=51;j<=58;j++){
								LCD_Fill(i*4,j*4,i*4+3,j*4+3,print_my_home[j-51][i-26]);
							}
					}
			myfree(0,buf1);
			return 0;
		}
		if(tp_dev.sta&TP_PRES_DOWN){
			if(tp_dev.x[0]> confirm_delet.x-2 && tp_dev.x[0]< confirm_delet.x + confirm_delet.width+2 
			&& tp_dev.y[0]> confirm_delet.y-2 && tp_dev.y[0]< confirm_delet.y + confirm_delet.height+2)
			{//确认删除，执行delet，返回delet的返回值
					myfree(0,buf1);
				  return delet(pmapnum,tg);
			}
			else if(tp_dev.x[0]> quit_delet.x-2 && tp_dev.x[0]< quit_delet.x + quit_delet.width+2 
			&& tp_dev.y[0]> quit_delet.y-2 && tp_dev.y[0]< quit_delet.y + quit_delet.height+2)
			{//取消删除，恢复画面后返回0
				if(SD_ReadDisk(buf1,MapAddr+(tg-1)*8,8)) Show_Str(65,105,128,16,"读取SD卡失败!",16,1);
					for(i=0;i<60;i++){
						for(j=0;j<60;j++){
							if(i<26||i>33||j<51||j>58)
							LCD_Fill(i*4,j*4,i*4+3,j*4+3,colorlist[*(buf1+j*60+i)]);
						}
					}
					for(i=26;i<=33;i++){	//家
							for(j=51;j<=58;j++){
								LCD_Fill(i*4,j*4,i*4+3,j*4+3,print_my_home[j-51][i-26]);
							}
					}
				myfree(0,buf1);
				return 0;
			}
		}
	}
}

//选择关卡功能主函数
//返回所选关数
int select(void){
	u8 selected = 1; //当前选定的关卡
	int i,j,k,flag,x=258,y=8,key;
	u8* pmapnum;
	pmapnum = mymalloc(0,512);
	u8* pixel_matrix;
	pixel_matrix = mymalloc(0,512*8);
	LCD_Clear(BLACK);
	POINT_COLOR=WHITE;
	if(SD_ReadDisk(pmapnum,MapNumAddr,1)) Show_Str(65,105,128,16,"读取SD卡失败!",16,1);
	if(*pmapnum==0){	//当前没有关卡，显示提示信息
		LCD_Clear(BLACK);
		Show_Str(75,105,200,16,"0.0当前没有地图？？？",16,1); 
		Show_Str(100,125,120,16,"请先创建地图！",16,1);
		delay_ms(2000);
		return 0;
	}
	LCD_Fill(240,0,319,239,GRAY);
	load_select_ui(*pmapnum);	//加载侧边栏
	if(SD_ReadDisk(pixel_matrix,MapAddr,8)) Show_Str(65,105,128,16,"读取SD卡失败!",16,1);
	for(k=0;k<60;k++){		//显示第一关地图
			for(j=0;j<60;j++){
				if(k<26||k>33||j<51||j>58)
				LCD_Fill(k*4,j*4,k*4+3,j*4+3,colorlist[*(pixel_matrix+j*60+k)]);
			}
	}
	for(i=26;i<=33;i++){	//渲染基地
			for(j=51;j<=58;j++){
					LCD_Fill(i*4,j*4,i*4+3,j*4+3,print_my_home[j-51][i-26]);
				}
	}
	//缺省选择第一关
	POINT_COLOR=YELLOW;
	Show_Str(x,y,64,16,"第1关",16,1);
	POINT_COLOR=RED;
	Show_Str(delet_stage.x,delet_stage.y,delet_stage.width,delet_stage.height,"删除",16,1);
	Show_Str(confirm_select.x,confirm_select.y,confirm_select.width,confirm_select.height,"确认",16,1);
	delay_ms(200);
	while(1){
		if(*pmapnum==0){ 	//若删除关卡后关卡总数为0，显示提示信息
			LCD_Clear(BLACK);
			POINT_COLOR=WHITE;
			Show_Str(75,105,200,16,"0.0当前没有地图？？？",16,1); 
			Show_Str(100,125,120,16,"请先创建地图！",16,1);
			delay_ms(2000);
			return 0;
		}
		tp_dev.scan(0);
		if(tp_dev.sta&TP_PRES_DOWN){		//触摸屏被按下
			flag=0;
			y=8;
			for(i=1;i<=*pmapnum;i++){	//扫描是否触控选择某一关
				if(tp_dev.x[0]>= x && tp_dev.x[0]<= x+48 
				&& tp_dev.y[0]>= y-2 && tp_dev.y[0]<= y+18){
					flag = 1;
					selected=i;
					if(SD_ReadDisk(pixel_matrix,MapAddr+(selected-1)*8,8)) Show_Str(65,105,128,16,"读取SD卡失败!",16,1);
					for(k=0;k<60;k++){
						for(j=0;j<60;j++){	//显示所选关卡的地图
							if(k<26||k>33||j<51||j>58)
								LCD_Fill(k*4,j*4,k*4+3,j*4+3,colorlist[*(pixel_matrix+j*60+k)]);
						}
					}
					for(int s=26;s<=33;s++){	//渲染基地
						for(int t=51;t<=58;t++){
							LCD_Fill(s*4,t*4,s*4+3,t*4+3,print_my_home[t-51][s-26]);
						}
					}
					//更新侧边栏
					load_select_ui(*pmapnum);	
					POINT_COLOR=YELLOW;
					Show_Str(x,y,48,16,"第 关",16,1);
					LCD_ShowxNum(x+16,y,i,1,16,1);
					break;
				}
				y+=20;
			}
			if(flag) continue;
			if(tp_dev.x[0]>= delet_stage.x-2 && tp_dev.x[0]<= delet_stage.x + delet_stage.width+1 
			&& tp_dev.y[0]>= delet_stage.y-2 && tp_dev.y[0]<= delet_stage.y + delet_stage.height+1)
			{		//删除关卡
					if(predelet(pmapnum,selected) && selected==*pmapnum+1)//若删除了关卡且删除的是最后一关
						selected--; 
			}
			if(tp_dev.x[0]>= confirm_select.x-2 && tp_dev.x[0]<= confirm_select.x + confirm_select.width+1 
			&& tp_dev.y[0]>= confirm_select.y-2 && tp_dev.y[0]<= confirm_select.y + confirm_select.height+1)
			{		//释放分配的内存，返回所选择关数
					myfree(0,pixel_matrix);
					myfree(0,pmapnum);
					return selected;
			}
			delay_ms(50);
		}
		y=8;
		key=KEY_Scan(0);		//扫描按键
		if(key==1){	//按了“上”键，所选择关数减一
			POINT_COLOR=BLUE;
			Show_Str(x,y+(selected-1)*20,48,16,"第 关",16,1);
			LCD_ShowxNum(x+16,y+(selected-1)*20,selected,1,16,1);
			selected=(--selected ? selected : *pmapnum);
			POINT_COLOR=YELLOW;
			Show_Str(x,y+(selected-1)*20,48,16,"第 关",16,1);
			LCD_ShowxNum(x+16,y+(selected-1)*20,selected,1,16,1);
			if(SD_ReadDisk(pixel_matrix,MapAddr+(selected-1)*8,8)) Show_Str(65,105,128,16,"读取SD卡失败!",16,1);
					for(k=0;k<60;k++){
						for(j=0;j<60;j++){
							if(k<26||k>33||j<51||j>58)
								LCD_Fill(k*4,j*4,k*4+3,j*4+3,colorlist[*(pixel_matrix+j*60+k)]);
						}
					}
					for(i=26;i<=33;i++){	//基地
							for(j=51;j<=58;j++){
								LCD_Fill(i*4,j*4,i*4+3,j*4+3,print_my_home[j-51][i-26]);
							}
					}
		}
		else if(key==2){	//按了“下”键，所选择关数加一
			POINT_COLOR=BLUE;
			Show_Str(x,y+(selected-1)*20,48,16,"第 关",16,1);
			LCD_ShowxNum(x+16,y+(selected-1)*20,selected,1,16,1);
			selected=(++selected>(*pmapnum) ? 1 : selected);
			POINT_COLOR=YELLOW;
			Show_Str(x,y+(selected-1)*20,48,16,"第 关",16,1);
			LCD_ShowxNum(x+16,y+(selected-1)*20,selected,1,16,1);
			if(SD_ReadDisk(pixel_matrix,MapAddr+(selected-1)*8,8)) Show_Str(65,105,128,16,"读取SD卡失败!",16,1);
					for(k=0;k<60;k++){
						for(j=0;j<60;j++){
							if(k<26||k>33||j<51||j>58)
							LCD_Fill(k*4,j*4,k*4+3,j*4+3,colorlist[*(pixel_matrix+j*60+k)]);
						}
					}
					for(i=26;i<=33;i++){	//基地
							for(j=51;j<=58;j++){
								LCD_Fill(i*4,j*4,i*4+3,j*4+3,print_my_home[j-51][i-26]);
							}
					}
		}
		else if(key==5){	//按了“确定”键，释放分配的内存，返回所选择关数
			myfree(0,pixel_matrix);
			myfree(0,pmapnum);
			return selected;
		}
		else if(key==6){	//按了“取消”键，删除
			if(predelet(pmapnum,selected) && selected==*pmapnum+1)	selected--; //删除
		}
	}
}
