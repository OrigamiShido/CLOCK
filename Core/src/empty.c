#include "ti_msp_dl_config.h"
#include "oled.h"
#include <eeprom_emulation_type_a.h>

//DEFINE
#define HOUR 0
#define MINUTE 1
#define SECOND 2
#define YEAR 3
#define MONTH 4
#define DAY 5
#define ADDRESS 0x9000

//OLED PARAMETER
uint8_t TxPacket[4] = {0x90, 0x00, 0x00, 0x00};  //��������
uint8_t RxPacket[4]={0x00, 0x00, 0x00, 0x00};   //��������
uint8_t RxTemp; //��ʱ���ݣ���ս���FIFO��

uint8_t hour=0;
uint8_t minute=0;
uint8_t second=0;

uint32_t year=2024;
uint8_t month=2;
uint8_t day=1;

uint32_t dataarray[EEPROM_EMULATION_DATA_SIZE / sizeof(uint32_t)]={0};
uint32_t EEPROMEmulationBuffer[EEPROM_EMULATION_DATA_SIZE / sizeof(uint32_t)]={0};

bool ischanged=false;

int countweek(uint32_t year,uint8_t month,uint8_t day);
int scan(void);
int debunce(uint32_t inputpin, uint32_t control);
int digitalcount(double input);
void showtime(uint8_t showhour,uint8_t showminute,uint8_t showsecond,uint32_t showyear,uint8_t showmonth,uint8_t showday,unsigned int showweek);
void DisplaySettings(void);
bool Settime(void);
bool Setdate(void);
int setparameter(int maxcount,int parameter,int number);

int main(void)
{
	unsigned int status=114514;
	unsigned int substatus=114514;
	unsigned int lastnumber=0;
	unsigned int sublastnumber=0;
	double result=0;
	bool isset=false;
	bool isclear=false;
	uint32_t EEPROMEmulationState;
	uint8_t week=9;
	SYSCFG_DL_init();                      //��ʼ��
	//	NVIC_EnableIRQ(ADC0_INT_IRQn);         //??ADC??
	//	gCheckADC = false;                     //???ADC???????
	//NVIC_EnableIRQ(MATRIX_INT_IRQN);
	//timer enabler
	DL_TimerG_startCounter(TIMER_0_INST);
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
	//OLED self test
	OLED_Init();			//��ʼ��OLED  
	OLED_Clear();

	EEPROM_TypeA_eraseAllSectors();
    EEPROMEmulationState = EEPROM_TypeA_init(&EEPROMEmulationBuffer[0]);
	
	while (1) 
	{
		
		DL_UART_Main_transmitDataBlocking(UART1, hour);
		DL_UART_Main_transmitDataBlocking(UART1, minute);
		DL_UART_Main_transmitDataBlocking(UART1, second);
		DL_UART_Main_transmitDataBlocking(UART1, '\n');
						
		week=countweek(year,month,day);

		if(ischanged)
		{
			showtime(hour,minute,second,year,month,day,week);
			ischanged=false;
		}
		status=scan();
		if(status!=114514)
		{
			if(lastnumber!=status)
			{
				switch(status)
				{
					case 12:
					while(1)
					{
						if(!isclear)
						{
							OLED_Clear();
							DisplaySettings();
							isclear=true;
						}
						substatus=scan();
						if(substatus!=114514)
						{
							if(sublastnumber!=substatus)
							{
								switch(substatus)
								{
									case 1:
									isclear=Settime();
									break;	
									case 2:
									isclear=Setdate();
									break;
									case 3:isset=true;
									isclear=false;
									break;
									default:break;
								}
							}
						}
						sublastnumber=substatus;
						if(isset)
						{
							isset=false;
							break;
						}
					}
					break;
					case 13:break;
					case 14:break;
					case 15:break;
					defualt:break;
				}
			}
		}
		else
		{
			;
		}
		lastnumber=status;
    }
}

void TIMER_0_INST_IRQHandler (void){
	DL_GPIO_togglePins(LEDLIGHTS_PORT, LEDLIGHTS_LEDlight_PIN);
	second++;
	if(second>=60)
	{
		second=0;
		minute++;
	}
	if(minute>=60)
	{
		minute=0;
		hour++;
	}
	if(hour>=24)
	{
		hour=0;
		day++;
	}
	switch(day)
	{
		case 29:if((year%4==0&&year%100!=0)||(year%400==0))
					{
						if(month==2)
						break;
					}
				else
				{
					if(month==2)
					{
						day=1;
						month++;
					}
				}
		case 30:if((year%4==0&&year%100!=0)||year%400==0)
				{
					if(month==2)
					{
						day=1;
						month++;
					}
				}
				else break;
		case 31:if(month==4||month==6||month==9||month==11)
				{
					day=1;
					month++;
				}
				else break;
		case 32:if(month==1||month==3||month==5||month==7||month==8||month==10||month==12)
				{
					day=1;
					month++;
				}
				else break;
		default:break;
	}
	if(month==13)
	{
		month=1;
		year++;
	}
	if(year>=10000)
	{
		year=0;
	}
	ischanged=true;
}

int countweek(uint32_t countyear,uint8_t countmonth,uint8_t countday)
{
	uint32_t tempyear=countyear;
	uint8_t tempmonth=countmonth;
	uint8_t tempday=countday;
	if(countmonth==1||countmonth==2)
	{
		tempmonth+=12;
		tempyear--;
	}
	return (tempday+2*tempmonth+3*(tempmonth+1)/5+tempyear+tempyear/4-tempyear/100+tempyear/400)%7;
}

void showtime(uint8_t showhour,uint8_t showminute,uint8_t showsecond,uint32_t showyear,uint8_t showmonth,uint8_t showday,unsigned int showweek)
{
	unsigned int yeardigital=digitalcount(showyear);
	OLED_Clear();
	
	switch(showhour)
	{
		case 0:OLED_ShowString(0,0,"00:");break;
		case 1:OLED_ShowString(0,0,"01:");break;
		case 2:OLED_ShowString(0,0,"02:");break;
		case 3:OLED_ShowString(0,0,"03:");break;
		case 4:OLED_ShowString(0,0,"04:");break;
		case 5:OLED_ShowString(0,0,"05:");break;
		case 6:OLED_ShowString(0,0,"06:");break;
		case 7:OLED_ShowString(0,0,"07:");break;
		case 8:OLED_ShowString(0,0,"08:");break;
		case 9:OLED_ShowString(0,0,"09:");break;
		case 10:OLED_ShowString(0,0,"10:");break;
		case 11:OLED_ShowString(0,0,"11:");break;
		case 12:OLED_ShowString(0,0,"12:");break;
		case 13:OLED_ShowString(0,0,"13:");break;
		case 14:OLED_ShowString(0,0,"14:");break;
		case 15:OLED_ShowString(0,0,"15:");break;
		case 16:OLED_ShowString(0,0,"16:");break;
		case 17:OLED_ShowString(0,0,"17:");break;
		case 18:OLED_ShowString(0,0,"18:");break;
		case 19:OLED_ShowString(0,0,"19:");break;
		case 20:OLED_ShowString(0,0,"20:");break;
		case 21:OLED_ShowString(0,0,"21:");break;
		case 22:OLED_ShowString(0,0,"22:");break;
		case 23:OLED_ShowString(0,0,"23:");break;
	}
	switch(showminute)
	{
		case 0:OLED_ShowString(24,0,"00:");break;
		case 1:OLED_ShowString(24,0,"01:");break;
		case 2:OLED_ShowString(24,0,"02:");break;
		case 3:OLED_ShowString(24,0,"03:");break;
		case 4:OLED_ShowString(24,0,"04:");break;
		case 5:OLED_ShowString(24,0,"05:");break;
		case 6:OLED_ShowString(24,0,"06:");break;
		case 7:OLED_ShowString(24,0,"07:");break;
		case 8:OLED_ShowString(24,0,"08:");break;
		case 9:OLED_ShowString(24,0,"09:");break;
		case 10:OLED_ShowString(24,0,"10:");break;
		case 11:OLED_ShowString(24,0,"11:");break;
		case 12:OLED_ShowString(24,0,"12:");break;
		case 13:OLED_ShowString(24,0,"13:");break;
		case 14:OLED_ShowString(24,0,"14:");break;
		case 15:OLED_ShowString(24,0,"15:");break;
		case 16:OLED_ShowString(24,0,"16:");break;
		case 17:OLED_ShowString(24,0,"17:");break;
		case 18:OLED_ShowString(24,0,"18:");break;
		case 19:OLED_ShowString(24,0,"19:");break;
		case 20:OLED_ShowString(24,0,"20:");break;
		case 21:OLED_ShowString(24,0,"21:");break;
		case 22:OLED_ShowString(24,0,"22:");break;
		case 23:OLED_ShowString(24,0,"23:");break;
		case 24:OLED_ShowString(24,0,"24:");break;
		case 25:OLED_ShowString(24,0,"25:");break;
		case 26:OLED_ShowString(24,0,"26:");break;
		case 27:OLED_ShowString(24,0,"27:");break;
		case 28:OLED_ShowString(24,0,"28:");break;
		case 29:OLED_ShowString(24,0,"29:");break;
		case 30:OLED_ShowString(24,0,"30:");break;
		case 31:OLED_ShowString(24,0,"31:");break;
		case 32:OLED_ShowString(24,0,"32:");break;
		case 33:OLED_ShowString(24,0,"33:");break;
		case 34:OLED_ShowString(24,0,"34:");break;
		case 35:OLED_ShowString(24,0,"35:");break;
		case 36:OLED_ShowString(24,0,"36:");break;
		case 37:OLED_ShowString(24,0,"37:");break;
		case 38:OLED_ShowString(24,0,"38:");break;
		case 39:OLED_ShowString(24,0,"39:");break;
		case 40:OLED_ShowString(24,0,"40:");break;
		case 41:OLED_ShowString(24,0,"41:");break;
		case 42:OLED_ShowString(24,0,"42:");break;
		case 43:OLED_ShowString(24,0,"43:");break;
		case 44:OLED_ShowString(24,0,"44:");break;
		case 45:OLED_ShowString(24,0,"45:");break;
		case 46:OLED_ShowString(24,0,"46:");break;
		case 47:OLED_ShowString(24,0,"47:");break;
		case 48:OLED_ShowString(24,0,"48:");break;
		case 49:OLED_ShowString(24,0,"49:");break;
		case 50:OLED_ShowString(24,0,"50:");break;
		case 51:OLED_ShowString(24,0,"51:");break;
		case 52:OLED_ShowString(24,0,"52:");break;
		case 53:OLED_ShowString(24,0,"53:");break;
		case 54:OLED_ShowString(24,0,"54:");break;
		case 55:OLED_ShowString(24,0,"55:");break;
		case 56:OLED_ShowString(24,0,"56:");break;
		case 57:OLED_ShowString(24,0,"57:");break;
		case 58:OLED_ShowString(24,0,"58:");break;
		case 59:OLED_ShowString(24,0,"59:");break;
	}
	switch(showsecond)
	{
		case 0:OLED_ShowString(48,0,"00");break;
		case 1:OLED_ShowString(48,0,"01");break;
		case 2:OLED_ShowString(48,0,"02");break;
		case 3:OLED_ShowString(48,0,"03");break;
		case 4:OLED_ShowString(48,0,"04");break;
		case 5:OLED_ShowString(48,0,"05");break;
		case 6:OLED_ShowString(48,0,"06");break;
		case 7:OLED_ShowString(48,0,"07");break;
		case 8:OLED_ShowString(48,0,"08");break;
		case 9:OLED_ShowString(48,0,"09");break;
		case 10:OLED_ShowString(48,0,"10");break;
		case 11:OLED_ShowString(48,0,"11");break;
		case 12:OLED_ShowString(48,0,"12");break;
		case 13:OLED_ShowString(48,0,"13");break;
		case 14:OLED_ShowString(48,0,"14");break;
		case 15:OLED_ShowString(48,0,"15");break;
		case 16:OLED_ShowString(48,0,"16");break;
		case 17:OLED_ShowString(48,0,"17");break;
		case 18:OLED_ShowString(48,0,"18");break;
		case 19:OLED_ShowString(48,0,"19");break;
		case 20:OLED_ShowString(48,0,"20");break;
		case 21:OLED_ShowString(48,0,"21");break;
		case 22:OLED_ShowString(48,0,"22");break;
		case 23:OLED_ShowString(48,0,"23");break;
		case 24:OLED_ShowString(48,0,"24");break;
		case 25:OLED_ShowString(48,0,"25");break;
		case 26:OLED_ShowString(48,0,"26");break;
		case 27:OLED_ShowString(48,0,"27");break;
		case 28:OLED_ShowString(48,0,"28");break;
		case 29:OLED_ShowString(48,0,"29");break;
		case 30:OLED_ShowString(48,0,"30");break;
		case 31:OLED_ShowString(48,0,"31");break;
		case 32:OLED_ShowString(48,0,"32");break;
		case 33:OLED_ShowString(48,0,"33");break;
		case 34:OLED_ShowString(48,0,"34");break;
		case 35:OLED_ShowString(48,0,"35");break;
		case 36:OLED_ShowString(48,0,"36");break;
		case 37:OLED_ShowString(48,0,"37");break;
		case 38:OLED_ShowString(48,0,"38");break;
		case 39:OLED_ShowString(48,0,"39");break;
		case 40:OLED_ShowString(48,0,"40");break;
		case 41:OLED_ShowString(48,0,"41");break;
		case 42:OLED_ShowString(48,0,"42");break;
		case 43:OLED_ShowString(48,0,"43");break;
		case 44:OLED_ShowString(48,0,"44");break;
		case 45:OLED_ShowString(48,0,"45");break;
		case 46:OLED_ShowString(48,0,"46");break;
		case 47:OLED_ShowString(48,0,"47");break;
		case 48:OLED_ShowString(48,0,"48");break;
		case 49:OLED_ShowString(48,0,"49");break;
		case 50:OLED_ShowString(48,0,"50");break;
		case 51:OLED_ShowString(48,0,"51");break;
		case 52:OLED_ShowString(48,0,"52");break;
		case 53:OLED_ShowString(48,0,"53");break;
		case 54:OLED_ShowString(48,0,"54");break;
		case 55:OLED_ShowString(48,0,"55");break;
		case 56:OLED_ShowString(48,0,"56");break;
		case 57:OLED_ShowString(48,0,"57");break;
		case 58:OLED_ShowString(48,0,"58");break;
		case 59:OLED_ShowString(48,0,"59");break;
	}

	switch(yeardigital)
	{
		case 1:OLED_ShowNum(24,2,showyear,1,18);break;
		case 2:OLED_ShowNum(16,2,showyear,2,18);break;
		case 3:OLED_ShowNum(8,2,showyear,3,18);break;
		case 4:OLED_ShowNum(0,2,showyear,4,18);break;
		default:break;
	}
	OLED_ShowString(32,2,"/");

	if(showmonth<10)
	{
		OLED_ShowNum(40,2,0,1,18);
		OLED_ShowNum(48,2,showmonth,1,18);
	}
	else
	{
		OLED_ShowNum(40,2,showmonth,2,18);
	}
	OLED_ShowString(56,2,"/");
	if(showday<10)
	{
		OLED_ShowNum(64,2,0,1,18);
		OLED_ShowNum(72,2,showday,1,18);
	}
	else
	{
		OLED_ShowNum(64,2,showday,2,18);
	}

	switch(showweek)
	{
		case 0:OLED_ShowString(2,4,"Mon");break;
		case 1:OLED_ShowString(2,4,"Tue");break;
		case 2:OLED_ShowString(2,4,"Wed");break;
		case 3:OLED_ShowString(2,4,"Thu");break;
		case 4:OLED_ShowString(2,4,"Fri");break;
		case 5:OLED_ShowString(2,4,"Sat");break;
		case 6:OLED_ShowString(2,4,"Sun");break;
		default:break;
	}
}

void DisplaySettings(void)
{
	OLED_Clear();
	OLED_ShowString(0,0,"Settings");
	OLED_ShowString(0,2,"1. Set Time");
	OLED_ShowString(0,4,"2. Set Date");
	OLED_ShowString(0,6,"3. Exit");
}

bool Settime(void)
{
	unsigned int yscale=0;
	unsigned int status=114514;
	unsigned int lastnumber=1;
	unsigned int sethour=0;
	unsigned int setminute=0;
	unsigned int setsecond=0;
	unsigned int settingparameter=HOUR;
	bool iscanceled=false;
	bool isset=false;
	bool isbackspaced=true;
	while(1)
	{
		if(isbackspaced)
		{
			OLED_Clear();
			OLED_ShowString(0,0,"Please input:");
			if(sethour!=0)
			{
				if(sethour>=10)
				{
					OLED_ShowNum(0,2,sethour,2,18);
				}
				else
				{
					OLED_ShowNum(0,2,sethour,1,18);
				}
			}
			OLED_ShowString(16,2,":");
			if(setminute!=0)
			{
				if(setminute>=10)
				{
					OLED_ShowNum(24,2,setminute,2,18);
				}
				else
				{
					OLED_ShowNum(24,2,setminute,1,18);
				}
			}
			OLED_ShowString(40,2,":");
			if(setsecond!=0)
			{
				if(setsecond>=10)
				{
					OLED_ShowNum(48,2,setsecond,2,18);
				}
				else
				{
					OLED_ShowNum(48,2,setsecond,1,18);
				}
			}
			isbackspaced=false;
		}
		status=scan();
		if(status!=114514)
		{
			if(status!=lastnumber)
			{
				switch(status)
				{
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:switch(settingparameter)
					{
						case HOUR:sethour=setparameter(2,sethour,status);
						if(yscale==0||yscale==8)
						{
							OLED_ShowNum(yscale,2,status,1,18);
							yscale+=8;
						}
						break;
						case MINUTE:setminute=setparameter(2,setminute,status);
						if(yscale==24||yscale==32)
						{
							OLED_ShowNum(yscale,2,status,1,18);
							yscale+=8;
						}
						break;
						case SECOND:setsecond=setparameter(2,setsecond,status);
						if(yscale==48||yscale==56)
						{
							OLED_ShowNum(yscale,2,status,1,18);
							yscale+=8;
						}
						break;
					}break;
					case 10:iscanceled=true;break;//取消
					case 11:isset=true;break;//确认
					case 12:settingparameter=HOUR;
						if(sethour==0)
							yscale=0;
						else if(sethour<10)
							yscale=8;
						else
							yscale=16;
						break;//时
					case 13:settingparameter=MINUTE;
						if(setminute==0)
							yscale=24;
						else if(setminute<10)
							yscale=32;
						else
							yscale=40;
						break;//分
					case 14:settingparameter=SECOND;
						if(setsecond==0)
							yscale=48;
						else if(setsecond<10)
							yscale=56;
						else
							yscale=64;
						break;//秒
					case 15:switch(settingparameter)
					{
						case HOUR:sethour/=10;
						yscale-=8;
						isbackspaced=true;
						break;
						case MINUTE:setminute/=10;
						yscale-=8;
						isbackspaced=true;
						break;
						case SECOND:setsecond/=10;
						yscale-=8;
						isbackspaced=true;
						break;
					}//退格
					default:break;
				}
			}
		}
		lastnumber=status;
		if(iscanceled)
		{
			break;
		}
		if(isset)
		{
			if(sethour>=0&&sethour<=23&&setminute>=0&&setminute<=59&&setsecond>=0&&setsecond<=59)
			{
				hour=sethour;
				minute=setminute;
				second=setsecond;
				ischanged=true;
				break;
			}
			else
			{
				OLED_Clear();
				OLED_ShowString(0,0,"Invalid input!");
				delay_cycles(120000000);//may cause error.
				ischanged=true;
				break;
			}
		}
	}
	return false;
}

bool Setdate(void)
{
	unsigned int yscale=0;
	unsigned int status=114514;
	unsigned int lastnumber=2;
	unsigned int settingparameter=YEAR;
	unsigned int setyear=0;
	unsigned int setmonth=0;
	unsigned int setday=0;
	bool iscanceled=false;
	bool isset=false;
	bool isbackspaced=true;
	while(1)
	{
		if(isbackspaced)
		{
			OLED_Clear();
			OLED_ShowString(0,0,"Please input:");
			if(setyear!=0)
			{
				if(setyear>=1000)
				{
					OLED_ShowNum(0,2,setyear,4,18);
				}
				else if(setyear>=100)
				{
					OLED_ShowNum(0,2,setyear,3,18);
				}
				else if(setyear>=10)
				{
					OLED_ShowNum(0,2,setyear,2,18);
				}
				else
				{
					OLED_ShowNum(0,2,setyear,1,18);
				}
			}
			OLED_ShowString(32,2,"/");
			if(setmonth!=0)
			{
				if(setmonth>=10)
				{
					OLED_ShowNum(40,2,setmonth,2,18);
				}
				else
				{
					OLED_ShowNum(40,2,setmonth,1,18);
				}
			}
			OLED_ShowString(56,2,"/");
			if(setday!=0)
			{
				if(setday>=10)
				{
					OLED_ShowNum(64,2,setday,2,18);
				}
				else
				{
					OLED_ShowNum(64,2,setday,1,18);
				}
			}
			isbackspaced=false;
		}
		status=scan();
		if(status!=114514)
		{
			if(status!=lastnumber)
			{
				switch(status)
				{
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:switch(settingparameter)
					{
						case YEAR:setyear=setparameter(4,setyear,status);
						if(yscale==0||yscale==8||yscale==16||yscale==24)
						{
							if(yscale==0)
							{
								OLED_ShowNum(0,2,status,1,18);
								yscale+=8;
							}
							else
							{
								OLED_ShowNum(yscale,2,status,1,18);
								yscale+=8;
							}
						}
						break;
						case MONTH:setmonth=setparameter(2,setmonth,status);
						if(yscale==40||yscale==48)
						{
							OLED_ShowNum(yscale,2,status,1,18);
							yscale+=8;
						}
						break;
						case DAY:setday=setparameter(2,setday,status);
						if(yscale==64||yscale==72)
						{
							OLED_ShowNum(yscale,2,status,1,18);
							yscale+=8;
						}
						break;
					}break;
					case 10:iscanceled=true;break;//取消
					case 11:isset=true;break;//确认
					case 12:settingparameter=YEAR;
					if(setyear==0)
						yscale=0;
					else if(setyear<10)
						yscale=8;
					else if(setyear<100)
						yscale=16;
					else
						yscale=24;
					break;//年
					case 13:settingparameter=MONTH;
					if(setmonth==0)
						yscale=40;
					else if(setmonth<10)
						yscale=48;
					break;//月
					case 14:settingparameter=DAY;
					if(setday==0)
						yscale=64;
					else if(setday<10)
						yscale=72;
					break;//日
					case 15:switch(settingparameter)
					{
						case YEAR:setyear/=10;
						if(yscale==8||yscale==16||yscale==24)
						{
							yscale-=8;
							isbackspaced=true;
						}
						else if(yscale==0)
						{
							isbackspaced=true;
						}
						break;
						case MONTH:setmonth/=10;
						if(yscale==48)
						{
							yscale-=8;
							isbackspaced=true;
						}
						else if(yscale==40)
						{
							isbackspaced=true;
						}
						break;
						case DAY:setday/=10;
						if(yscale==72)
						{
							yscale-=8;
							isbackspaced=true;
						}
						else if(yscale==64)
						{
							isbackspaced=true;
						}
						break;
					}//退格
					default:break;
				}
			}
		}
		lastnumber=status;
		if(iscanceled)
		{
			break;
		}
		if(isset)
		{
			if(setyear>=0&&setyear<=9999&&setmonth>=1&&setmonth<=12&&setday>=1&&setday<=31)
			{
				if((setmonth==4||setmonth==6||setmonth==9||setmonth==11)&&setday==31)
				{
					OLED_Clear();
					OLED_ShowString(0,0,"Invalid input!");
					delay_cycles(120000000);//may cause error.
					ischanged=true;
					break;
				}
				if(setmonth==2)
				{
					if((setyear%4==0&&setyear%100!=0)||(setyear%400==0))
					{
						if(setday>29)
						{
							OLED_Clear();
							OLED_ShowString(0,0,"Invalid input!");
							delay_cycles(120000000);//may cause error.
							ischanged=true;
							break;
						}
					}
					else
					{
						if(setday>28)
						{
							OLED_Clear();
							OLED_ShowString(0,0,"Invalid input!");
							delay_cycles(120000000);//may cause error.
							ischanged=true;
							break;
						}
					}
				}
				year=setyear;
				month=setmonth;
				day=setday;
				ischanged=true;
				break;
			}
			else
			{
				OLED_Clear();
				OLED_ShowString(0,0,"Invalid input!");
				delay_cycles(120000000);//may cause error.
				ischanged=true;
				break;
			}
		}
	}
	return false;
}

int setparameter(int maxcount,int parameter,int number)
{
	int count=digitalcount(parameter);
	if(count>=maxcount)
		return parameter;
	else
	{
		parameter=parameter*10+number;
		return parameter;
	}
}

int digitalcount(double input)
{
	int object=(int)input;
	int result=1;
	for(int i=10;;i=i*10)
	{
		if(object/i!=0)
		result++;
		else break;
	}
	//if(result==1)result=2;
	return result;
}

int scan(void)
{
	int num=114514;
					DL_GPIO_clearPins(MATRIX_PORT,MATRIX_V1_PIN);
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V2_PIN);
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V3_PIN);
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V4_PIN);
						if(debunce(MATRIX_H1_PIN,0))
						{
							num=7;
						}
						if(debunce(MATRIX_H2_PIN,1))
						{
							num=4;
						}
						if(debunce(MATRIX_H3_PIN,0))
						{
							num=1;
						}
						if(debunce(MATRIX_H4_PIN,0))
						{
							num=10;
						}
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V1_PIN);
					DL_GPIO_clearPins(MATRIX_PORT,MATRIX_V2_PIN);
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V3_PIN);
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V4_PIN);
						if(debunce(MATRIX_H1_PIN,0))
						{
							num=8;
						}
						if(debunce(MATRIX_H2_PIN,1))
						{
							num=5;
						}
						if(debunce(MATRIX_H3_PIN,0))
						{
							num=2;
						}
						if(debunce(MATRIX_H4_PIN,0))
						{
							num=0;
						}
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V1_PIN);
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V2_PIN);
					DL_GPIO_clearPins(MATRIX_PORT,MATRIX_V3_PIN);
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V4_PIN);
    
						if(debunce(MATRIX_H1_PIN,0))
						{
							num=9;
						}
						if(debunce(MATRIX_H2_PIN,1))
						{
							num=6;
						}
						if(debunce(MATRIX_H3_PIN,0))
						{
							num=3;
						}
						if(debunce(MATRIX_H4_PIN,0))
						{
							num=11;
						}
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V1_PIN);
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V2_PIN);
					DL_GPIO_setPins(MATRIX_PORT,MATRIX_V3_PIN);
					DL_GPIO_clearPins(MATRIX_PORT,MATRIX_V4_PIN);
						if(debunce(MATRIX_H1_PIN,0))
						{
							num=12;
						}
						if(debunce(MATRIX_H2_PIN,1))
						{
							num=13;
						}
						if(debunce(MATRIX_H3_PIN,0))
						{
							num=14;
						}
						if(debunce(MATRIX_H4_PIN,0))
						{
							num=15;
						}
						return num;
}

int debunce(uint32_t inputpin, uint32_t control)
{
	if(!control){
	if(!(DL_GPIO_readPins(MATRIX_PORT, inputpin)))
	{
	for(uint32_t i=0;i<=20000;i++){}
	if(!(DL_GPIO_readPins(MATRIX_PORT, inputpin)))
		return 1;
	else 
		return 0;
	}
	else return 0;
}
	
	else {
		for(uint32_t i=0;i<=10000;i++){}
	if(!(DL_GPIO_readPins(MATRIX_PORT, inputpin)))
	{
	for(uint32_t i=0;i<=20000;i++){}
	if(!(DL_GPIO_readPins(MATRIX_PORT, inputpin)))
		return 1;
	else 
		return 0;
	}
	else return 0;
	}
}