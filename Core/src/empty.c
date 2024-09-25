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

#define CLOCK 6
#define ALARM1 7
#define ALARM2 8
#define ALARM3 9

#define ADDRESS 0x9000
#define ALARMADDRESS 0x9018
#define ALARMADDRESS2 0x902c
#define ALARMADDRESS3 0x9044

#define C1 262
#define C1U 277
#define D1 294
#define D1U 311
#define E1 330
#define F1 349
#define F1U 370
#define G1 392
#define G1U 415
#define A1 440
#define A1U 466
#define B1 494
#define C2 523
#define C2U 554
#define D2 587
#define D2U 622
#define E2 659
#define F2 698
#define F2U 740
#define G2 784
#define G2U 831
#define A2 880
#define A2U 932
#define B2 988
#define C3 1046
#define C3U 1109
#define D3 1175
#define D3U 1245
#define E3 1318
#define F3 1397
#define F3U 1480
#define G3 1568
#define G3U 1661
#define A3 1760
#define A3U 1865
#define B3 1976

//OLED PARAMETER
uint8_t TxPacket[4] = {0x90, 0x00, 0x00, 0x00};  //��������
uint8_t RxPacket[4]={0x00, 0x00, 0x00, 0x00};   //��������
uint8_t RxTemp; //��ʱ���ݣ���ս���FIFO��

uint8_t data=0;
uint8_t databuff[20]={0};
uint8_t idx=0;

struct Time{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

struct Date{
	uint32_t year;
	uint8_t month;
	uint8_t day;
	struct Time time;
};

struct Alarm{
	struct Time time;
  bool ison;
};

struct Date date={2024,2,1,0,0,0};
struct Alarm alarms[3]={{0,0,0,true},{0,0,0,false},{0,0,0,false}};
uint32_t EEPROMEmulationBuffer[EEPROM_EMULATION_DATA_SIZE / sizeof(uint32_t)]={0};

bool ischanged=false;
bool isticked=false;
uint8_t timersecond=60;
uint8_t timerminute=60;
uint8_t timerhour=24;

const uint16_t month_days_table[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};

int countweek(uint32_t year,uint8_t month,uint8_t day);
int scan(void);
int debunce(uint32_t inputpin, uint32_t control);
int digitalcount(double input);
void showtime(uint8_t showhour,uint8_t showminute,uint8_t showsecond,uint32_t showyear,uint8_t showmonth,uint8_t showday,unsigned int showweek);
void showtimein12(uint8_t showhour,uint8_t showminute,uint8_t showsecond,uint32_t showyear,uint8_t showmonth,uint8_t showday,unsigned int showweek);
void DisplaySettings(void);
struct Time Settime(struct Time original,uint8_t num);
void Setdate(void);
int setparameter(int maxcount,int parameter,int number);
struct Date judge(struct Date judgedate);
void save(struct Date savedate,struct Alarm savealarm[3]);
struct Date read(uint8_t mode);
bool validate(struct Date target);
void DisplayFunctions(void);

//new functions
void DisplayCounter(void);
bool CheckAlarm(struct Date target,struct Alarm alarms[3]);
void Beep(struct Date target);
void readalarm(struct Alarm* target);
bool validatealarm(struct Alarm target);
void DisplayAlarm(struct Alarm* alarms);
void showtimesimplified(int x, int y, uint8_t showhour,uint8_t showminute,uint8_t showsecond);
bool CompareAlarm(struct Alarm target1,struct Alarm target2);
void Buzz(unsigned int frequency, unsigned int duration);

void transmittophone(float curve ,float thd, float u[5]);
void order(void);
void transmittime(uint8_t mode);
void transmitclock(uint8_t mode,uint8_t idx);
void blesettime(uint8_t mode);
void blesetclock(uint8_t mode,uint8_t idx);
void datetostamp(void);
char* itoa(int num,char* str,int radix);
void transmitstring(char* p);
bool leapyear(uint32_t year);

int main(void)
{
	//VARIABLES
	unsigned int status=114514;
	unsigned int lastnumber=0;

	bool isset=false;
	bool isclear=false;
	bool is24hour=true;

	uint32_t EEPROMEmulationState;

	uint8_t week=9;
	
	struct Date breakpoint={0,0,0,0,0,0};
	struct Alarm breakalarms[3];
	//struct Alarm alarms[3]={{0,0,0,false},{0,0,0,false},{0,0,0,false}};
	// struct Alarm alarms[3]={{0,0,0,true},{0,0,0,false},{0,0,0,false}};
	SYSCFG_DL_init();
	//timer enabler
	DL_TimerG_startCounter(TIMER_0_INST);
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
	NVIC_ClearPendingIRQ(UART0_INT_IRQn);
	NVIC_EnableIRQ(UART0_INT_IRQn);
	//OLED self test
	OLED_Init();
	OLED_Clear();

	//EEPROM_TypeA_eraseAllSectors();
    //EEPROMEmulationState = EEPROM_TypeA_init(&EEPROMEmulationBuffer[0]);

	breakpoint=read(CLOCK);
	if(validate(breakpoint))
	{
		date=breakpoint;
		date.time.second--;
	}
	for(uint8_t i=0;i<3;i++)
	{
	  readalarm(breakalarms);
	  if(validatealarm(breakalarms[i]))
	  {
		alarms[i]=breakalarms[i];
	  }
	}
	while (1) 
	{
		week=countweek(date.year,date.month,date.day);
		if(ischanged)
		{
			if(is24hour)
				showtime(date.time.hour,date.time.minute,date.time.second,date.year,date.month,date.day,week);
			else
				showtimein12(date.time.hour,date.time.minute,date.time.second,date.year,date.month,date.day,week);
			// save(date,alarms);
			ischanged=false;
			// if(CheckAlarm(date,alarms))
			// {
			//   Beep(date);
			// }
		}
		status=scan();
		if(status!=114514)
		{
			if(lastnumber!=status)
			{
				switch(status)
				{
					case 12://时钟设置子菜单
					while(1)
					{
						if(!isclear)
						{
							OLED_Clear();
							DisplaySettings();
							isclear=true;
						}
						status=scan();
						if(status!=114514)
						{
							if(lastnumber!=status)
							{
								switch(status)
								{
									case 1:
									isclear=false;
									Setdate();
									break;	
									case 2:
									isclear=false;
									date.time=Settime(date.time,2);
									break;
									case 3:isset=true;
									isclear=false;
									OLED_Clear();
									break;
									default:break;
								}
							}
						}
						lastnumber=status;
						if(isset)
						{
							isset=false;
							break;
						}
					}
					break;
					case 13://24小时制切换
					is24hour=!is24hour;
					OLED_Clear();
					if(is24hour)
						showtime(date.time.hour,date.time.minute,date.time.second,date.year,date.month,date.day,week);
					else
						showtimein12(date.time.hour,date.time.minute,date.time.second,date.year,date.month,date.day,week);
					break;
					case 14://额外功能子菜单：闹钟，计时器
					while(1)
					{
						if(!isclear)
						{
							OLED_Clear();
							DisplayFunctions();
							isclear=true;
						}
						status=scan();
						if(status!=114514)
						{
							if(lastnumber!=status)
							{
								switch(status)
								{
									case 1:
									isclear=false;
									DisplayAlarm(alarms);
									break;	
									case 2:
									isclear=false;
									DisplayCounter();
									break;
									case 3:isset=true;
									isclear=false;
									OLED_Clear();
									break;
									default:break;
								}
							}
						}
						lastnumber=status;
						if(isset)
						{
							isset=false;
							break;
						}
					}
					break;
					case 15:
						isclear=false;
						DisplayCounter();
					break;
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

void UART0_IRQHandler()
{
	switch(DL_UART_Main_getPendingInterrupt(UART0))
	{
		case DL_UART_MAIN_IIDX_RX:
			data=DL_UART_Main_receiveData(UART0);
			databuff[idx]=data;
			idx++;
			if(data=='\n')
			{
				for(uint8_t i=0;i<idx;i++)
				{
					databuff[i]=0;
				}
				idx=0;
				order();
			}
		break;
		// case DL_UART_MAIN_IIDX_TX:
		// break;
		default:break;
	}
}

void TIMER_0_INST_IRQHandler (void){
	DL_GPIO_togglePins(LEDLIGHTS_PORT, LEDLIGHTS_LEDlight_PIN);
	date.time.second++;
	date=judge(date);
	//transmit(date);
	save(date,alarms);
		if(CheckAlarm(date,alarms))
	{
	  Beep(date);
	}
	ischanged=true;
}

void TIMER_1_INST_IRQHandler (void){
	DL_GPIO_togglePins(LEDLIGHTS_PORT, LEDLIGHTS_LEDlight2_PIN);
	isticked=true;
	if(timersecond==0)
	{	
		if(timerminute==0)
		{
			if(timerhour==0)
			{
				return;
			}
			else
			{
				timerhour--;
				timerminute+=60;
			}
		}
		timersecond+=60;
		timerminute--;
	}
	timersecond--;
}

void order(void)
{
	switch(databuff[0])
	{
		case '?'://询问指令：?t*,?t#,?c*1,?c*2,?c*3,?c*a
			switch(databuff[2])
			{
				case 't':transmittime(databuff[1]);break;
				case 'c':transmitclock(databuff[1],databuff[3]);break;
			}
		break;
		case 's'://设置指令：st*,st#,sc*1,sc*2,sc*3,sc1y,sc1n,sc2y,sc2n,sc3y,sc3n/yyyymmddhhmmss
			switch(databuff[2])
			{
				case 't':blesettime(databuff);break;
				case 'c':blesetclock(databuff);break;
			}
		break;
		default:break;
	}
}

void transmittime(uint8_t mode)
{
	uint8_t* p=NULL;
	uint32_t stamp=0;
	switch(mode)
	{
		case '*':
			bledate(date);//发送时间
		break;//正常日期
		case '#':
			stamp=timetostamp(date);
			p=(uint8_t*)&stamp;
			for(uint8_t i=0;i<4;i++)
			{
				DL_UART_transmitDataBlocking(UART0,*(p+i));
			}
			DL_UART_transmitDataBlocking(UART0,'\n');
		break;//时间戳
	}
}

void transmitclock(uint8_t mode,uint8_t idx)
{
	switch(mode)
	{
		case '*':
			if(idx='a')
			{
				DL_UART_transmitDataBlocking(UART0,(alarms[0].ison)?'Y':'N');
				bletime(alarms[0].time);
				DL_UART_transmitDataBlocking(UART0,(alarms[1].ison)?'Y':'N');
				bletime(alarms[1].time);
				DL_UART_transmitDataBlocking(UART0,(alarms[2].ison)?'Y':'N');
				bletime(alarms[2].time);
			}
			else if(idx>='0'&&idx<='5')
			{
				DL_UART_transmitDataBlocking(UART0,(alarms[idx-'0'-1].ison)?'Y':'N');
				bletime(alarms[idx-'0'-1].time);
			}
		break;//发送闹钟
		default:break;
	}
}

void blesettime(char* datas)
{
	uint32_t year=0;
	uint8_t month=0;
	uint8_t day=0;
	uint8_t hour=0;
	uint8_t minute=0;
	uint8_t second=0;
	uint32_t stamp=0;
	struct Date temp={0,0,0,0,0,0};
	switch(datas[2])
	{
		case '*':
			year=(uint32_t)(datas[3]-'0')*1000+(uint32_t)(datas[4]-'0')*100+(uint32_t)(datas[5]-'0')*10+(uint32_t)(datas[6]-'0');
			month=(uint8_t)(datas[7]-'0')*10+(uint8_t)(datas[8]-'0');
			day=(uint8_t)(datas[9]-'0')*10+(uint8_t)(datas[10]-'0');
			hour=(uint8_t)(datas[11]-'0')*10+(uint8_t)(datas[12]-'0');
			minute=(uint8_t)(datas[13]-'0')*10+(uint8_t)(datas[14]-'0');
			second=(uint8_t)(datas[15]-'0')*10+(uint8_t)(datas[16]-'0');
			date.year=year;
			date.month=month;
			date.day=day;
			date.time.hour=hour;
			date.time.minute=minute;
			date.time.second=second;
			ischanged=true;
			//设置时间
		break;//正常日期
		case '#':
			stamp=stringtostamp(datas);
			stamptotime(stamp,&temp);
			//validate(temp);
			date=temp;
			ischanged=true;
		break;//时间戳
	}
	return;
}

void blesetclock(char* datas)
{
	uint8_t hour=0;
	uint8_t minute=0;
	uint8_t second=0;
	struct Alarm temp={0,false};
	uint8_t index=datas[3]-'0';
	switch(datas[2])
	{
		case '*':
			hour=(uint8_t)(datas[4]-'0')*10+(uint8_t)(datas[5]-'0');
			minute=(uint8_t)(datas[6]-'0')*10+(uint8_t)(datas[7]-'0');
			second=(uint8_t)(datas[8]-'0')*10+(uint8_t)(datas[9]-'0');
			temp.time.hour=hour;
			temp.time.minute=minute;
			temp.time.second=second;
			temp.ison=(datas[10]=='Y')?true:false;
			if(validatealarm(temp))
			{
				alarms[index]=temp;
				alarms[index].ison=true;
			}
			//设置闹钟
		break;//设置闹钟
		case '1':
		case '2':
		case '3':if(datas[4]=='Y'||datas[4]=='y')
				{
					alarms[datas[2]-1-'0'].ison=true;
				}
				else if(datas[4]=='N'||datas[4]=='n')
				{
					alarms[datas[2]-1-'0'].ison=false;				}
		default:break;
	}
}

void bledate(struct Date target)
{
	char string[5]={'\0'};
	uint8_t week=9;
	itoa(target.year,string,10);
	transmitstring(string);
	itoa(target.month,string,10);
	if(target.month<10)
	{
		DL_UART_transmitDataBlocking(UART0,'0');
	}
	transmitstring(string);
	itoa(target.day,string,10);
	if(target.day<10)
	{
		DL_UART_transmitDataBlocking(UART0,'0');
	}
	transmitstring(string);
	week=countweek(target.year,target.month,target.day);
	switch(week)
	{
		case 0:transmitstring("MON");break;
		case 1:transmitstring("TUE");break;
		case 2:transmitstring("WED");break;
		case 3:transmitstring("THU");break;
		case 4:transmitstring("FRI");break;
		case 5:transmitstring("SAT");break;
		case 6:transmitstring("SUN");break;
		default:break;
	}
	bletime(target.time);
	return;
}

void bletime(struct Time target)
{
	uint8_t* p=NULL;
	char string[3]={'\0'};
	itoa(target.hour,string,10);
	if(target.hour<10)
	{
		DL_UART_transmitDataBlocking(UART0,'0');
	}
	transmitstring(string);
	itoa(target.minute,string,10);
	if(target.minute<10)
	{
		DL_UART_transmitDataBlocking(UART0,'0');
	}
	transmitstring(string);
	itoa(target.second,string,10);
	if(target.second<10)
	{
		DL_UART_transmitDataBlocking(UART0,'0');
	}
	transmitstring(string);
	DL_UART_transmitDataBlocking(UART0,'\n');
	return;
}

uint32_t timetostamp(struct Date target)
{
	static uint32_t dax=0;
	static uint32_t day_count=0;
	uint16_t leap_year_count=0;
	uint16_t i;

	for(i=1970;i<target.year;i++)
	{
		if(leapyear(i))
		{
			leap_year_count++;
		}
	}

	day_count=leap_year_count*366+(target.year-1970-leap_year_count)*365;

	for(i=1;i<target.month;i++)
	{
		if((2==i)&&(leapyear(target.year)))
		{
			day_count+=29;
		}
		else
		{
			day_count+=month_days_table[i];
		}
	}

	day_count+=(target.day-1);

	dax=(uint32_t)(day_count*86400)+(uint32_t)((uint32_t)target.time.hour*3600)+(uint32_t)((uint32_t)target.time.minute*60)+(uint32_t)target.time.second;

	dax=dax-8*60*60;

	return dax;
}

uint32_t stamptotime(uint32_t timep,struct Date* target)
{
	uint32_t days=0;
	uint32_t rem=0;

	timep=timep+8*60*60;

	days=(uint32_t)(timep/86400);
	rem=(uint32_t)(timep%86400);

	uint16_t year;
	for(year=1970;;++year)
	{
		uint16_t leap=((year%4==0&&year%100!=0)||(year%400==0));
		uint16_t ydays=leap?366:365;
		if(days<ydays)
		{
			break;
		}
		days-=ydays;
	}

	target->year=year;

	static const uint16_t days_in_month[]={31,28,31,30,31,30,31,31,30,31,30,31};
	uint16_t month;

	for(month=0;month<12;month++)
	{
		uint16_t mdays=days_in_month[month];
		if(month==1&&((year%4==0&&year%100!=0)||(year%400==0)))
		{
			mdays=29;
		}
		if(days<mdays)
		{
			break;
		}
		days-=mdays;
	}
	target->month=month+1;

	target->day=days+1;

	target->time.hour=rem/3600;
	rem%=3600;
	target->time.minute=rem/60;
	target->time.second=rem%60;

	return 0;

}

uint32_t stringtostamp(char* target)
{
	//从3开始
	const char shex[]="0123456789abcdef";
	const char bhex[]="0123456789ABCDEF";
	uint32_t stamp=0;

	for(uint8_t i=3;target[i]!='\n';i++)
	{
		uint8_t j=0;
		for(j=0;j<16;j++)
		{
			if(target[i]==shex[j]||target[i]==bhex[j])
			{
				break;
			}
		}
		stamp=stamp*16+j;
	}
	return stamp;
}

bool leapyear(uint32_t year)
{
	return(((year%4==0)&&(year%100!=0))||(year%400==0));
}

bool validate(struct Date target)
{
	if(target.year>=0&&target.year<=9999&&target.month>=1&&target.month<=12&&target.day>=1&&target.day<=31&&target.time.hour>=0&&target.time.hour<=23&&target.time.minute>=0&&target.time.minute<=59&&target.time.second>=0&&target.time.second<=59)
	{
		if((target.month==4||target.month==6||target.month==9||target.month==11)&&target.day==31)
		{
			return false;
		}
		if(target.month==2)
		{
			if((target.year%4==0&&target.year%100!=0)||(target.year%400==0))
			{
				if(target.day>29)
				{
					return false;
				}
			}
			else
			{
				if(target.day>28)
				{
					return false;
				}
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

struct Date judge(struct Date judgedate)
{
	if(judgedate.time.second>=60)
	{
		judgedate.time.second=0;
		judgedate.time.minute++;
	}
	if(judgedate.time.minute>=60)
	{
		judgedate.time.minute=0;
		judgedate.time.hour++;
	}
	if(judgedate.time.hour>=24)
	{
		judgedate.time.hour=0;
		judgedate.day++;
	}
	switch(judgedate.day)
	{
		case 29:if((judgedate.year%4==0&&judgedate.year%100!=0)||(judgedate.year%400==0))
					{
						if(judgedate.month==2)
						break;
					}
				else
				{
					if(judgedate.month==2)
					{
						judgedate.day=1;
						judgedate.month++;
					}
				}
		case 30:if((judgedate.year%4==0&&judgedate.year%100!=0)||judgedate.year%400==0)
				{
					if(judgedate.month==2)
					{
						judgedate.day=1;
						judgedate.month++;
					}
				}
				else break;
		case 31:if(judgedate.month==4||judgedate.month==6||judgedate.month==9||judgedate.month==11)
				{
					judgedate.day=1;
					judgedate.month++;
				}
				else break;
		case 32:if(judgedate.month==1||judgedate.month==3||judgedate.month==5||judgedate.month==7||judgedate.month==8||judgedate.month==10||judgedate.month==12)
				{
					judgedate.day=1;
					judgedate.month++;
				}
				else break;
		default:break;
	}
	if(judgedate.month==13)
	{
		judgedate.month=1;
		judgedate.year++;
	}
	if(judgedate.year>=10000)
	{
		judgedate.year=0;
	}
	return judgedate;
}

void showtime(uint8_t showhour,uint8_t showminute,uint8_t showsecond,uint32_t showyear,uint8_t showmonth,uint8_t showday,unsigned int showweek)
{
	unsigned int yeardigital=digitalcount(showyear);
	//OLED_Clear();
	
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

void showtimein12(uint8_t showhour,uint8_t showminute,uint8_t showsecond,uint32_t showyear,uint8_t showmonth,uint8_t showday,unsigned int showweek)
{
	if(showhour>=12)
	{
		if(showhour>12)
			showhour-=12;
		showtime(showhour,showminute,showsecond,showyear,showmonth,showday,showweek);
		OLED_ShowString(64,0,"(PM)");
	}
	else
	{
		showtime(showhour,showminute,showsecond,showyear,showmonth,showday,showweek);
		OLED_ShowString(64,0,"(AM)");
	}
}

void showtimesimplified(int x, int y,uint8_t showhour,uint8_t showminute,uint8_t showsecond)
{
  switch(showhour)
	{
		case 0:OLED_ShowString(x,y,"00:");break;
		case 1:OLED_ShowString(x,y,"01:");break;
		case 2:OLED_ShowString(x,y,"02:");break;
		case 3:OLED_ShowString(x,y,"03:");break;
		case 4:OLED_ShowString(x,y,"04:");break;
		case 5:OLED_ShowString(x,y,"05:");break;
		case 6:OLED_ShowString(x,y,"06:");break;
		case 7:OLED_ShowString(x,y,"07:");break;
		case 8:OLED_ShowString(x,y,"08:");break;
		case 9:OLED_ShowString(x,y,"09:");break;
		case 10:OLED_ShowString(x,y,"10:");break;
		case 11:OLED_ShowString(x,y,"11:");break;
		case 12:OLED_ShowString(x,y,"12:");break;
		case 13:OLED_ShowString(x,y,"13:");break;
		case 14:OLED_ShowString(x,y,"14:");break;
		case 15:OLED_ShowString(x,y,"15:");break;
		case 16:OLED_ShowString(x,y,"16:");break;
		case 17:OLED_ShowString(x,y,"17:");break;
		case 18:OLED_ShowString(x,y,"18:");break;
		case 19:OLED_ShowString(x,y,"19:");break;
		case 20:OLED_ShowString(x,y,"20:");break;
		case 21:OLED_ShowString(x,y,"21:");break;
		case 22:OLED_ShowString(x,y,"22:");break;
		case 23:OLED_ShowString(x,y,"23:");break;
	}
	switch(showminute)
	{
		case 0:OLED_ShowString(x+24,y,"00:");break;
		case 1:OLED_ShowString(x+24,y,"01:");break;
		case 2:OLED_ShowString(x+24,y,"02:");break;
		case 3:OLED_ShowString(x+24,y,"03:");break;
		case 4:OLED_ShowString(x+24,y,"04:");break;
		case 5:OLED_ShowString(x+24,y,"05:");break;
		case 6:OLED_ShowString(x+24,y,"06:");break;
		case 7:OLED_ShowString(x+24,y,"07:");break;
		case 8:OLED_ShowString(x+24,y,"08:");break;
		case 9:OLED_ShowString(x+24,y,"09:");break;
		case 10:OLED_ShowString(x+24,y,"10:");break;
		case 11:OLED_ShowString(x+24,y,"11:");break;
		case 12:OLED_ShowString(x+24,y,"12:");break;
		case 13:OLED_ShowString(x+24,y,"13:");break;
		case 14:OLED_ShowString(x+24,y,"14:");break;
		case 15:OLED_ShowString(x+24,y,"15:");break;
		case 16:OLED_ShowString(x+24,y,"16:");break;
		case 17:OLED_ShowString(x+24,y,"17:");break;
		case 18:OLED_ShowString(x+24,y,"18:");break;
		case 19:OLED_ShowString(x+24,y,"19:");break;
		case 20:OLED_ShowString(x+24,y,"20:");break;
		case 21:OLED_ShowString(x+24,y,"21:");break;
		case 22:OLED_ShowString(x+24,y,"22:");break;
		case 23:OLED_ShowString(x+24,y,"23:");break;
		case 24:OLED_ShowString(x+24,y,"24:");break;
		case 25:OLED_ShowString(x+24,y,"25:");break;
		case 26:OLED_ShowString(x+24,y,"26:");break;
		case 27:OLED_ShowString(x+24,y,"27:");break;
		case 28:OLED_ShowString(x+24,y,"28:");break;
		case 29:OLED_ShowString(x+24,y,"29:");break;
		case 30:OLED_ShowString(x+24,y,"30:");break;
		case 31:OLED_ShowString(x+24,y,"31:");break;
		case 32:OLED_ShowString(x+24,y,"32:");break;
		case 33:OLED_ShowString(x+24,y,"33:");break;
		case 34:OLED_ShowString(x+24,y,"34:");break;
		case 35:OLED_ShowString(x+24,y,"35:");break;
		case 36:OLED_ShowString(x+24,y,"36:");break;
		case 37:OLED_ShowString(x+24,y,"37:");break;
		case 38:OLED_ShowString(x+24,y,"38:");break;
		case 39:OLED_ShowString(x+24,y,"39:");break;
		case 40:OLED_ShowString(x+24,y,"40:");break;
		case 41:OLED_ShowString(x+24,y,"41:");break;
		case 42:OLED_ShowString(x+24,y,"42:");break;
		case 43:OLED_ShowString(x+24,y,"43:");break;
		case 44:OLED_ShowString(x+24,y,"44:");break;
		case 45:OLED_ShowString(x+24,y,"45:");break;
		case 46:OLED_ShowString(x+24,y,"46:");break;
		case 47:OLED_ShowString(x+24,y,"47:");break;
		case 48:OLED_ShowString(x+24,y,"48:");break;
		case 49:OLED_ShowString(x+24,y,"49:");break;
		case 50:OLED_ShowString(x+24,y,"50:");break;
		case 51:OLED_ShowString(x+24,y,"51:");break;
		case 52:OLED_ShowString(x+24,y,"52:");break;
		case 53:OLED_ShowString(x+24,y,"53:");break;
		case 54:OLED_ShowString(x+24,y,"54:");break;
		case 55:OLED_ShowString(x+24,y,"55:");break;
		case 56:OLED_ShowString(x+24,y,"56:");break;
		case 57:OLED_ShowString(x+24,y,"57:");break;
		case 58:OLED_ShowString(x+24,y,"58:");break;
		case 59:OLED_ShowString(x+24,y,"59:");break;
	}
	switch(showsecond)
	{
		case 0:OLED_ShowString(x+48,y,"00");break;
		case 1:OLED_ShowString(x+48,y,"01");break;
		case 2:OLED_ShowString(x+48,y,"02");break;
		case 3:OLED_ShowString(x+48,y,"03");break;
		case 4:OLED_ShowString(x+48,y,"04");break;
		case 5:OLED_ShowString(x+48,y,"05");break;
		case 6:OLED_ShowString(x+48,y,"06");break;
		case 7:OLED_ShowString(x+48,y,"07");break;
		case 8:OLED_ShowString(x+48,y,"08");break;
		case 9:OLED_ShowString(x+48,y,"09");break;
		case 10:OLED_ShowString(x+48,y,"10");break;
		case 11:OLED_ShowString(x+48,y,"11");break;
		case 12:OLED_ShowString(x+48,y,"12");break;
		case 13:OLED_ShowString(x+48,y,"13");break;
		case 14:OLED_ShowString(x+48,y,"14");break;
		case 15:OLED_ShowString(x+48,y,"15");break;
		case 16:OLED_ShowString(x+48,y,"16");break;
		case 17:OLED_ShowString(x+48,y,"17");break;
		case 18:OLED_ShowString(x+48,y,"18");break;
		case 19:OLED_ShowString(x+48,y,"19");break;
		case 20:OLED_ShowString(x+48,y,"20");break;
		case 21:OLED_ShowString(x+48,y,"21");break;
		case 22:OLED_ShowString(x+48,y,"22");break;
		case 23:OLED_ShowString(x+48,y,"23");break;
		case 24:OLED_ShowString(x+48,y,"24");break;
		case 25:OLED_ShowString(x+48,y,"25");break;
		case 26:OLED_ShowString(x+48,y,"26");break;
		case 27:OLED_ShowString(x+48,y,"27");break;
		case 28:OLED_ShowString(x+48,y,"28");break;
		case 29:OLED_ShowString(x+48,y,"29");break;
		case 30:OLED_ShowString(x+48,y,"30");break;
		case 31:OLED_ShowString(x+48,y,"31");break;
		case 32:OLED_ShowString(x+48,y,"32");break;
		case 33:OLED_ShowString(x+48,y,"33");break;
		case 34:OLED_ShowString(x+48,y,"34");break;
		case 35:OLED_ShowString(x+48,y,"35");break;
		case 36:OLED_ShowString(x+48,y,"36");break;
		case 37:OLED_ShowString(x+48,y,"37");break;
		case 38:OLED_ShowString(x+48,y,"38");break;
		case 39:OLED_ShowString(x+48,y,"39");break;
		case 40:OLED_ShowString(x+48,y,"40");break;
		case 41:OLED_ShowString(x+48,y,"41");break;
		case 42:OLED_ShowString(x+48,y,"42");break;
		case 43:OLED_ShowString(x+48,y,"43");break;
		case 44:OLED_ShowString(x+48,y,"44");break;
		case 45:OLED_ShowString(x+48,y,"45");break;
		case 46:OLED_ShowString(x+48,y,"46");break;
		case 47:OLED_ShowString(x+48,y,"47");break;
		case 48:OLED_ShowString(x+48,y,"48");break;
		case 49:OLED_ShowString(x+48,y,"49");break;
		case 50:OLED_ShowString(x+48,y,"50");break;
		case 51:OLED_ShowString(x+48,y,"51");break;
		case 52:OLED_ShowString(x+48,y,"52");break;
		case 53:OLED_ShowString(x+48,y,"53");break;
		case 54:OLED_ShowString(x+48,y,"54");break;
		case 55:OLED_ShowString(x+48,y,"55");break;
		case 56:OLED_ShowString(x+48,y,"56");break;
		case 57:OLED_ShowString(x+48,y,"57");break;
		case 58:OLED_ShowString(x+48,y,"58");break;
		case 59:OLED_ShowString(x+48,y,"59");break;
	}
}

void DisplaySettings(void)
{
	OLED_Clear();
	OLED_ShowString(0,0,"Settings");
	OLED_ShowString(0,2,"1. Set Date");
	OLED_ShowString(0,4,"2. Set Time");
	OLED_ShowString(0,6,"3. Exit");
}

void DisplayFunctions(void)
{
	OLED_Clear();
	OLED_ShowString(0,0,"Functions");
	OLED_ShowString(0,2,"1. Set Alarm");
	OLED_ShowString(0,4,"2. Set Timer");
	OLED_ShowString(0,6,"3. Exit");
}

struct Time Settime(struct Time original,uint8_t num)
{
	struct Time result;
	unsigned int yscale=0;
	unsigned int status=114514;
	unsigned int lastnumber=num;
	
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
			//OLED_Init();
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
					case 15:/*switch(settingparameter)
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
					}//退格*/
					default:break;
				}
			}
		}
		lastnumber=status;
		if(iscanceled)
		{
			return original;
			break;
		}
		if(isset)
		{
			if(sethour>=0&&sethour<=23&&setminute>=0&&setminute<=59&&setsecond>=0&&setsecond<=59)
			{
				result.hour=sethour;
				result.minute=setminute;
				result.second=setsecond;
				ischanged=true;
				return result;
				break;
			}
			else
			{
				OLED_Clear();
				OLED_ShowString(0,0,"Invalid input!");
				delay_cycles(120000000);//may cause error.
				ischanged=true;
				return original;
				break;
			}
		}
	}
	return original;
}

void Setdate(void)
{
	unsigned int yscale=0;
	unsigned int status=114514;
	unsigned int lastnumber=1;
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
				date.year=setyear;
				date.month=setmonth;
				date.day=setday;
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
	//return false;
}

void DisplayAlarm(struct Alarm* alarms)
{
  int status=114514;
  int lastnumber=1;
  bool isquit=false;
  bool isclear=true;
  while(1)
  {
    status=scan();
	if(isclear)
	{
		OLED_Clear();
		OLED_ShowString(0,0,"Please choose:");
		OLED_ShowString(0,2,"1.");
		OLED_ShowString(0,4,"2.");
		OLED_ShowString(0,6,"3.");
		for(uint8_t i=0;i<3;i++)
		{
			showtimesimplified(16,2+2*i,alarms[i].time.hour,alarms[i].time.minute,alarms[i].time.second);
			if(alarms[i].ison)
			OLED_ShowString(88,2+2*i,"ON");
			else
			OLED_ShowString(88,2+2*i,"OFF");				
		}
		isclear=false;
	}
    if(status!=114514)
    {
      if(lastnumber!=status&&lastnumber!=3)
      {
        switch(status)
        {
          case 1:
		  case 2:
		  case 3:
		  	alarms[status-1].time=Settime(alarms[status-1].time,status);
			alarms[status-1].ison=true;
		  status=3;
		  lastnumber=3;
		  isclear=true;
		  break;//选中
		  case 10://退出
		  isquit=true;
		  break;
		  case 12:alarms[0].ison=!alarms[0].ison;isclear=true;break;
		  case 13:alarms[1].ison=!alarms[1].ison;isclear=true;break;
		  case 14:alarms[2].ison=!alarms[2].ison;isclear=true;break;
		  case 16:isclear=true;break;
		  default:break;
        }
      }
    }
    lastnumber=status;
    if(isquit)
    {
      break;
    }
  }
}

void DisplayCounter(void)
{
	struct Date timer={0,0,0,0,0,0};
  int status=114514;
  int lastnumber=15;
  bool isquit=false;
  bool isstart=false;
  bool isticking=false;
  bool ismove=true;
  isticked=false;
  //OLED_ShowString(0,6,"Start");
  while(1)
  {
	if(isticked)
	{
		timer.time.second=timersecond;
		timer.time.minute=timerminute;
		timer.time.hour=timerhour;
		if(timer.time.hour==0&&timer.time.minute==0&&timer.time.second==0)
		{	
			OLED_Clear();
			isstart=false;
			isticking=false;
			DL_TimerG_stopCounter(TIMER_1_INST);
			NVIC_DisableIRQ(TIMER_1_INST_INT_IRQN);
			Beep(timer);
			ismove=true;
			isticked=false;
		}
		/*if(timer.time.second==0)
		{
			timersecond+=60;
		}
		if(timer.time.second==59)
		{
			if(timer.time.minute!=0)
			{
				timer.time.minute--;
			}
			else if(timer.time.hour!=0)
			{
				timer.time.hour--;
				timer.time.minute+=59;
			}
		}*/
		//isticked=false;
		//ismove=true;
	}
	if(isticking)
	{
		showtimesimplified(0,0,timerhour,timerminute,timersecond);
		isticked=false;
	}
	if(ismove)
	{
		OLED_Clear();
		showtimesimplified(0,0,timer.time.hour,timer.time.minute,timer.time.second);
		ismove=false;
	}
    status=scan();
    if(status!=114514)
    {
      if(lastnumber!=status&&lastnumber!=15)
      {
        switch(status)
        {
		case 10:  //clear or quit
		  if(isstart)
		  {
			timer.time.hour=0;
			timer.time.minute=0;
			timer.time.second=0;
			isstart=false;
			isticking=false;
			DL_TimerG_stopCounter(TIMER_1_INST);
			NVIC_DisableIRQ(TIMER_1_INST_INT_IRQN);
			ismove=true;
		  }
		  else
		  {
			isquit=true;
			ischanged=true;
		  }
		break;
        case 11://Start or pause
		  if(timer.time.hour==0&&timer.time.minute==0&&timer.time.second==0)
		  	break;
		  else if(!isticking&&!isstart)
		  {
			isstart=true;
			isticking=true;
			DL_TimerG_startCounter(TIMER_1_INST);
			NVIC_EnableIRQ(TIMER_1_INST_INT_IRQN);
		  }	
		  else if(!isticking&&isstart)
		  {
			DL_TimerG_startCounter(TIMER_1_INST);
			isticking=true;
		  }
		  else if(isticking)
		  {
			isticking=false;
			DL_TimerG_stopCounter(TIMER_1_INST);
		  }
		break;
		case 15://set
		if(!isticking)
		{
			timer.time=Settime(timer.time,15);
			timersecond=timer.time.second;
			timerminute=timer.time.minute;
			timerhour=timer.time.hour;
			ismove=true;
		}
		break;
          default:break;
        }
      }
    }
    lastnumber=status;
    if(isquit)
    {
      break;
    }
  }
}

bool CheckAlarm(struct Date target,struct Alarm alarms[3])
{
  struct Alarm comp={target.time.hour, target.time.minute,target.time.second,true};
  if(CompareAlarm(alarms[0],comp)||CompareAlarm(alarms[1],comp)||CompareAlarm(alarms[2],comp))
  	return true;
  else
  	return false;
}

bool CompareAlarm(struct Alarm target1,struct Alarm target2)
{
	if(target1.time.hour==target2.time.hour&&target1.time.minute==target2.time.minute&&target1.time.second==target2.time.second&&target1.ison==target2.ison)
		return true;
	else
		return false;
}

void Beep(struct Date target)
{
	DL_GPIO_clearPins(BUZZER_PORT,BUZZER_SDA_PIN);
	OLED_Clear();
	showtimesimplified(0,0,target.time.hour,target.time.minute,target.time.second);
	OLED_ShowString(0,6,"Press to stop");
	while(1)
	{
		Buzz(C3,200);
		delay_cycles(1700000);
		if(scan()!=114514)
		{
			break;
		}
	}
	DL_GPIO_setPins(BUZZER_PORT,BUZZER_SDA_PIN);
	OLED_Clear();
}

void Buzz(unsigned int frequency,unsigned int duration)
{
	unsigned int buzztime=(duration/1000.0)*frequency;
	for(unsigned int i=0;i<buzztime;i++)
	{
		DL_GPIO_togglePins(BUZZER_PORT,BUZZER_SCL_PIN);
		delay_cycles(34000000/frequency);
	}
	DL_GPIO_clearPins(BUZZER_PORT,BUZZER_SCL_PIN);
	delay_cycles(3400000);
}

void readalarm(struct Alarm* target)
{
  for(unsigned int i=0;i<EEPROM_EMULATION_DATA_SIZE/sizeof(uint32_t);i++)
		EEPROMEmulationBuffer[i]=0;
	for(int i=0;(*(uint32_t *)(ALARMADDRESS+i))!=0xFFFFFFFF;i+=4)
		EEPROMEmulationBuffer[i/4]=*(uint32_t *)(ALARMADDRESS+i);
	struct Alarm result[3]={{EEPROMEmulationBuffer[0],EEPROMEmulationBuffer[1],EEPROMEmulationBuffer[2],EEPROMEmulationBuffer[3]},{EEPROMEmulationBuffer[4],EEPROMEmulationBuffer[5],EEPROMEmulationBuffer[6],EEPROMEmulationBuffer[7]},{EEPROMEmulationBuffer[8],EEPROMEmulationBuffer[9],EEPROMEmulationBuffer[10],EEPROMEmulationBuffer[11]}};
	target[0]=result[0];
	target[1]=result[1];
	target[2]=result[2];
}

bool validatealarm(struct Alarm target)
{
  if(target.time.hour>=0&&target.time.hour<=23&&target.time.minute>=0&&target.time.minute<=59&&target.time.second>=0&&target.time.second<=59)
    return true;
  else
    return false;
}

void save(struct Date savedate,struct Alarm savealarm[3])
{
	uint32_t dataarray[EEPROM_EMULATION_DATA_SIZE / sizeof(uint32_t)]={0};
	EEPROM_TypeA_eraseAllSectors();
	for(unsigned int i=0;i<EEPROM_EMULATION_DATA_SIZE/sizeof(uint32_t);i++)
		dataarray[i]=0;
	dataarray[0]=savedate.year;
	dataarray[1]=savedate.month;
	dataarray[2]=savedate.day;
	dataarray[3]=savedate.time.hour;
	dataarray[4]=savedate.time.minute;
	dataarray[5]=savedate.time.second;
	dataarray[6]=savealarm[0].time.hour;
	dataarray[7]=savealarm[0].time.minute;
	dataarray[8]=savealarm[0].time.second;
	dataarray[9]=savealarm[0].ison;
	dataarray[10]=savealarm[1].time.hour;
	dataarray[11]=savealarm[1].time.minute;
	dataarray[12]=savealarm[1].time.second;
	dataarray[13]=savealarm[1].ison;
	dataarray[14]=savealarm[2].time.hour;
	dataarray[15]=savealarm[2].time.minute;
	dataarray[16]=savealarm[2].time.second;
	dataarray[17]=savealarm[2].ison;
	DL_FlashCTL_unprotectSector( FLASHCTL, ADDRESS, DL_FLASHCTL_REGION_SELECT_MAIN);
	DL_FlashCTL_programMemoryFromRAM( FLASHCTL, ADDRESS, dataarray, 18, DL_FLASHCTL_REGION_SELECT_MAIN);
	// DL_FlashCTL_programMemoryFromRAM( FLASHCTL, ADDRESS, dataarray, 6, DL_FLASHCTL_REGION_SELECT_MAIN);

}

struct Date read(uint8_t mode)
{
	uint32_t address=ADDRESS;
	switch(mode)
	{
		case CLOCK:address=ADDRESS;break;
		case ALARM1:break;
		case ALARM2:break;
		case ALARM3:break;
		default:break;
	}
	for(unsigned int i=0;i<EEPROM_EMULATION_DATA_SIZE/sizeof(uint32_t);i++)
		EEPROMEmulationBuffer[i]=0;
	for(int i=0;(*(uint32_t *)(address+i))!=0xFFFFFFFF;i+=4)
		EEPROMEmulationBuffer[i/4]=*(uint32_t *)(address+i);
	struct Date result={EEPROMEmulationBuffer[0],EEPROMEmulationBuffer[1],EEPROMEmulationBuffer[2],EEPROMEmulationBuffer[3],EEPROMEmulationBuffer[4],EEPROMEmulationBuffer[5]};
	return result;	
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
	delay_cycles(400000);
	if(!(DL_GPIO_readPins(MATRIX_PORT, inputpin)))
		return 1;
	else 
		return 0;
	}
	else return 0;
}
	
	else {
		delay_cycles(800000);
	if(!(DL_GPIO_readPins(MATRIX_PORT, inputpin)))
	{
	delay_cycles(800000);
	if(!(DL_GPIO_readPins(MATRIX_PORT, inputpin)))
		return 1;
	else 
		return 0;
	}
	else return 0;
	}
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

/*更新预计
--4，计时器配置,4s卡一下--
6,debunce to bool

目前问题
--3，存储不灵敏--
5, 退格
*/

char* itoa(int num,char* str,int radix)
{
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//索引表
    unsigned unum;//存放要转换的整数的绝对值,转换的整数可能是负数
    int i=0,j,k;//i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。
 
    //获取要转换的整数的绝对值
    if(radix==10&&num<0)//要转换成十进制数并且是负数
    {
        unum=(unsigned)-num;//将num的绝对值赋给unum
        str[i++]='-';//在字符串最前面设置为'-'号，并且索引加1
    }
    else unum=(unsigned)num;//若是num为正，直接赋值给unum
 
    //转换部分，注意转换后是逆序的
    do
    {
        str[i++]=index[unum%(unsigned)radix];//取unum的最后一位，并设置为str对应位，指示索引加1
        unum/=radix;//unum去掉最后一位
 
    }while(unum);//直至unum为0退出循环
 
    str[i]='\0';//在字符串最后添加'\0'字符，c语言字符串以'\0'结束。
 
    //将顺序调整过来
    if(str[0]=='-') k=1;//如果是负数，符号不用调整，从符号后面开始调整
    else k=0;//不是负数，全部都要调整
 
    char temp;//临时变量，交换两个值时用到
    for(j=k;j<=(i-1)/2;j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
    {
        temp=str[j];//头部赋值给临时变量
        str[j]=str[i-1+k-j];//尾部赋值给头部
        str[i-1+k-j]=temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
    }
 
    return str;//返回转换后的字符串
 
}

void transmitstring(char* p)
{
	for(uint8_t i=0;p[i]!='\0';i++)
	{
		DL_UART_transmitDataBlocking(UART0,p[i]);
	}
	return;
}
/*
void transmitdata(uint8_t mode, int value)
{
	char* thd="thd.val=";
	char* u1="u1.val=";
	char* u2="u2.val=";
	char* u3="u3.val=";
	char* u4="u4.val=";
	char* u5="u5.val=";
	char* tail="\xff\xff\xff";
	char* add="add s0.id,0,";
	char* p=NULL;
	char stringvalue[10]={'\0'};
	itoa(value,stringvalue,10);
	switch(mode)
	{
		case 1:p=thd;break;
		case 2:p=u1;break;
		case 3:p=u2;break;
		case 4:p=u3;break;
		case 5:p=u4;break;
		case 6:p=u5;break;
		case 7:p=add;break;
	}
	transmitstring(p);
	transmitstring(stringvalue);
	transmitstring(tail);
	return;
}

void transmit(float thd, float us[5],float curve)
{
	int thdvalue=thd*100;
	int usvalue[5];
	int curvevalue=(curvevalue>=0.6)?255:curve*1000/600*255;
	for(uint8_t i=0;i<5;i++)
	{
		usvalue[i]=us[i]*100;
	}
	transmitdata(1,thdvalue);
	transmitdata(2,usvalue[0]);
	transmitdata(3,usvalue[1]);
	transmitdata(4,usvalue[2]);
	transmitdata(5,usvalue[3]);
	transmitdata(6,usvalue[4]);
	transmitdata(7,curvevalue);
	return;
}
*/
/*
void transmittophone(float curve ,float thd, float u[5])
{
	uint8_t head=0xa5;
	uint8_t tail=0x5a;
	uint8_t check=0;
	uint8_t* p=(uint8_t*)&curve;
	DL_UART_transmitDataBlocking(UART0,head);
	for(uint8_t i=0;i<4;i++)
	{
		DL_UART_transmitDataBlocking(UART0,*p);
		check+=*p;
		p++;
	}
	p=(uint8_t*)&thd;
	for(uint8_t i=0;i<4;i++)
	{
		DL_UART_transmitDataBlocking(UART0,*p);
		check+=*p;
		p++;
	}
	p=(uint8_t*)u;
	for(uint8_t i=0;i<20;i++)
	{
		DL_UART_transmitDataBlocking(UART0,*p);
		check+=*p;
		p++;
	}
	DL_UART_transmitDataBlocking(UART0,check);
	DL_UART_transmitDataBlocking(UART0,tail);
	return;
}*/


// void transmittophone(float data)
// {
// 	uint8_t head=0xa5;
// 	uint8_t tail=0x5a;
// 	uint8_t check=0;
// 	uint8_t* p=(uint8_t*)&data;
// 	DL_UART_transmitDataBlocking(UART0,head);
// 	for(uint8_t i=0;i<4;i++)
// 	{
// 		DL_UART_transmitDataBlocking(UART0,*p);
// 		check+=*p;
// 		p++;
// 	}
// 	DL_UART_transmitDataBlocking(UART0,check);
// 	DL_UART_transmitDataBlocking(UART0,tail);
// 	return;
// }