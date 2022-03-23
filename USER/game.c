#include <stdbool.h> 
#include "sys.h"
#include "delay.h"  
#include "usart.h" 
#include "lcd.h"
#include "touch.h"
#include "key.h"
#include "malloc.h" 
#include "piclib.h"	
#include "sdio_sdcard.h"
#include "guix.h"
#include "text.h"
#include "data_addr.h"
#include "game.h"
#include "remote.h"
#include "rng.h"


#define MAXLIFE 9

#define MAX_LEVEL 9  //定义关卡数
#define BULLET_NUM 50//定义屏幕中最大子弹数
#define YMAX  59//地图最下方的纵假坐标
#define TANKWI 7 //tank的宽度（假坐标）
#define HOME_WIDTH 8 //定义老家宽度
#define XMED  30//地图中心横假坐标
#define UP   1//定义好方向，为后面的direction做准备
#define DOWN 2
#define LEFT  3
#define RIGHT 4
#define WALL1 BROWN  //普通墙
#define WATER BLUE //水面
#define WALL3 DGRAY//不可破坏的墙
#define WALL4 GRAY//地图边界
#define HOME WHITE//老家（颜色）
#define COLOR0 RED    //玩家1颜色颜色
#define COLOR1 CYAN   //普通坦克颜色
#define COLOR2 PURPLE  //快速坦克颜色
#define COLOR3 GREEN  //坚固坦克1形态颜色
#define COLOR4 OLIVE   //坚固坦克2形态颜色
#define COLOR5 MAGENTA  //坚固坦克3形态颜色
#define COLOR6 YELLOW	//子弹颜色
#define COLOR7 DARKBLUE   //玩家2 颜色
#define COLOR8 DCYAN  //填充颜色


extern u16 colorlist[15];

extern u16 print_ui_tank[5][5];

u16 print_my_tank[7][7]=
		{
			{BLACK,BLACK,BLACK,COLOR0,BLACK,BLACK,BLACK},
			{BLACK,BLACK,BLACK,COLOR0,BLACK,BLACK,BLACK},
			{BLACK,BLACK,BLACK,COLOR0,BLACK,BLACK,BLACK},
   		{BLACK,COLOR0,COLOR0,COLOR0,COLOR0,COLOR0,BLACK},
			{BLACK,COLOR0,COLOR0,COLOR0,COLOR0,COLOR0,BLACK},
			{BLACK,COLOR0,COLOR8,COLOR0,COLOR8,COLOR0,BLACK},
			{BLACK,COLOR0,COLOR0,COLOR0,COLOR0,COLOR0,BLACK}

		};

u16 print_my_tank2[7][7]=
		{
			{BLACK,BLACK,BLACK,COLOR7,BLACK,BLACK,BLACK},
			{BLACK,BLACK,BLACK,COLOR7,BLACK,BLACK,BLACK},
			{BLACK,BLACK,BLACK,COLOR7,BLACK,BLACK,BLACK},
   			{BLACK,COLOR7,COLOR7,COLOR7,COLOR7,COLOR7,BLACK},
			{BLACK,COLOR7,COLOR7,COLOR7,COLOR7,COLOR7,BLACK},
			{BLACK,COLOR7,COLOR8,COLOR7,COLOR8,COLOR7,BLACK},
			{BLACK,COLOR7,COLOR7,COLOR7,COLOR7,COLOR7,BLACK}

		};

u16 print_normal_tank[7][7]=
		{
			{BLACK,BLACK,BLACK,COLOR1,BLACK,BLACK,BLACK},
			{BLACK,BLACK,BLACK,COLOR1,BLACK,BLACK,BLACK},
  			{BLACK,BLACK,COLOR1,COLOR1,COLOR1,BLACK,BLACK},
			{BLACK,COLOR1,COLOR1,COLOR1,COLOR1,COLOR1,BLACK},
  			{BLACK,COLOR1,COLOR8,COLOR1,COLOR8,COLOR1,BLACK},
			{BLACK,COLOR1,COLOR1,COLOR8,COLOR1,COLOR1,BLACK},
			{BLACK,BLACK,COLOR1,COLOR1,COLOR1,BLACK,BLACK}
			
		};

u16 print_fast_tank[7][7]=
		{
			{BLACK,BLACK,BLACK,COLOR2,BLACK,BLACK,BLACK},
			{BLACK,BLACK,BLACK,COLOR2,BLACK,BLACK,BLACK},
			{BLACK,BLACK,COLOR2,COLOR2,COLOR2,BLACK,BLACK},
			{BLACK,COLOR2,COLOR2,COLOR8,COLOR2,COLOR2,BLACK},
			{BLACK,BLACK,COLOR2,COLOR2,COLOR2,BLACK,BLACK},
			{BLACK,COLOR2,COLOR2,COLOR2,COLOR2,COLOR2,BLACK},
			{BLACK,BLACK,COLOR2,BLACK,COLOR2,BLACK,BLACK}
		};


u16 print_firm_tank1[7][7]=
		{
			{BLACK,BLACK,COLOR3,BLACK,COLOR3,BLACK,BLACK},
			{BLACK,BLACK,COLOR3,BLACK,COLOR3,BLACK,BLACK},
			{BLACK,COLOR3,COLOR3,COLOR3,COLOR3,COLOR3,BLACK},
			{COLOR3,COLOR3,COLOR3,COLOR3,COLOR3,COLOR3,COLOR3},
			{COLOR3,COLOR8,COLOR8,COLOR3,COLOR8,COLOR8,COLOR3},
			{COLOR3,COLOR3,COLOR8,COLOR3,COLOR8,COLOR3,COLOR3},
			{BLACK,COLOR3,COLOR3,COLOR3,COLOR3,COLOR3,BLACK}
		};


u16 print_firm_tank2[7][7]=
		{
			{BLACK,BLACK,COLOR4,BLACK,COLOR4,BLACK,BLACK},
			{BLACK,BLACK,COLOR4,BLACK,COLOR4,BLACK,BLACK},
			{BLACK,COLOR4,COLOR4,COLOR4,COLOR4,COLOR4,BLACK},
			{COLOR4,COLOR4,COLOR4,COLOR4,COLOR4,COLOR4,COLOR4},
			{COLOR4,COLOR8,COLOR8,COLOR4,COLOR8,COLOR8,COLOR4},
			{COLOR4,COLOR4,COLOR8,COLOR4,COLOR8,COLOR4,COLOR4},
			{BLACK,COLOR4,COLOR4,COLOR4,COLOR4,COLOR4,BLACK}

		};


u16 print_firm_tank3[7][7]=
		{
			
			{BLACK,BLACK,COLOR5,BLACK,COLOR5,BLACK,BLACK},
			{BLACK,BLACK,COLOR5,BLACK,COLOR5,BLACK,BLACK},
			{BLACK,COLOR5,COLOR5,COLOR5,COLOR5,COLOR5,BLACK},
			{COLOR5,COLOR5,COLOR5,COLOR5,COLOR5,COLOR5,COLOR5},
			{COLOR5,COLOR8,COLOR8,COLOR5,COLOR8,COLOR8,COLOR5},
			{COLOR5,COLOR5,COLOR8,COLOR5,COLOR8,COLOR5,COLOR5},
			{BLACK,COLOR5,COLOR5,COLOR5,COLOR5,COLOR5,BLACK}
		};

u16 print_my_home[8][8]=
		{
			{BLACK,BLACK,BLACK,HOME,HOME,HOME,BLACK,BLACK},
			{BLACK,BLACK,BLACK,HOME,HOME,BLACK,BLACK,BLACK},
			{HOME,HOME,HOME,BLACK,BLACK,HOME,HOME,HOME},
			{BLACK,HOME,HOME,HOME,HOME,HOME,HOME,BLACK},
			{BLACK,BLACK,HOME,HOME,HOME,HOME,BLACK,BLACK},
			{BLACK,BLACK,HOME,BLACK,BLACK,HOME,BLACK,BLACK},
			{BLACK,HOME,BLACK,HOME,HOME,BLACK,HOME,BLACK},
			{HOME,HOME,BLACK,HOME,HOME,BLACK,HOME,HOME}
		};

//全局变量
u16 map[60][60];   //地图二维数组
int bul_num=0;      //子弹编号
int position;     //位置计数,对应AI坦克生成位置,-1为左位置,0为中间,1为右,2为我的坦克位置
int speed = 7;      //游戏速度,调整用
int remain_enemy;	//剩余敌人数量
int plyernum;	//1:单人 ；2：双人
int stgindx;	//当前是第几关
u8 life1,life2;	//玩家1生命值；玩家2生命值
u32 score1,score2;	//玩家1分数；玩家2分数
u8 key1,key2=0;		//玩家1输入；玩家2输入


typedef struct             //这里的出现次序指的是一个AI_tank变量中的次序
{                          //∵设定每个AI_tank每种特殊坦克只出现一次 ∴fast_tank & firm_tank 最多出现次数不超过1
	int fast_tank_order;   //fast_tank出现的次序(在第fast_tank_order次复活出现,从第0次开始)，且每个AI_tank只出现一次
	int firm_tank_order;   //firm_tank出现的次序，同上
} LevInfo;                 //关卡出现的坦克信息
LevInfo level_info [MAX_LEVEL] = { {2,-1},{2,3},{3,2},{1,3},{3,1},{1,2},{2,1},{2,3},{3,2} };   //初始化，比如 { {-1,3} , {2,3} }  (-1代表没有该类型坦克)表示共有两关，在第二关快坦克和坚固坦克分别在第二和第三个坦克生成时出现


typedef struct      //子弹结构体
{
	int x, y;        //子弹坐标,假坐标
	int direction;  //子弹方向变量（依据坦克的方向）
	bool exist;     //子弹存在与否的变量,1为存在，0不存在
	bool initial;   //子弹是否处于建立初状态的值，1为处于建立初状态，0为处于非建立初状态
	int my;        //区分AI子弹与玩家子弹的标记,0为AI子弹，1为玩家1子弹,2为玩家2
} Bullet;
Bullet bullet[BULLET_NUM]; 


typedef struct      //坦克结构体
{
	int x, y;        //坦克中心坐标
	int direction;  //坦克方向  （后文会定义如何确定坦克方向）
	u16 color;      //颜色参方向数,具体在PrintTank函数定义有说明
	int model;      //坦克图案模型，值为1,2,3，分别代表不同的坦克图案,0为我的坦克图案，AI不能使用  //
	int stop;       //只能是AI坦克使用的参数，非0代表坦克停止走动,0为可以走动
	int revive;     //坦克复活次数
	int num;        //AI坦克编号（固定值，为常量，初始化函数中定下）0~3
	int CD;         //发射子弹冷却计时
	int my;        //是否敌方坦克参数，玩家1坦克此参数为1,玩家2坦克为2，敌军为0
	bool alive;     //存活为1，不存活为0
}  Tank;
Tank AI_tank[4], my_tank,my_tank2; 






//某些重要函数

int game(void);

//void LCD_Fast_DrawPoint(u16 x,u16 y,u16 color);

void printf_home(void); //打印家
void Initialize(void);      //初始化
int GameCheck(void);//检测游戏进度


//子弹部分
void BuildAIBullet(Tank* tank);                //AI坦克发射子弹（含有对my_tank的读取,只读取了my_tank坐标）
void BuildmyBullet(Tank*my_tank);         //一旦按下攻击键，执行此建立我的子弹函数
void BuildBullet(Tank tank);                 //子弹发射（建立）（人机共用）(含全局变量bullet的修改)我的坦克发射子弹直接调用该函数,AI通过AIshoot间接调用
int BulletFly(Bullet bullet[BULLET_NUM]); //子弹移动和打击（人机共用）,
int BulletHit(Bullet* bullet);            //子弹碰撞（人机共用）(含Tank全局变量的修改)，只通过BulletFly调用，子弹间的碰撞不在本函数,子弹间碰撞已在BulletShoot中检测并处理
void PrintBullet(int x, int y, int T);         //打印子弹（人机共用）
void ClearBullet(int x, int y, int T);         //清除子弹（人机共用）
int  BulletCheak(int x, int y);               //判断子弹前方情况（人机共用）

//坦克部分
void BuildAITank(int* position, Tank* AI_tank); //建立AI坦克
void BuildMyTank(Tank* my_tank);                //建立我的坦克
void MoveAITank(Tank* AI_tank);                //AI坦克移动
void MoveMyTank(int turn,Tank* tank);                     //我的坦克移动，只通过keyboard函数调用，既键盘控制
void ClearTank(int x, int y);                  //清除坦克（人机共用）
void PrintTank(Tank tank);                    //打印坦克（人机共用）
bool TankCheak(Tank tank, int direction);      //检测坦克dirtection方向的障碍物,返值1阻碍,0 畅通
int  AIPositionCheak(int position);           //检测AI坦克建立位置是否有障碍物

///////************************ 游戏部分的UI定义 ****************///////////////
extern mybutton btn_start_game;
extern mybutton btn_edit_map;
extern mybutton btn_select_stage;
extern mybutton btn_power_off;
extern mybutton btn_help;

extern void help(void);//帮助文档

mybutton btn_oneplayer={60,163,32,17};
mybutton btn_twoplayer={128,163,32,17};
mybutton btn_quit={196,163,32,17};

myframe frame_pause={30,110,180,60};
mybutton btn_continue={48,135,64,16};
mybutton btn_exit={128,135,64,16};

myframe nextgame={30,80,180,80};
mybutton btn_next={60,135,48,16};

myframe title_enmyleft={245,40,65,16};
myframe title_life={245,100,65,16};
myframe title_score={245,170,65,16};


//触控检测
u32 touch_scan_2(void)
{	  	
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)		//触摸屏被按下
		{	
		 	if(tp_dev.x[0]> btn_oneplayer.x && tp_dev.x[0]< btn_oneplayer.x + btn_oneplayer.width 
			&& tp_dev.y[0]> btn_oneplayer.y && tp_dev.y[0]< btn_oneplayer.y + btn_oneplayer.height)
			{			
					return 1;
			}
			else if(tp_dev.x[0]> btn_twoplayer.x && tp_dev.x[0]< btn_twoplayer.x + btn_twoplayer.width 
				   && tp_dev.y[0]> btn_twoplayer.y && tp_dev.y[0]< btn_twoplayer.y + btn_twoplayer.height)
			{			
					return 2;
			}
			else if(tp_dev.x[0]> btn_quit.x && tp_dev.x[0]< btn_quit.x + btn_quit.width 
				   && tp_dev.y[0]> btn_quit.y && tp_dev.y[0]< btn_quit.y + btn_quit.height)
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




///////////****************************************************///////////////////


//选择单人/双人，进入游戏
void pregame(int stgindex){
	int flag = 1;		//当前选定的按钮
	int pre = 5;   	//前一个选定的按钮
	int tmp;
	int update=1;  	//标记是否切换了按钮
	u32 key;
	u32 touch=0;
	/********加载界面********/
	LCD_Fill(btn_start_game.x - 20,btn_start_game.y-2,btn_select_stage.x+64,btn_select_stage.y+btn_select_stage.height+1,BLACK);
	POINT_COLOR=RED;
	Show_Str(btn_oneplayer.x,btn_oneplayer.y,btn_oneplayer.width,btn_oneplayer.height,"单人",16,1);
	Show_Str(btn_twoplayer.x,btn_twoplayer.y,btn_twoplayer.width,btn_twoplayer.height,"双人",16,1);
	Show_Str(btn_quit.x,btn_quit.y,btn_quit.width,btn_quit.height,"退出",16,1);
	LCD_Draw_Circle(26,219,10);	
	POINT_COLOR=WHITE;
	LCD_Draw_Circle(70,219,10);
	POINT_COLOR=RED;
	LCD_ShowChar(67,211,'?',16,1);
	delay_ms(100);
	/***********************/
	while(1){
		touch=touch_scan_2();						//触控扫描
		key=KEY_Scan(0);								//按键扫描
		POINT_COLOR=BLACK;
		if(key==6) return;							//退出
		/*确定选择的按钮，进入相应功能模块*/
		if(touch == flag || key==5){
			switch(flag){
				//按钮为“单人”
				case 1:{
					delay_ms(50);
					plyernum=1;								//玩家数为1
					stgindx=stgindex;					//当前关数为所选关数
					life1=MAXLIFE;						//初始化玩家1生命值
					score1=0;									//初始化玩家1分数
					while(game()) stgindx++;	//开始单人游戏
					return;										
				}
				//按钮为“双人”
				case 2:{
					delay_ms(50);
					plyernum=2;
					stgindx=stgindex;
					life1=MAXLIFE;
					score1=0;
					life2=MAXLIFE;
					score2=0;
					Remote_Start();   //开始捕捉红外信号（玩家2）
					while(game()) stgindx++;	//开始双人游戏
					Remote_Stop();		//停止捕捉红外信号					
					return;
				}
				//按钮为“退出”
				case 3:{	//退出
					return;
				}
				//按钮为“待机”
				case 4:{	//待机
					PWR_EnterSTANDBYMode();
					break;
				
				}
				//按钮为“帮助”
				case 5:{	//帮助
					help();
					while(ai_load_picfile("0:/PICTURE/主界面.bmp",0,0,lcddev.width,lcddev.height,1)); //显示背景图片
					POINT_COLOR=RED;
					Show_Str(btn_oneplayer.x,btn_oneplayer.y,btn_oneplayer.width,btn_oneplayer.height,"单人",16,1);
					Show_Str(btn_twoplayer.x,btn_twoplayer.y,btn_twoplayer.width,btn_twoplayer.height,"双人",16,1);
					Show_Str(btn_quit.x,btn_quit.y,btn_quit.width,btn_quit.height,"退出",16,1);
					LCD_Draw_Circle(26,219,10);	
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(70,219,10);
					POINT_COLOR=RED;
					LCD_ShowChar(67,211,'?',16,1);
					update=1;
					break;
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
					//LCD_Fill(btn_oneplayer.x - 20,btn_oneplayer.y-2,btn_oneplayer.x,btn_oneplayer.y+btn_oneplayer.height+1,WHITE);
					LCD_Fill(btn_twoplayer.x - 20,btn_twoplayer.y-2,btn_twoplayer.x,btn_twoplayer.y+btn_twoplayer.height+1,BLACK);
					LCD_Fill(btn_quit.x - 20,btn_quit.y-2,btn_quit.x,btn_quit.y+btn_quit.height+1,BLACK);
					for(int i=0;i<5;i++){
						for(int j=0;j<5;j++){
							LCD_Fill(btn_oneplayer.x - 20+j*4,btn_oneplayer.y-2+i*4,btn_oneplayer.x - 20+(j*4+3),btn_oneplayer.y-2+(i*4+3),print_ui_tank[i][j]);
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
					LCD_Fill(btn_oneplayer.x - 20,btn_oneplayer.y-2,btn_oneplayer.x,btn_oneplayer.y+btn_oneplayer.height+1,BLACK);
					//LCD_Fill(btn_twoplayer.x - 20,btn_twoplayer.y-2,btn_twoplayer.x,btn_twoplayer.y+btn_twoplayer.height+1,WHITE);
					LCD_Fill(btn_quit.x - 20,btn_quit.y-2,btn_quit.x,btn_quit.y+btn_quit.height+1,BLACK);
					for(int i=0;i<5;i++){
						for(int j=0;j<5;j++){
							LCD_Fill(btn_twoplayer.x - 20+j*4,btn_twoplayer.y-2+i*4,btn_twoplayer.x - 20+(j*4+3),btn_twoplayer.y-2+(i*4+3),print_ui_tank[i][j]);
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
					LCD_Fill(btn_oneplayer.x - 20,btn_oneplayer.y-2,btn_oneplayer.x,btn_oneplayer.y+btn_oneplayer.height+1,BLACK);
					LCD_Fill(btn_twoplayer.x - 20,btn_twoplayer.y-2,btn_twoplayer.x,btn_twoplayer.y+btn_twoplayer.height+1,BLACK);
					//LCD_Fill(btn_quit.x - 20,btn_quit.y-2,btn_quit.x,btn_quit.y+btn_quit.height+1,WHITE);
					for(int i=0;i<5;i++){
						for(int j=0;j<5;j++){
							LCD_Fill(btn_quit.x - 20+j*4,btn_quit.y-2+i*4,btn_quit.x - 20+(j*4+3),btn_quit.y-2+(i*4+3),print_ui_tank[i][j]);
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
					LCD_Fill(btn_oneplayer.x - 20,btn_oneplayer.y-2,btn_oneplayer.x,btn_oneplayer.y+btn_oneplayer.height+1,BLACK);
					LCD_Fill(btn_twoplayer.x - 20,btn_twoplayer.y-2,btn_twoplayer.x,btn_twoplayer.y+btn_twoplayer.height+1,BLACK);
					LCD_Fill(btn_quit.x - 20,btn_quit.y-2,btn_quit.x,btn_quit.y+btn_quit.height+1,BLACK);
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
					LCD_Fill(btn_oneplayer.x - 20,btn_oneplayer.y-2,btn_oneplayer.x,btn_oneplayer.y+btn_oneplayer.height+1,BLACK);
					LCD_Fill(btn_twoplayer.x - 20,btn_twoplayer.y-2,btn_twoplayer.x,btn_twoplayer.y+btn_twoplayer.height+1,BLACK);
					LCD_Fill(btn_quit.x - 20,btn_quit.y-2,btn_quit.x,btn_quit.y+btn_quit.height+1,BLACK);
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







//暂停游戏
//返回值：0继续游戏 1退出游戏
int pause(int plyrnum){
	u8 key1,key2=0;
	int flag=1; //当前选定按钮
	/***加载界面***/
	LCD_Fill(frame_pause.x,frame_pause.y,frame_pause.x+frame_pause.width,frame_pause.y+frame_pause.height,GRAY);
	POINT_COLOR=YELLOW;
	Show_Str(btn_continue.x,btn_continue.y,64,16,"继续游戏",16,1);
	POINT_COLOR=RED;
	Show_Str(btn_exit.x,btn_exit.y,64,16,"退出游戏",16,1);
	while(1){
		key1=KEY_Scan(0);
		if(plyrnum==2) key2=Remote_Scan();	//玩家2亦可用红外遥控暂停游戏
		//更改选定按钮
		if(key1==3 || key1==4 || key2==3 || key2==4)
		{
			if(flag==1){
				flag=2;
				POINT_COLOR=RED;
				Show_Str(btn_continue.x,btn_continue.y,64,16,"继续游戏",16,1);
				POINT_COLOR=YELLOW;
				Show_Str(btn_exit.x,btn_exit.y,64,16,"退出游戏",16,1);
			}
			else{
				flag=1;
				POINT_COLOR=YELLOW;
				Show_Str(btn_continue.x,btn_continue.y,64,16,"继续游戏",16,1);
				POINT_COLOR=RED;
				Show_Str(btn_exit.x,btn_exit.y,64,16,"退出游戏",16,1);
			}
		}
		//确认选定按钮
		else if(key1==5 || key2==5){
			delay_ms(50);
			return (flag-1);		//按选定项返回值
		}
		//按取消键直接继续游戏
		else if(key1==6 || key2==6){
			delay_ms(50);
			return 0;        //继续游戏
		}
	}
}


//通过一关或玩家都输了之后，显示相应界面，选择进入下一关或结束游戏
//返回值：1=下一关；0=退出游戏
int game_end(int win){
	u8* buf;
	POINT_COLOR=RED;
	buf=mymalloc(0,512);
	SD_ReadDisk(buf,MapNumAddr,1);
	if(win){	//赢了
		if(stgindx==*buf){	//当前关卡为最后一关，即通关
			LCD_Fill(20,65,220,215,GRAY);
			SD_ReadDisk(buf,HighScoreAddr,1);
			Show_Str(77,72,192,24,"CONGRATS",24,1);
			Show_Str(84,96,96,24,"通关啦！",24,1);
			if(plyernum==1){	//玩家数为1
				Show_Str(90,120,128,16,"得分 ",16,1);
				LCD_ShowxNum(138,120,score1,5,16,1);
				Show_Str(42,136,176,16,"历史最高分 ",16,1);
				LCD_ShowxNum(138,136,*((u32*)buf),5,16,1);
				if(score1 > *((u32*)buf))
				{
					Show_Str(84,152,96,24,"新纪录！",24,1);
					*((u32*)buf)=score1;
					SD_WriteDisk(buf,HighScoreAddr,1);
				}
			}
			if(plyernum==2){	//玩家数为2
				Show_Str(42,120,128,16,"玩家1得分 ",16,1);
				LCD_ShowxNum(138,120,score1,5,16,1);
				Show_Str(42,136,128,16,"玩家2得分 ",16,1);
				LCD_ShowxNum(138,136,score2,5,16,1);
				Show_Str(42,152,176,16,"历史最高分 ",16,1);
				LCD_ShowxNum(138,152,*((u32*)buf),5,16,1);
				if(score1>*((u32*)buf)||score2>*((u32*)buf))
				{
					Show_Str(84,168,96,24,"新纪录！",24,1);
					*((u32*)buf)=(score1>score2 ? score1 : score2);
					SD_WriteDisk(buf,HighScoreAddr,1);
				}
			}
			myfree(0,buf);
			int cnt=5;
			while(cnt){				//倒计时5秒，之后退出游戏
				LCD_ShowxNum(124,192,cnt--,1,16,0);
				delay_ms(1000);
			}
			return 0;
		}
		else{	 							//赢了，但还没通关，问要不要进入下一关						
			u8 key1,key2=0;
			int flag=1;	//1=下一关；0=退出游戏
			LCD_Fill(nextgame.x,nextgame.y,nextgame.x+nextgame.width,nextgame.y+nextgame.height,GRAY);
			Show_Str(nextgame.x+42,nextgame.y+16,96,16,"恭喜通过本关",16,1);
			POINT_COLOR=YELLOW;
			Show_Str(btn_next.x,btn_next.y,48,16,"下一关",16,1);
			POINT_COLOR=RED;
			Show_Str(btn_exit.x,btn_exit.y,48,16,"不玩了",16,1);
			while(1){
				key1=KEY_Scan(0);
				if(plyernum==2) key2=Remote_Scan();
				if(key1==3 || key1==4 || key2==3 || key2==4)
				{
					if(flag==1){
						flag=0;
						POINT_COLOR=RED;
						Show_Str(btn_next.x,btn_next.y,48,16,"下一关",16,1);
						POINT_COLOR=YELLOW;
						Show_Str(btn_exit.x,btn_exit.y,48,16,"不玩了",16,1);
					}
					else{
						flag=1;
						POINT_COLOR=YELLOW;
						Show_Str(btn_next.x,btn_next.y,48,16,"下一关",16,1);
						POINT_COLOR=RED;
						Show_Str(btn_exit.x,btn_exit.y,48,16,"不玩了",16,1);
					}
				}
				else if(key1==5 || key2==5){
					if(flag==0){
					SD_ReadDisk(buf,HighScoreAddr,1);
					LCD_Fill(20,65,220,215,GRAY);
					POINT_COLOR=RED;
					Show_Str(46,84,144,24,"欢迎再次挑战",24,1);
					if(plyernum==1){
						Show_Str(90,120,128,16,"得分 ",16,1);
						LCD_ShowxNum(138,120,score1,5,16,1);
						Show_Str(42,136,176,16,"历史最高分 ",16,1);
						LCD_ShowxNum(138,136,*((u32*)buf),5,16,1);
						if(score1>*((u32*)buf))
						{
							Show_Str(84,152,96,24,"新纪录！",24,1);
							*((u32*)buf)=score1;
							SD_WriteDisk(buf,HighScoreAddr,1);
						}
					}
					if(plyernum==2){
						Show_Str(42,120,128,16,"玩家1得分 ",16,1);
						LCD_ShowxNum(138,120,score1,5,16,1);
						Show_Str(42,136,128,16,"玩家2得分 ",16,1);
						LCD_ShowxNum(138,136,score2,5,16,1);
						Show_Str(42,152,176,16,"历史最高分 ",16,1);
						LCD_ShowxNum(138,152,*((u32*)buf),5,16,1);
						if(score1>*((u32*)buf)||score2>*((u32*)buf))
						{
							Show_Str(84,168,96,24,"新纪录！",24,1);
							*((u32*)buf)=(score1>score2 ? score1 : score2);
							SD_WriteDisk(buf,HighScoreAddr,1);
						}
					}
					int cnt=5;
					while(cnt){
							LCD_ShowxNum(124,192,cnt--,1,16,0);
							delay_ms(1000);
						}
					}
					myfree(0,buf);
					return flag;		//按选定项返回值
				}
			}
		}
	}
	else{		//输了
		SD_ReadDisk(buf,HighScoreAddr,1);
		LCD_Fill(20,65,220,215,GRAY);
		Show_Str(94,72,192,24,"FAILL",24,1);
			Show_Str(72,96,96,24,"游戏结束",24,1);
			if(plyernum==1){	//玩家数为1
				Show_Str(90,120,128,16,"得分 ",16,1);
				LCD_ShowxNum(138,120,score1,5,16,1);
				Show_Str(42,136,176,16,"历史最高分 ",16,1);
				LCD_ShowxNum(138,136,*((u32*)buf),5,16,1);
				if(score1>*((u32*)buf))
				{
					Show_Str(84,152,96,24,"新纪录！",24,1);
					*((u32*)buf)=score1;
					SD_WriteDisk(buf,HighScoreAddr,1);
				}
			}
			if(plyernum==2){	//玩家数为2
				Show_Str(42,120,128,16,"玩家1得分 ",16,1);
				LCD_ShowxNum(138,120,score1,5,16,1);
				Show_Str(42,136,128,16,"玩家2得分 ",16,1);
				LCD_ShowxNum(138,136,score2,5,16,1);
				Show_Str(42,152,176,16,"历史最高分 ",16,1);
				LCD_ShowxNum(138,152,*((u32*)buf),5,16,1);
				if(score1>*((u32*)buf)||score2>*((u32*)buf))
				{
					Show_Str(84,168,96,24,"新纪录！",24,1);
					*((u32*)buf)=(score1>score2 ? score1 : score2);
					SD_WriteDisk(buf,HighScoreAddr,1);
				}
			}
			myfree(0,buf);
			int cnt=5;
			while(cnt){			//倒计时5秒，之后退出游戏
				LCD_ShowxNum(124,192,cnt--,1,16,0);
				delay_ms(1000);
			}
			return 0;
	}
}




//	RNG_Get_RandomRange(min,max); 随机数函数 [min,max]

void load_game_slide(void){
	LCD_Fill(240,0,319,239,GRAY);//侧边栏
	POINT_COLOR=BLUE; 
	Show_Str(257,8,48,16,"第 关",16,1);
	LCD_ShowxNum(257+16,8,stgindx,1,16,1);
	POINT_COLOR=BLACK; 
	Show_Str(title_enmyleft.x,title_enmyleft.y,title_enmyleft.width,title_enmyleft.height,"剩余敌军",16,1);
	LCD_ShowxNum(title_enmyleft.x,title_enmyleft.y+16,remain_enemy,2,16,0);
	Show_Str(title_life.x,title_life.y,title_life.width,title_life.height,"生命值",16,1);
	LCD_ShowxNum(title_life.x,title_life.y+16,life1,1,16,0);
	if(plyernum==2) LCD_ShowxNum(title_life.x,title_life.y+32,life2,1,16,0);
	Show_Str(title_score.x,title_score.y,65,16,"得分",16,1);
	LCD_ShowxNum(title_score.x,title_score.y+16,score1,5,16,0);
	if(plyernum==2) LCD_ShowxNum(title_score.x,title_score.y+32,score2,5,16,0);
}

//游戏
int game(){
	u8* buf1;
	int i,j,rval;
	remain_enemy=16;	//单人模式下：第1、2关15个敌人；第3、4、5关20个敌人；第6、7、8关25个敌人；第9关30个敌人
	if(plyernum==2) remain_enemy=remain_enemy*2; //双人模式下敌人数量翻倍
	load_game_slide(); //加载侧边栏
	//加载战场
	LCD_Fill(0,0,239,239,BLACK);
	buf1=mymalloc(0,512*8);
	POINT_COLOR=RED;
	while(SD_ReadDisk(buf1,MapAddr+(stgindx-1)*8,8)) Show_Str(120,150,100,16,"地图加载失败!",16,0);
	for(i=0;i<60;i++)
		for(j=0;j<60;j++){
			map[i][j] = colorlist[*(buf1+i*60+j)];
			LCD_Fill(j*4,i*4,j*4+3,i*4+3,map[i][j]);
		}
		//myframe base={104,204,32,32};
	for(i=26;i<=33;i++){
		for(j=51;j<=58;j++){
			LCD_Fill(i*4,j*4,i*4+3,j*4+3,print_my_home[j-51][i-26]);
		}
	}
	myfree(0,buf1);
	delay_ms(100);
	u8 interval[14] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1 };  //间隔计数器数组，用于控制速度
	Initialize();                         //初始化，全局变量level初值便是1 
	while(1){
		rval = GameCheck();                //游戏胜负检测
		if(rval==0 || rval==1) return rval;
		key1=KEY_Scan(1); 
		if(plyernum==2) key2=Remote_Scan();
		if(key1==6 || key2==6) {	//暂停
			u16 screen[60][60];	//用一个二维数组存屏幕显示，以便恢复显示
			for(i=0;i<60;i++){
				for(j=0;j<60;j++){
					screen[i][j] = LCD_ReadPoint(i*4,j*4);
				}
			}
			if(key1==6){
				delay_ms(20);
				if(KEY_Scan(1)==6){ 
					if(pause(plyernum)==0)	//继续游戏
					{
						delay_ms(100);
						for(i=0;i<60;i++)
							for(j=0;j<60;j++){
								LCD_Fill(i*4,j*4,i*4+3,j*4+3,screen[i][j]);
						}
					}
					else	//退出游戏
					{
						return 0;
					}
				}
			}
			else if(pause(plyernum)==0)	//继续游戏
			{
				delay_ms(100);
				for(i=0;i<60;i++)
					for(j=0;j<60;j++){
						LCD_Fill(i*4,j*4,i*4+3,j*4+3,screen[i][j]);
					}
			}
			else	//退出游戏
			{
				return 0;
			}
		}
		else if(key1==1 || key1==2 ||key1==3 || key1==4)
		{
			MoveMyTank(key1,&my_tank);
		}
		if(key2==1 || key2==2 ||key2==3 || key2==4)
		{
			MoveMyTank(key2,&my_tank2);
		}
		rval=BulletFly(bullet);
		delay_ms(15);
		if(rval==0 || rval==1) return rval;
			for (i = 0; i <= 3; i++)         //AI坦克移动循环
			{
				if (AI_tank[i].model == 2 && interval[i]++ % 2 == 0) //四个坦克中的快速坦克单独使用计数器1,2,3,4
					MoveAITank(&AI_tank[i]);
				if (AI_tank[i].model != 2 && interval[i + 5]++ % 3 == 0) //四个坦克中的慢速坦克单独使用计数器5,6,7,8
					MoveAITank(&AI_tank[i]);
			}
			int x=(plyernum==1? 4: 8);
			for (i = 0; i <= 3; i++){//建立AI坦克部分
				if(AI_tank[i].alive == 0 && AI_tank[i].revive < x && interval[9]++ % 90 == 0)  //一个敌方坦克每局只有4条命
				{                                               //如果坦克不存活。计时,每次建立有间隔  1750 ms
					BuildAITank(&position, &AI_tank[i]);     //建立AI坦克（复活）
					break;                                      //每次循环只建立一个坦克
				}
			}
			for (i = 0; i <= 3; i++)
				if (AI_tank[i].alive)
					BuildAIBullet(&AI_tank[i]);                 //AIshoot自带int自增计数CD,不使用main中的CD interval
			if (my_tank.alive && interval[10]++ % 2 == 0)
				BuildmyBullet(&my_tank);
			if (my_tank.alive == 0 && interval[11]++ % 30 == 0 && my_tank.revive < MAXLIFE)
				BuildMyTank(&my_tank);
			if (plyernum==2 && my_tank2.alive && interval[12]++ % 2 == 0)
				BuildmyBullet(&my_tank2);
			if (plyernum==2 && my_tank2.alive == 0 && interval[13]++ % 30 == 0 && my_tank2.revive < MAXLIFE)
				BuildMyTank(&my_tank2);
	LCD_ShowxNum(title_enmyleft.x,title_enmyleft.y+16,remain_enemy,2,16,0);
	LCD_ShowxNum(title_life.x,title_life.y+16,life1,1,16,0);
	if(plyernum==2) LCD_ShowxNum(title_life.x,title_life.y+32,life2,1,16,0);
	LCD_ShowxNum(title_score.x,title_score.y+16,score1,5,16,0);
	if(plyernum==2) LCD_ShowxNum(title_score.x,title_score.y+32,score2,5,16,0);
		delay_ms(10);
	}
}


void BuildAIBullet(Tank* tank)   //AI子弹发射   这个函数是AI坦克依照规则开火的函数，具体如下
{
	if (tank->CD == 15)
	{
		if (!(RNG_Get_RandomNum() % 11))     //冷却结束后在随后的每个游戏周期中有10分之一的可能发射子弹
		{
			BuildBullet(*tank);
			tank->CD = 0;
		}
	}
	else
		tank->CD++;
	if (tank->CD >= 14)       //AI强化部分，在冷却到达一定范围即可使用
	{
		if (tank->y == YMAX-TANKWI/2)     //如果坦克在底部(这个最优先)
		{
			if (tank->x < XMED-HOME_WIDTH/2) //在老家左边
		
				if (tank->direction == RIGHT)  //坦克方向朝右
				{
					BuildBullet(*tank);     //发射子弹
					tank->CD = 0;
				}
			}
			else             //在老家右边
				if (tank->direction == LEFT)   //坦克方向朝左
				{
					BuildBullet(*tank);     //发射子弹
					tank->CD = 0;
				}
		}
		if(plyernum==1){
		if (tank->x == my_tank.x +TANKWI/2 || tank->x == my_tank.x || tank->x == my_tank.x - TANKWI/2)  //AI坦克在纵向上"炮口"对准我的坦克 即AI tank与玩家坦克相隔半个身位，此时AI要将炮口对准玩家   
		{
			if ((tank->direction == DOWN && my_tank.y > tank->y) || (tank->direction == UP && my_tank.y < tank->y))
			{                               //若是AI朝下并且我的坦克在AI坦克下方(数值大的在下面)或者AI朝上我的坦克在AI上方
				int big = my_tank.y, smal = tank->y, i;
				if (my_tank.y < tank->y)
				{
					big = tank->y;
					smal = my_tank.y;
				}
				for (i = smal + TANKWI/2+1; i <= big - (TANKWI/2+1); i++)  //判断AI炮口的直线上两坦克间有无障碍  
					if (map[i][tank->x] != BLACK||map[i][tank->x] !=WATER )      //若有障碍 ，这里的障碍判定为读取颜色判定是否有障碍
						break;
				if (i == big - TANKWI/2)                //若i走到big-?说明无障碍  
				{
					BuildBullet(*tank);     //则发射子弹
					tank->CD = 0;
				}
			}
		}
		else if (tank->y == my_tank.y + TANKWI/2|| tank->y == my_tank.y || tank->y == my_tank.y - TANKWI/2) //AI坦克在横向上"炮口"对准我的坦克    //此部分是将上部分的方向改变，变成AI tank横向打玩家
		{
			if (tank->direction == RIGHT && my_tank.x > tank->x || tank->direction == LEFT && my_tank.x < tank->x)
			{                  //若是AI朝右并且我的坦克在AI坦克右方(数值大的在下面)或者AI朝左我的坦克在AI左方
				int big = my_tank.y, smal = tank->y, i;
				if (my_tank.x < tank->x)
				{
					big = tank->x;
					smal = my_tank.x;
				}
				for (i = smal + TANKWI/2+1; i <= big - (TANKWI/2+1); i++)  //判断AI炮口的直线上两坦克间有无障碍
					if (map[tank->y][i] != BLACK ||map[i][tank->x] !=WATER )      //若有障碍
						break;
				if (i == big - TANKWI/2)   //若i走到big-TANKWI/2说明无障碍
				{
					BuildBullet(*tank);     //则发射子弹
					tank->CD = 0;
				}
			}
		}
	}
	if(plyernum==2){
		if (tank->x == my_tank2.x +TANKWI/2 || tank->x == my_tank2.x || tank->x == my_tank2.x - TANKWI/2)  //AI坦克在纵向上"炮口"对准我的坦克 即AI tank与玩家坦克相隔半个身位，此时AI要将炮口对准玩家   
		{
			if ((tank->direction == DOWN && my_tank2.y > tank->y) || (tank->direction == UP && my_tank2.y < tank->y))
			{                               //若是AI朝下并且我的坦克在AI坦克下方(数值大的在下面)或者AI朝上我的坦克在AI上方
				int big = my_tank2.y, smal = tank->y, i;
				if (my_tank2.y < tank->y)
				{
					big = tank->y;
					smal = my_tank2.y;
				}
				for (i = smal + TANKWI/2+1; i <= big - (TANKWI/2+1); i++)  //判断AI炮口的直线上两坦克间有无障碍  
					if (map[i][tank->x] != BLACK||map[i][tank->x] !=WATER )      //若有障碍 ，这里的障碍判定为读取颜色判定是否有障碍
						break;
				if (i == big - TANKWI/2)                //若i走到big-?说明无障碍  
				{
					BuildBullet(*tank);     //则发射子弹
					tank->CD = 0;
				}
			}
		}
		else if (tank->y == my_tank2.y + TANKWI/2|| tank->y == my_tank2.y || tank->y == my_tank2.y - TANKWI/2) //AI坦克在横向上"炮口"对准我的坦克    //此部分是将上部分的方向改变，变成AI tank横向打玩家
		{
			if (tank->direction == RIGHT && my_tank2.x > tank->x || tank->direction == LEFT && my_tank2.x < tank->x)
			{                  //若是AI朝右并且我的坦克在AI坦克右方(数值大的在下面)或者AI朝左我的坦克在AI左方
				int big = my_tank2.y, smal = tank->y, i;
				if (my_tank2.x < tank->x)
				{
					big = tank->x;
					smal = my_tank2.x;
				}
				for (i = smal + TANKWI/2+1; i <= big - (TANKWI/2+1); i++)  //判断AI炮口的直线上两坦克间有无障碍
					if (map[tank->y][i] != BLACK ||map[i][tank->x] !=WATER )      //若有障碍
						break;
				if (i == big - TANKWI/2)   //若i走到big-TANKWI/2说明无障碍
				{
					BuildBullet(*tank);     //则发射子弹
					tank->CD = 0;
				}
			}
		}
	}
}

void BuildmyBullet(Tank*my_tank)
{
	if (my_tank->CD == 5)
	{
		if (key1==5&& my_tank->my==1)//此处为检验攻击键读入，我不会
		{
			BuildBullet(*my_tank);
			my_tank->CD = 0;
		}
		else if(key2==5&& my_tank->my==2){
			BuildBullet(*my_tank);
			my_tank->CD = 0;
		}
	}
	else
		my_tank->CD++;
}

void BuildBullet(Tank tank)  //子弹发射（建立）,传入结构体Tank,这里包含改变了全局变量结构体bullet
{                            //∵实现方式为顺序循环建立子弹，每次调用改变的bullet数组元素都不同
	switch (tank.direction)   //∴为了方便,不将bullet放入参数,bullet作为全局变量使用
	{
	case UP:
		bullet[bul_num].x = tank.x;
		bullet[bul_num].y = tank.y - (TANKWI/2+1);
		bullet[bul_num].direction = UP;                              //定义子弹方向，一开始已定义
		break; 
	case DOWN:
		bullet[bul_num].x = tank.x;
		bullet[bul_num].y = tank.y + (TANKWI/2+1);
		bullet[bul_num].direction = DOWN;
		break;
	case LEFT:
		bullet[bul_num].x = tank.x - (TANKWI/2+1);
		bullet[bul_num].y = tank.y;
		bullet[bul_num].direction = LEFT;
		break;
	case RIGHT:
		bullet[bul_num].x = tank.x + (TANKWI/2+1);
		bullet[bul_num].y = tank.y;
		bullet[bul_num].direction = RIGHT;
		break;
	}
	bullet[bul_num].exist = 1;    //子弹被建立,此值为1则此子弹存在
	bullet[bul_num].initial = 1;  //子弹处于初建立状态
	bullet[bul_num].my = tank.my;   //如果是玩家1坦克发射的子弹bullet.my=1，玩家2，2；否则为0
	bul_num++;
	if (bul_num == BULLET_NUM)        //如果子弹编号增长到最大子弹数，那么重头开始编号
		bul_num = 0;                 
}



int BulletFly(Bullet bullet[]) //子弹移动和打击
{                                         //含有全局变量Bullet的改变
	for (int i = 0; i < BULLET_NUM; i++)
	{
		if (bullet[i].exist)              //如果子弹存在
		{
			if (bullet[i].initial == 0)     //如果子弹不是初建立的
			{
				if (map[bullet[i].y][bullet[i].x] == BLACK || map[bullet[i].y][bullet[i].x] == WATER)   //如果子弹坐标当前位置无障碍
					ClearBullet(bullet[i].x, bullet[i].y, BulletCheak(bullet[i].x, bullet[i].y));     //抹除子弹图形
				switch (bullet[i].direction)                                      //然后子弹坐标变化（子弹变到下一个坐标）
				{
				case UP:(bullet[i].y)--; break;
				case DOWN:(bullet[i].y)++; break;
				case LEFT:(bullet[i].x)--; break;
				case RIGHT:(bullet[i].x)++; break;
				}
			}
			int collide = BulletCheak(bullet[i].x, bullet[i].y);   //判断子弹当前位置情况,判断子弹是否碰撞,是否位于水面上。
			if (collide)                                                //如果检测到当前子弹坐标无障碍(无碰撞)（包括在地面上与在水面上）
				PrintBullet(bullet[i].x, bullet[i].y, collide);       //则打印子弹，若有碰撞则不打印
			else{
				
				return BulletHit(&bullet[i]);     //若有碰撞则执行子弹碰撞函数  
			}
			if (bullet[i].initial)             //若子弹初建立，则把初建立标记去除
				bullet[i].initial = 0;
			for (int j = 0; j < BULLET_NUM; j++)  //子弹间的碰撞判断,若是我方子弹和敌方子弹碰撞则都删除,若为两敌方子弹则无视
				if (bullet[j].exist && j != i  && bullet[i].x == bullet[j].x && bullet[i].y == bullet[j].y)
				{                              //同样的两颗我方子弹不可能产生碰撞不予考虑
					bullet[j].exist = 0;
					bullet[i].exist = 0;
					ClearBullet(bullet[j].x, bullet[j].y, BulletCheak(bullet[j].x, bullet[j].y));  //抹除j子弹图形,子弹i图形已被抹除
					break;
				}
		}
	}
	return 2;
}




int BulletHit(Bullet* bullet)  //含有Tank全局变量的修改,子弹间的碰撞不在本函数,子弹间碰撞已在BulletShoot中检测并处理及上个函数尾部
{                               //∵每次打中的坦克都不一样，不可能把所有坦克放在参数表中
	int x = bullet->x;            //∴这里的Tank使用全局变量
	int y = bullet->y;            //这里传入的值是子弹坐标,这两个值不需要改变
	int i;
	if (map[y][x] == WALL1)  //子弹碰到可以破坏砖块 
	{
		if (bullet->direction == UP || bullet->direction == DOWN)   //如果子弹是纵向的
			for (i = -2; i <= 2; i++)
				if (map[y][x + i] == WALL1)  //如果子弹打中砖块两旁为砖块,则删除砖,若不是(一旁为坦克或其他地形)则忽略
				{
					map[y][x + i] = BLACK;
					LCD_Fill((x+i)*4,y*4,(x+i)*4+3,y*4+3,BLACK);
				}
		if (bullet->direction == LEFT || bullet->direction == RIGHT)     //若子弹是横向的  (与子弹纵向实现同理)
			for (i = -2; i <= 2; i++)
				if (map[y + i][x] == WALL1)
				{
					 map[y + i][x] = BLACK;
					 LCD_Fill(x*4,(y+i)*4,x*4+3,(y+i)*4+3,BLACK);
				}
		bullet->exist = 0;           //这颗子弹已经不存在了
	}
	else if (map[y][x] == WALL3 ||map[y][x]==WALL4)  //子弹碰到边框或者不可摧毁方块
		bullet->exist = 0;
	else if (bullet->my &&map[y][x] >= 100 && map[y][x] < 104)  //若我的子弹碰到了敌方坦克
	{
		int num = map[y][x] % 100;
		if (AI_tank[num].model == 3 && AI_tank[num].color == COLOR3)   //若为firm tank,且为第一形态,表明没有受到伤害  
			AI_tank[num].color = COLOR4;                        //则变成color4
		else if (AI_tank[num].model == 3 && AI_tank[num].color == COLOR4)
			AI_tank[num].color = COLOR5;                        //4为红色
		else                       //其他类型的坦克或者firm tank为红色的情况
		{if(bullet->my==1){
			if(AI_tank[num].model==1)	//加玩家分
				score1+=100;
			if(AI_tank[num].model==2)
				score1+=300;
			if(AI_tank[num].model==3)
				score1+=500;
			}
			else if(bullet->my==2){
			if(AI_tank[num].model==1)	//加玩家分
				score2+=100;
			if(AI_tank[num].model==2)
				score2+=300;
			if(AI_tank[num].model==3)
				score2+=500;
			}
			AI_tank[num].alive = 0;
			remain_enemy--;
			ClearTank(AI_tank[num].x, AI_tank[num].y);      //清除该坦克
		}
		bullet->exist=0;
	}
	else if (map[y][x] == COLOR0 &&my_tank.alive==1&& bullet->my!=my_tank.my)   //若敌方子弹击中玩家1坦克
	{
		my_tank.alive = 0;
		ClearTank(my_tank.x, my_tank.y);
		bullet->exist = 0;
		      //我的坦克复活次数+1(∵我的坦克复活次数与生命值有关∴放在这里自减)
		life1--;
	}		
		else if (map[y][x] == COLOR7 &&my_tank2.alive==1&& bullet->my!=my_tank2.my)   //若敌方子弹击中玩家2坦克
	{
		my_tank2.alive = 0;
		ClearTank(my_tank2.x, my_tank2.y);
		bullet->exist = 0;
		      //我的坦克复活次数+1(∵我的坦克复活次数与生命值有关∴放在这里自减)
		life2--;
	}	
	else if(bullet->my==0 && (100 <=map[y][x]&&map[y][x]<=103) )
			bullet->exist=0;
	else if (map[y][x] == HOME)      //子弹碰到家(无论是谁的子弹)
	{
		delay_ms(1000);
		return game_end(0);
	}
	return 2;
}

int BulletCheak(int x, int y)  //判断子弹当前位置情况,判断子弹是否碰撞,是否位于水面上。
{                              //有障碍返回0,无障碍且子弹在地面返回1，子弹在水面上返回2
	if (map[y][x] == BLACK)
		return 1;
	else if (map[y][x] == WATER)//水面颜色
		return 2;
	else
		return 0;
}


void PrintBullet(int x, int y, int T)   //当前坐标BulletCheak 的值做参量 T
{
	 LCD_Fill(x*4,y*4,x*4+3,y*4+3,YELLOW);
}

void ClearBullet(int x, int y, int T)   //回复当前背景
{
	if (T == 2)        //  T==2 表示子弹当前坐标在水面上
	{
		 LCD_Fill(x*4,y*4,x*4+3,y*4+3,BLUE);
	}
	else if (T == 1)   //  T==1 表示子弹当前坐标在陆地上
	{
		 LCD_Fill(x*4,y*4,x*4+3,y*4+3,BLACK);
	}
}

//position为坦克生成位置,-1为左位置,0为中间,1为右,2为我的坦克位置
void BuildAITank(int* position, Tank* AI_tank)   //执行一次该函数只建立一个坦克
{                                         //rand函数公式：0<=rand()%(a+1)<=a  0+m<=rand()%(n-m+1)+m<=n  
										  //rand函数实现1到n：1<=rand()%(n)+1<=n
	if (AIPositionCheak(*position))        //若此位置无障碍,可生成。position参数详见AIPositionCheak函数定义
	{
		AI_tank->x =(*position==1? 55 : XMED + (XMED-(TANKWI/2+1+1)) * (*position));  //对应三个生成位置的x假坐标
		AI_tank->y = TANKWI/2+1;
		if (AI_tank->revive == level_info[stgindx - 1].firm_tank_order)  //坦克出现(复活)次序==关卡信息(level_info)中firm tank的出现次序
		{
			AI_tank->model = 3;           //?为firm tank的模型(外观) 
			AI_tank->color = COLOR3;           //颜色参数
		}
		else if (AI_tank->revive == level_info[stgindx - 1].fast_tank_order)  //同上if，这里是fast_tank的
		{
			AI_tank->model = 2;
			AI_tank->color = COLOR2; 
		}
		else      //普通坦克
		{
			AI_tank->model = 1;
			AI_tank->color = COLOR1; 
		}
		AI_tank->alive = 1;       //坦克变为存在
		AI_tank->direction = DOWN;  //方向朝下
		AI_tank->revive++;        //复活次数+1
		PrintTank(*AI_tank);
		(*position)++;
		{
			}//插入在副屏幕上打印剩余坦克数函数
		if (*position == 2)          //position只能为0,1，-1，这里position循环重置
			* position = -1;
		return;                  //若生成了一辆坦克，则结束该函数
	}
}


int AIPositionCheak(int position)    //position为坦克生成位置2为我的坦克位置，其余为AI位，-1为左位，0为中间位置，1为右位
{
	int	x, y;

		y = 4, x =(position==1? 55 : XMED + (XMED-(TANKWI/2+1+1)) * (position));  //XMED + (XMED-(TANKWI/2+1)) * (*position) 对应三个生成位置的x假坐标
	for (int i = -TANKWI/2; i <=TANKWI/2; i++)
		for (int j = -TANKWI/2; j <=TANKWI/2; j++)
			if (map[y + j ][x + i ] != BLACK)  //如果遍历的X宫格里有障碍物
				return 0;              //则返回0，表示此生成位置有阻碍
	return 1;                          //否则生成1，表示此生成位置无阻碍
}


void MoveAITank(Tank* AI_tank) //AI专用函数，该函数主要为AI加强
{
	if (AI_tank->alive)         //如果坦克活着
	{
		if (AI_tank->stop != 0)   //坦克是否停止运动的判断，若stop参数不为0
		{
			AI_tank->stop--;   //则此坦克本回合停止运动
			return;
		}
		if (!(RNG_Get_RandomNum() % 23))     //22分之1的概率执行方向重置
		{
			AI_tank->direction = RNG_Get_RandomNum() % 4 + 1;
			if (RNG_Get_RandomNum() % 3)     //在方向重置后有2分之1的概率停止走动3步的时间
			{
				AI_tank->stop = 2;
				return;
			}
		}
		ClearTank(AI_tank->x, AI_tank->y);
		if (TankCheak(*AI_tank, AI_tank->direction))   //如果前方无障碍
			switch (AI_tank->direction)
			{
			case UP: AI_tank->y--; break;  //上前进一格
			case DOWN: AI_tank->y++; break;  //下前进一格
			case LEFT: AI_tank->x--; break;  //左前进一格
			case RIGHT: AI_tank->x++; break;  //右前进一格
			}
		else                     //前方有障碍
		{
			if (!(RNG_Get_RandomNum() % 4))      //3分之1的概率乱转
			{
				AI_tank->direction = RNG_Get_RandomNum() % 4 + 1;
				AI_tank->stop = 2; //乱转之后停止走动3步的时间
				PrintTank(*AI_tank);
				return;          //∵continue会跳过下面的打印函数,∴这里先打印
			}
			else                 //另外3分之2的几率选择正确的方向
			{
				int j;
				for (j = 1; j <= 4; j++)
					if (TankCheak(*AI_tank, j))  //循环判断坦克四周有无障碍,此函数返值1为可通过
						break;
				if (j == 5)         //j==5说明此坦克四周都有障碍物，无法通行
				{
					PrintTank(*AI_tank);
					return;      //则跳过下面的while循环以防程序卡死
				}
				while (TankCheak(*AI_tank, AI_tank->direction) == 0)  //如果前方仍有障碍
					AI_tank->direction = (RNG_Get_RandomNum() % 4 + 1);                    //则换个随机方向检测
			}
		}
		PrintTank(*AI_tank);     //打印AI坦克
	}
}

void BuildMyTank(Tank* my_tank) //建立我的坦克
{
	if(life1==0&&my_tank->my==1) return;
	if(life2==0&&my_tank->my==2) return;
    int flag=1;
	if(my_tank->my==1){
		my_tank->x = 14;//位置位于基地左侧
		my_tank->y = 55;
		my_tank->stop = NULL;
		my_tank->direction = UP;
		my_tank->model = 0;
		my_tank->color = COLOR0;
		my_tank->CD = 5;
	}
	else{
		my_tank->x = 45;//位置位于基地左侧
		my_tank->y = 55;
		my_tank->stop = NULL;
		my_tank->direction = UP;
		my_tank->model = 0;
		my_tank->color = COLOR7;
		my_tank->CD = 5;
	}
	for(int i=-TANKWI/2;i<=TANKWI/2;i++)
	for(int j=-TANKWI/2;j<=TANKWI/2;j++)
	{
		if(map[my_tank->y+j][my_tank->x+i] != BLACK)
		flag=0;
	}
	if(flag){	
		my_tank->alive = 1;
		PrintTank(*my_tank);   //打印我的坦克
		my_tank->revive++;
	}
}


void MoveMyTank(int turn,Tank* tank)   //玩家专用函数，turn为keyboard函数里因输入不同方向键而传入的不同的值
{
	if(tank->alive==0) return;
	ClearTank(tank->x, tank->y);        //map 数组中“我的坦克”参数清除工作已在此函数中完成
	tank->direction = turn;                  //将键盘输入的方向值传入我的坦克方向值
	if (TankCheak(*tank, tank->direction))  //若此时我的坦克当前方向上无障碍
		switch (turn)
		{
		case UP: tank->y--; break;  //上前进一格
		case DOWN: tank->y++; break;  //下前进一格
		case LEFT: tank->x--; break;  //左前进一格
		case RIGHT: tank->x++; break;  //右前进一格
		}                                        //若坦克当前方向上有障碍则跳过坐标变化直接打印该转向的坦克
	PrintTank(*tank);
}


bool TankCheak(Tank tank, int direction)  //检测坦克前方障碍函数,参量为假坐标。返值1为可通过,返值0为阻挡(人机共用)
{
	int i,flag=1;
	switch (direction)                    //direction变量  
	{
	case UP:
            for(i=-TANKWI/2;i<=TANKWI/2;i++)
		{
		if (map[tank.y - (TANKWI/2+1)][tank.x+i] != BLACK )
			flag=0;
		}
		return flag;
		
	case DOWN:
		for(i=-TANKWI/2;i<=TANKWI/2;i++)
		{if (map[tank.y + TANKWI/2+1][tank.x+i] != BLACK )
			flag=0;
		}
		return flag;
	case LEFT:
		for(i=-TANKWI/2;i<=TANKWI/2;i++)
		{if (map[tank.y+i][tank.x -(TANKWI/2+1) ] != BLACK )
			flag=0;}
		return flag;
	case RIGHT:
		for(i=-TANKWI/2;i<=TANKWI/2;i++)
		{if (map[tank.y+i][tank.x + TANKWI/2+1] != BLACK )
			flag=0;}
		return flag;
	default:
		return 0;
	}
}


void ClearTank(int x, int y)   //清除坦克函数（人机共用）
{
	for (int i = -TANKWI/2; i <= TANKWI/2; i++)
		for (int j = -TANKWI/2; j <= TANKWI/2; j++)
		{                     
			map[y + j][x + i] = BLACK;  //将方格置为背景 
		}
	LCD_Fill((x-TANKWI/2)*4,(y-TANKWI/2)*4,(x+TANKWI/2)*4+3,(y+TANKWI/2)*4+3,BLACK);
}


void PrintTank(Tank tank)     //打印坦克（人机共用） 由于读取的Tank参数较多,故就不将参数一一传入了
{   
		int i,j;
	//LCD_Fill((tank.x-TANKWI/2)*4,(tank.y-TANKWI/2)*4,(tank.x+TANKWI/2)*4+3,(tank.y+TANKWI/2)*4+3,tank.color);
		for (i = -TANKWI/2; i <= TANKWI/2; i++)
		for (j = -TANKWI/2; j <= TANKWI/2; j++){
			if (!tank.my)       
				map[tank.y + j ][tank.x + i ] = 100 + tank.num;
				else if(tank.my==1)
					map[tank.y + j ][tank.x + i ]= COLOR0;
				else
					map[tank.y + j ][tank.x + i ]= COLOR7;
		}
	if(tank.my==1&&tank.direction==UP)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_my_tank[3+j][3+i]);
					}
				} 
		}
	if(tank.my==1&&tank.direction==DOWN)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_my_tank[3-j][3+i]);
					}
				}
		}
	if(tank.my==1&&tank.direction==LEFT)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_my_tank[3+i][3-j]);
					}
				}
		}
	if(tank.my==1&&tank.direction==RIGHT)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_my_tank[3-i][3-j]);
					}
				}
		}
	if(tank.my==2&&tank.direction==UP)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_my_tank2[3+j][3+i]);
					}
				} 
		}
		
	if(tank.my==2&&tank.direction==DOWN)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_my_tank2[3-j][3+i]);
					}
				}
		}
	if(tank.my==2&&tank.direction==LEFT)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_my_tank2[3+i][3-j]);
					}
				}				
		}
	if(tank.my==2&&tank.direction==RIGHT)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_my_tank2[3-i][3-j]);
					}
				}				
		}
		
	if(tank.model==1&&tank.direction==UP)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_normal_tank[3+j][3+i]);
					}
				}				
		}
	if(tank.model==1&&tank.direction==DOWN)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_normal_tank[3-j][3+i]);
					}
				}				
		}
	if(tank.model==1&&tank.direction==LEFT)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_normal_tank[3+i][3-j]);
					}
				}					
		}
	if(tank.model==1&&tank.direction==RIGHT)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_normal_tank[3-i][3-j]);
					}
				}				
		}
	if(tank.model==2&&tank.direction==UP)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_fast_tank[3+j][3+i]);
					}
				}	
		}
	if(tank.model==2&&tank.direction==DOWN)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_fast_tank[3-j][3+i]);
					}
				}	
		}
	if(tank.model==2&&tank.direction==LEFT)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_fast_tank[3+i][3-j]);
					}
				}
		}
	if(tank.model==2&&tank.direction==RIGHT)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_fast_tank[3-i][3-j]);
					}
				}
		}
	if(tank.model==3&&tank.direction==UP&&tank.color==COLOR3)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank1[3+j][3+i]);
					}
				}	
		}
	if(tank.model==3&&tank.direction==DOWN&& tank.color==COLOR3)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank1[3-j][3+i]);
					}
				}	
		}
	if(tank.model==3&&tank.direction==LEFT&&tank.color==COLOR3)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank1[3+i][3-j]);
					}
				}	
		}
	if(tank.model==3&&tank.direction==RIGHT&&tank.color==COLOR3)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank1[3-i][3-j]);
					}
				}	
		}
	if(tank.model==3&&tank.direction==UP&&tank.color==COLOR4)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank2[3+j][3+i]);
					}
				}	
		}
	if(tank.model==3&&tank.direction==DOWN&&tank.color==COLOR4)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank2[3-j][3+i]);
					}
				}	
		}
	if(tank.model==3&&tank.direction==LEFT&&tank.color==COLOR4)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank2[3+i][3-j]);
					}
				}	
		}
	if(tank.model==3&&tank.direction==RIGHT&&tank.color==COLOR4)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank2[3-i][3-j]);
					}
				}
		}
	if(tank.model==3&&tank.direction==UP&&tank.color==COLOR5)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank3[3+j][3+i]);
					}
				}	
		}
	if(tank.model==3&&tank.direction==DOWN&&tank.color==COLOR5)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank3[3-j][3+i]);
					}
				}	
		}
	if(tank.model==3&&tank.direction==LEFT&&tank.color==COLOR5)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank3[3+i][3-j]);
					}
				}	
		}
	if(tank.model==3&&tank.direction==RIGHT&&tank.color==COLOR5)
		{
				for(i=-3;i<=3;i++){
					for(j=-3;j<=3;j++){
						LCD_Fill((tank.x+i)*4,(tank.y+j)*4,(tank.x+i)*4+3,(tank.y+j)*4+3,print_firm_tank3[3-i][3-j]);
					}
				}	
		}
		
}


void Initialize()      //初始化
{
	my_tank.revive = 0;  //我的坦克复活次数为0
	my_tank.my=1;
	position = 0;
	bul_num = 0;
	BuildMyTank(&my_tank);
	if(plyernum==2) 
	{
	my_tank2.revive=0;
	my_tank2.my=2;
	BuildMyTank(&my_tank2);
	}
	for (int i = 0; i < 50; i++)     //子弹初始化
	{
		bullet[i].exist = 0;
		bullet[i].initial = 0;
	}
	for (int i = 0; i <= 3; i++)         //AI坦克初始化
	{
		AI_tank[i].revive = 0;
		AI_tank[i].alive = 0;  //初始化坦克全是不存活的，BuildAITank()会建立重新建立不存活的坦克
		AI_tank[i].stop = 0;
		AI_tank[i].num = i;
		AI_tank[i].my = 0;
		AI_tank[i].CD = 0;
	}

}

int GameCheck()
{                           //剩余敌人为0且四坦克全部不存活
	if (remain_enemy <= 0 && !AI_tank[0].alive && !AI_tank[1].alive && !AI_tank[2].alive && !AI_tank[3].alive)
		return game_end(1);
	if (life1==0&&life2==0)   //我的生命值(复活次数)全部用完 MAX_LIFE
		return game_end(0);        //游戏结束，传入0代表我的复活次数用光(生命值为0)。游戏结束有两种判断，另一种是老家被毁
	else return 2;
}
