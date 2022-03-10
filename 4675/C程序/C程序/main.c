//  采用DHT11   1602A 液晶显示程序。
//  可以用按键设置保存温湿度上下限。 带蜂鸣器
//   2013年11月， 供大家学习 
#include <reg52.h>
#include "1602.h"
#include "dht.h"
#include "2402.h" 

//定义三个LED灯
sbit L2=P1^3;		
sbit L3=P1^4;		
sbit L4=P1^5;
sbit L5=P1^6;		
sbit Key_SET = P3^2;			//设置键
sbit Key_UP= P3^3;				//加  键
sbit Key_DOWN = P3^4;			//减  键
sbit Key_SAVE = P3^5;			//保存键
sbit Relay = P2^0;

//定义标识
volatile bit FlagStartRH = 0;  //开始温湿度转换标志
volatile bit FlagKeyPress = 0; //有键按下


//定义温湿度传感器用外部变量
extern U8  U8FLAG,k;
extern U8  U8count,U8temp;
extern U8  U8T_data_H,U8T_data_L,U8RH_data_H,U8RH_data_L,U8checkdata;
extern U8  U8T_data_H_temp,U8T_data_L_temp,U8RH_data_H_temp,U8RH_data_L_temp,U8checkdata_temp;
extern U8  U8comdata;
extern U8  count, count_r;

U16 temp;
S16 temperature, humidity;
S16 idata TH, TL, HH, HL;  //温度上限和湿度上限
char * pSave;
U8 keyvalue, keySET, keySAVE, keyUP, keyDOWN;

//定义变量
U16 RHCounter;
U8 FlagSet,hide;


//数据初始化
void Data_Init()
{
   RHCounter = 0;
   L2 = 1;
   L3 = 1;
   L4 = 1;
   TH = 40;
   TL = 20;
   HH = 85;
   HL = 20;
   keyvalue = 0;
   keySET = 1;
   keySAVE = 1;
   keyUP = 1;
   keyDOWN = 1;
   FlagSet = 0;
   hide = 0;
}

//定时器0初始化
void Timer0_Init()
{
	ET0 = 1;        //允许定时器0中断
	TMOD = 1;       //定时器工作方式选择
	TL0 = 0x06;     
	TH0 = 0xf8;     //定时器赋予初值
	TR0 = 1;        //启动定时器
}

//定时器0中断
void Timer0_ISR (void) interrupt 1 using 0
{
	TL0 = 0x06;
	TH0 = 0xf8;     //定时器赋予初值

    RHCounter ++;
	//设闪烁标志
	if (RHCounter>400) hide = 0;
	else hide = 1;
	//每2秒钟启动一次温湿度转换
    if (RHCounter >= 800)
    {
       FlagStartRH = 1;
	   RHCounter = 0;
    }
}

//存入设定值、
void Save_Setting()
{
   pSave =  (char *)&TL;	  //地址低位对应低8位，高位对应高8位
   wrteeprom(0, *pSave);	  //存温度上限值TH低8位
   DELAY(500);
   pSave ++;
   wrteeprom(1, *pSave);	  //存温度上限值TH高8位
   DELAY(500);
   pSave =  (char *)&TH;
   wrteeprom(2, *pSave);	  //存温度下限值TL低8位
   DELAY(500);
   pSave ++;
   wrteeprom(3, *pSave);	  //存温度下限值TL高8位
   DELAY(500);
   pSave =  (char *)&HL;	  //地址低位对应低8位，高位对应高8位
   wrteeprom(4, *pSave);	  //存湿度上限值HH低8位
   DELAY(500);
   pSave ++;
   wrteeprom(5, *pSave);	  //存湿度上限值HH高8位
   DELAY(500);
   pSave =  (char *)&HH;
   wrteeprom(6, *pSave);	  //存湿度下限值HL低8位
   DELAY(500);
   pSave ++;
   wrteeprom(7, *pSave);	  //存湿度下限值HL高8位
   DELAY(500);

}

//载入设定值、
void Load_Setting()
{
   pSave =  (char *)&TL;
   *pSave++ = rdeeprom(0);
   *pSave = rdeeprom(1);
   pSave = (char *)&TH;
   *pSave++ = rdeeprom(2);
   *pSave = rdeeprom(3);
   pSave =  (char *)&HL;
   *pSave++ = rdeeprom(4);
   *pSave = rdeeprom(5);
   pSave = (char *)&HH;
   *pSave++ = rdeeprom(6);
   *pSave = rdeeprom(7);

   if ((TL>99)||(TL<0)) TL = 20;
   if ((TH>99)||(TH<0)) TH = 40;
   if ((HL>99)||(HL<0)) HH = 20;
   if ((HH>99)||(HH<0)) HH = 85;
}

void KeyProcess(uint num)
{
   //键盘设置功能：通过设置键和加、减键修改当前设置，只有按下保存键才存盘，否则掉电不保存。
   switch (num)
   {
	  case 1:		//设置键按下，依次设定TL\TH\HL\HH\退出
         FlagSet ++;
		 if (FlagSet>4) 
		 {
		    FlagSet = 0;
	        L1602_char(1, 12, TL/10+48);
	        L1602_char(1, 13, TL%10+48);
	        L1602_char(1, 15, TH/10+48);
	        L1602_char(1, 16, TH%10+48);
	        L1602_char(2, 12, HL/10+48);
	        L1602_char(2, 13, HL%10+48);
	        L1602_char(2, 15, HH/10+48);
	        L1602_char(2, 16, HH%10+48);
		 }
		 break;
	  case 2:		//加键按下
	     if (FlagSet==1)	 //设TL
		 {
		    if (TL<100)  TL++;
	        L1602_char(1, 12, TL/10+48);
	        L1602_char(1, 13, TL%10+48);
		 }
	     if (FlagSet==2)	 //设TH
		 {
		    if (TH<100)  TH++;
	        L1602_char(1, 15, TH/10+48);
	        L1602_char(1, 16, TH%10+48);
		 }
	     if (FlagSet==3)	 //设HL
		 {
		    if (HL<100)  HL++;
	        L1602_char(2, 12, HL/10+48);
	        L1602_char(2, 13, HL%10+48);
		 }
	     if (FlagSet==4)	 //设HH
		 {
		    if (HH<100)  HH++;
	        L1602_char(2, 15, HH/10+48);
	        L1602_char(2, 16, HH%10+48);
		 }
	     break;
	  case 3:	   //减键按下
	     if (FlagSet==1)	 //设TL
		 {
		    if (TL>0)  TL--;
	        L1602_char(1, 12, TL/10+48);
	        L1602_char(1, 13, TL%10+48);
		 }
	     if (FlagSet==2)	 //设TH
		 {
		    if (TH>0)  TH--;
	        L1602_char(1, 15, TH/10+48);
	        L1602_char(1, 16, TH%10+48);
		 }
	     if (FlagSet==3)	 //设HL
		 {
		    if (HL>0)  HL--;
	        L1602_char(2, 12, HL/10+48);
	        L1602_char(2, 13, HL%10+48);
		 }
	     if (FlagSet==4)	 //设HH
		 {
		    if (HH>0)  HH--;
	        L1602_char(2, 15, HH/10+48);
	        L1602_char(2, 16, HH%10+48);
		 }
	     break;
	  case 4:		  //保存键按下
		 Save_Setting();
		 FlagSet = 0;
	     break;
	  default:
	     break;
   }

}

/********************************************************************
* 名称 : Main()
* 功能 : 主函数
***********************************************************************/
void main()
{
    U16 i, j, testnum;

    EA = 0;

	Timer0_Init();  //定时器0初始化

    Data_Init();
	EA = 1;

	L1602_init();
	L1602_string(1,1," Welcome to T&H   ");
	L1602_string(2,1," Control System!  ");
	//延时
	for (i=0;i<1000;i++)
	   for (j=0;j<1000;j++)
	   {;}
    //清屏
	L1602_string(1,1,"                ");
	L1602_string(2,1,"                ");
    L1602_string(1,1,"Tem:    C    -  ");
	L1602_string(2,1,"Hum:    %    -  ");
    
    //载入温度上限和湿度上限设定值
	Load_Setting();
	L1602_char(1, 12, TL/10+48);
	L1602_char(1, 13, TL%10+48);
	L1602_char(1, 15, TH/10+48);
	L1602_char(1, 16, TH%10+48);
	L1602_char(2, 12, HL/10+48);
	L1602_char(2, 13, HL%10+48);
	L1602_char(2, 15, HH/10+48);
	L1602_char(2, 16, HH%10+48);


	while(1)
	{
       //温湿度转换标志检查
		 if (FlagStartRH == 1)
		 {
		     TR0 = 0;
             testnum = RH();
 			 FlagStartRH = 0;
			 TR0 = 1;
             //读出温湿度，只取整数部分
			 humidity = U8RH_data_H;
			 temperature = U8T_data_H;
 			 //显示温湿度
			 L1602_int(1,5,temperature);
		     L1602_int(2,5,humidity);	
		}
		//闪烁显示
		if (hide == 1)
		{
		   switch (FlagSet)
		   {
		      case 0:  break;
			  case 1:  
			     L1602_string(1,12,"  ");
				 break;
		      case 2:
			     L1602_string(1,15,"  ");
				 break;
		      case 3:
			     L1602_string(2,12,"  ");
				 break;
			  case 4:
			     L1602_string(2,15,"  ");
				 break;
		   }
		}
		else
		{
	          L1602_char(1, 12, TL/10+48);
	          L1602_char(1, 13, TL%10+48);
              L1602_char(1, 15, TH/10+48);
	          L1602_char(1, 16, TH%10+48);
	          L1602_char(2, 12, HL/10+48);
	          L1602_char(2, 13, HL%10+48);
	          L1602_char(2, 15, HH/10+48);
	          L1602_char(2, 16, HH%10+48);
		}
		//温湿度控制
		//温度低于下限
		if (temperature < TL)  L2 = 0; 
		else    L2 = 1;			        
		//温度高于上限
		if (temperature > TH)  L3 = 0; 
		else    L3 = 1;			        
		//湿度低于下限
		if (humidity < HL  )   L4 = 0; 
		else    L4 = 1;			        
		//湿度高于上限
		if (humidity > HH)     L5 = 0; 
		else                   L5 = 1;	
		//控制继电器动作		        
		if ((temperature<TL)||(temperature>TH)||(humidity<HL)||(humidity>HH))	 Relay = 0;
		else  Relay = 1;
		 
		//键盘查询，在弹起时响应
		if ((Key_SET)&&(keySET==0)) {FlagKeyPress = 1; keyvalue = 1;}	  //设置键值1
        else if ((Key_UP)&&(keyUP==0)) {FlagKeyPress = 1; keyvalue = 2;}  //上  键值2
        else if ((Key_DOWN)&&(keyDOWN==0)) {FlagKeyPress = 1; keyvalue = 3;}//下键值3
        else if ((Key_SAVE)&&(keySAVE==0)) {FlagKeyPress = 1; keyvalue = 4;}//保存键值4
		if (FlagKeyPress == 1)
		{
           KeyProcess(keyvalue);
           FlagKeyPress = 0;           
		}
        if (!Key_SET) keySET = 0;
		else keySET = 1;
        if (!Key_UP) keyUP = 0;
		else keyUP = 1;
        if (!Key_DOWN) keyDOWN = 0;
		else keyDOWN = 1;
        if (!Key_SAVE) keySAVE = 0;
		else keySAVE = 1;
		DELAY(500); 
	}	
}