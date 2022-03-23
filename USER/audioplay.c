#include "includes.h"		
#include "sys.h"
#include "audioplay.h"
#include "settings.h"
#include "ucos_ii.h"
#include "wm8978.h"
#include "i2s.h"
#include "wavplay.h" 
#include "mp3play.h" 

__audiodev audiodev;	


//音频播放任务
//设置任务优先级
#define AUDIO_PLAY_TASK_PRIO       			2
//设置任务堆栈大小
#define AUDIO_PLAY_STK_SIZE  					1024
//任务堆栈	
OS_STK AUDIO_PLAY_TASK_STK[AUDIO_PLAY_STK_SIZE];
//任务函数
void audioplay_task(void *pdata);

//播放音乐任务邮箱
OS_EVENT * audiombox;//事件控制块

const char * BGM1="0:/MUSIC/bgm1.wav";		//bgm1路径+文件名
const char * BGM2="0:/MUSIC/bgm2.wav";		//bgm2路径+文件名
const char * SOUND_EFFECT_1="0:/MUSIC/bgm1.wav";		//sound_effect_1路径+文件名

//开始音频播放
void audio_start(void)
{
	audiodev.status|=1<<1;		//开启播放
	audiodev.status|=1<<0;		//非暂停状态
	I2S_Play_Start();
} 
//停止音频播放
void audio_stop(void)
{
	audiodev.status&=~(1<<0);	//暂停位清零
	audiodev.status&=~(1<<1);	//结束播放
	I2S_Play_Stop();
}    

void audioplay_task(void *pdata){
	u8 * fname;
	u32 status;
	LCD_Clear(BLUE);
	WM8978_ADDA_Cfg(1,0);	//开启DAC
	WM8978_Input_Cfg(0,0,0);//关闭输入通道
	WM8978_Output_Cfg(1,0);	//开启DAC输出
 // status = (u32)OSMboxPend(audiombox,0,&rval);
	
/*	switch(status){
		case 1:{
			strcpy((char*)fname,BGM1);
			while(1) wav_play_song(fname);
		}
		case 2:{
			break;
		}*/
	while(1)
	{ 
		audiodev.status|=1<<5;//标记切歌了
		audiodev.status|=1<<4;//标记正在播放音乐 
		strcpy((char*)fname,"0:/MUSIC/sf.mp3");
		mp3_play_song(fname);
		//delay_ms(100);
	}
			
}

void audioplay_task_creat(void){
	audiodev.status=1<<7;								//允许音频播放任务运行
	OSTaskCreate(audioplay_task,(void *)0,(OS_STK*)&AUDIO_PLAY_TASK_STK[AUDIO_PLAY_STK_SIZE-1],AUDIO_PLAY_TASK_PRIO);				   
}

