//  ����DHT11   1602A Һ����ʾ����
//  �����ð������ñ�����ʪ�������ޡ� ��������
//   2013��11�£� �����ѧϰ 
#include <reg52.h>
#include "1602.h"
#include "dht.h"
#include "2402.h" 

//��������LED��
sbit L2=P1^3;		
sbit L3=P1^4;		
sbit L4=P1^5;
sbit L5=P1^6;		
sbit Key_SET = P3^2;			//���ü�
sbit Key_UP= P3^3;				//��  ��
sbit Key_DOWN = P3^4;			//��  ��
sbit Key_SAVE = P3^5;			//�����
sbit Relay = P2^0;

//�����ʶ
volatile bit FlagStartRH = 0;  //��ʼ��ʪ��ת����־
volatile bit FlagKeyPress = 0; //�м�����


//������ʪ�ȴ��������ⲿ����
extern U8  U8FLAG,k;
extern U8  U8count,U8temp;
extern U8  U8T_data_H,U8T_data_L,U8RH_data_H,U8RH_data_L,U8checkdata;
extern U8  U8T_data_H_temp,U8T_data_L_temp,U8RH_data_H_temp,U8RH_data_L_temp,U8checkdata_temp;
extern U8  U8comdata;
extern U8  count, count_r;

U16 temp;
S16 temperature, humidity;
S16 idata TH, TL, HH, HL;  //�¶����޺�ʪ������
char * pSave;
U8 keyvalue, keySET, keySAVE, keyUP, keyDOWN;

//�������
U16 RHCounter;
U8 FlagSet,hide;


//���ݳ�ʼ��
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

//��ʱ��0��ʼ��
void Timer0_Init()
{
	ET0 = 1;        //����ʱ��0�ж�
	TMOD = 1;       //��ʱ��������ʽѡ��
	TL0 = 0x06;     
	TH0 = 0xf8;     //��ʱ�������ֵ
	TR0 = 1;        //������ʱ��
}

//��ʱ��0�ж�
void Timer0_ISR (void) interrupt 1 using 0
{
	TL0 = 0x06;
	TH0 = 0xf8;     //��ʱ�������ֵ

    RHCounter ++;
	//����˸��־
	if (RHCounter>400) hide = 0;
	else hide = 1;
	//ÿ2��������һ����ʪ��ת��
    if (RHCounter >= 800)
    {
       FlagStartRH = 1;
	   RHCounter = 0;
    }
}

//�����趨ֵ��
void Save_Setting()
{
   pSave =  (char *)&TL;	  //��ַ��λ��Ӧ��8λ����λ��Ӧ��8λ
   wrteeprom(0, *pSave);	  //���¶�����ֵTH��8λ
   DELAY(500);
   pSave ++;
   wrteeprom(1, *pSave);	  //���¶�����ֵTH��8λ
   DELAY(500);
   pSave =  (char *)&TH;
   wrteeprom(2, *pSave);	  //���¶�����ֵTL��8λ
   DELAY(500);
   pSave ++;
   wrteeprom(3, *pSave);	  //���¶�����ֵTL��8λ
   DELAY(500);
   pSave =  (char *)&HL;	  //��ַ��λ��Ӧ��8λ����λ��Ӧ��8λ
   wrteeprom(4, *pSave);	  //��ʪ������ֵHH��8λ
   DELAY(500);
   pSave ++;
   wrteeprom(5, *pSave);	  //��ʪ������ֵHH��8λ
   DELAY(500);
   pSave =  (char *)&HH;
   wrteeprom(6, *pSave);	  //��ʪ������ֵHL��8λ
   DELAY(500);
   pSave ++;
   wrteeprom(7, *pSave);	  //��ʪ������ֵHL��8λ
   DELAY(500);

}

//�����趨ֵ��
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
   //�������ù��ܣ�ͨ�����ü��ͼӡ������޸ĵ�ǰ���ã�ֻ�а��±�����Ŵ��̣�������粻���档
   switch (num)
   {
	  case 1:		//���ü����£������趨TL\TH\HL\HH\�˳�
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
	  case 2:		//�Ӽ�����
	     if (FlagSet==1)	 //��TL
		 {
		    if (TL<100)  TL++;
	        L1602_char(1, 12, TL/10+48);
	        L1602_char(1, 13, TL%10+48);
		 }
	     if (FlagSet==2)	 //��TH
		 {
		    if (TH<100)  TH++;
	        L1602_char(1, 15, TH/10+48);
	        L1602_char(1, 16, TH%10+48);
		 }
	     if (FlagSet==3)	 //��HL
		 {
		    if (HL<100)  HL++;
	        L1602_char(2, 12, HL/10+48);
	        L1602_char(2, 13, HL%10+48);
		 }
	     if (FlagSet==4)	 //��HH
		 {
		    if (HH<100)  HH++;
	        L1602_char(2, 15, HH/10+48);
	        L1602_char(2, 16, HH%10+48);
		 }
	     break;
	  case 3:	   //��������
	     if (FlagSet==1)	 //��TL
		 {
		    if (TL>0)  TL--;
	        L1602_char(1, 12, TL/10+48);
	        L1602_char(1, 13, TL%10+48);
		 }
	     if (FlagSet==2)	 //��TH
		 {
		    if (TH>0)  TH--;
	        L1602_char(1, 15, TH/10+48);
	        L1602_char(1, 16, TH%10+48);
		 }
	     if (FlagSet==3)	 //��HL
		 {
		    if (HL>0)  HL--;
	        L1602_char(2, 12, HL/10+48);
	        L1602_char(2, 13, HL%10+48);
		 }
	     if (FlagSet==4)	 //��HH
		 {
		    if (HH>0)  HH--;
	        L1602_char(2, 15, HH/10+48);
	        L1602_char(2, 16, HH%10+48);
		 }
	     break;
	  case 4:		  //���������
		 Save_Setting();
		 FlagSet = 0;
	     break;
	  default:
	     break;
   }

}

/********************************************************************
* ���� : Main()
* ���� : ������
***********************************************************************/
void main()
{
    U16 i, j, testnum;

    EA = 0;

	Timer0_Init();  //��ʱ��0��ʼ��

    Data_Init();
	EA = 1;

	L1602_init();
	L1602_string(1,1," Welcome to T&H   ");
	L1602_string(2,1," Control System!  ");
	//��ʱ
	for (i=0;i<1000;i++)
	   for (j=0;j<1000;j++)
	   {;}
    //����
	L1602_string(1,1,"                ");
	L1602_string(2,1,"                ");
    L1602_string(1,1,"Tem:    C    -  ");
	L1602_string(2,1,"Hum:    %    -  ");
    
    //�����¶����޺�ʪ�������趨ֵ
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
       //��ʪ��ת����־���
		 if (FlagStartRH == 1)
		 {
		     TR0 = 0;
             testnum = RH();
 			 FlagStartRH = 0;
			 TR0 = 1;
             //������ʪ�ȣ�ֻȡ��������
			 humidity = U8RH_data_H;
			 temperature = U8T_data_H;
 			 //��ʾ��ʪ��
			 L1602_int(1,5,temperature);
		     L1602_int(2,5,humidity);	
		}
		//��˸��ʾ
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
		//��ʪ�ȿ���
		//�¶ȵ�������
		if (temperature < TL)  L2 = 0; 
		else    L2 = 1;			        
		//�¶ȸ�������
		if (temperature > TH)  L3 = 0; 
		else    L3 = 1;			        
		//ʪ�ȵ�������
		if (humidity < HL  )   L4 = 0; 
		else    L4 = 1;			        
		//ʪ�ȸ�������
		if (humidity > HH)     L5 = 0; 
		else                   L5 = 1;	
		//���Ƽ̵�������		        
		if ((temperature<TL)||(temperature>TH)||(humidity<HL)||(humidity>HH))	 Relay = 0;
		else  Relay = 1;
		 
		//���̲�ѯ���ڵ���ʱ��Ӧ
		if ((Key_SET)&&(keySET==0)) {FlagKeyPress = 1; keyvalue = 1;}	  //���ü�ֵ1
        else if ((Key_UP)&&(keyUP==0)) {FlagKeyPress = 1; keyvalue = 2;}  //��  ��ֵ2
        else if ((Key_DOWN)&&(keyDOWN==0)) {FlagKeyPress = 1; keyvalue = 3;}//�¼�ֵ3
        else if ((Key_SAVE)&&(keySAVE==0)) {FlagKeyPress = 1; keyvalue = 4;}//�����ֵ4
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