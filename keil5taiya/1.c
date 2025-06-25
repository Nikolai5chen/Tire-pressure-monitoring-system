#include <reg51.h>
#include <intrins.h>
//ADC0832 引脚
sbit ADCS = P2 ^ 0;  //AD的片选 
sbit ADDI = P3 ^ 7;
sbit ADDO = P3 ^ 7;
sbit ADCLK = P3 ^ 6;  //AD的CLK 

unsigned char dispbitcode[8] = { 0xF7,0xFB,0xFD,0xFE,0xEF,0xDF,0xBF,0x7F }; //位扫描
unsigned char dispcode[11] = { 0xC0,0xF9,0xA4,0xbB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF };  //最后一个是全亮，防止出现一些问题 
//段选码 共阳极
unsigned char dispbuf[4];
unsigned int temp;
unsigned char getdata;

void delay_1ms(void)
{
	unsigned char x, y;
	x = 3;
	while (x--)
	{
		y = 40;
		while (y--);
	}
}
void display(void)
{
	char k;
	for (k = 0; k < 4; k++)
	{
		P1 = ~dispbitcode[k];
		P0 = ~dispcode[dispbuf[k]];
		if (k == 1)
			P0 = P0 + 0x80;
		delay_1ms();
	}
}

unsigned int ADC0832(unsigned char channel)  //AD转换，返回结果
{
	unsigned char i = 0;
	unsigned char j;
	unsigned int dat = 0;
	unsigned char ndat = 0;

	if (channel == 0) channel = 2;
	if (channel == 1) channel = 3;
	ADDI = 1;
	_nop_();
	_nop_();
	ADCS = 0;
	_nop_();
	_nop_();
	ADCLK = 1;
	_nop_();
	_nop_();
	ADCLK = 0;
	_nop_();
	_nop_();
	ADCLK = 1;
	ADDI = channel & 0x1;
	_nop_();
	_nop_();
	ADCLK = 0;
	_nop_();
	_nop_();
	ADCLK = 1;
	ADDI = (channel >> 1) & 0x1;
	_nop_();
	_nop_();
	ADCLK = 0;
	ADDI = 1;
	_nop_();
	_nop_();
	dat = 0;
	for (i = 0; i < 8; i++)
	{
		dat |= ADDO;
		ADCLK = 1;
		_nop_();
		_nop_();
		ADCLK = 0;
		_nop_();
		_nop_();
		dat <<= 1;
		if (i == 7) dat |= ADDO;
	}
	for (i = 0; i < 8; i++)
	{
		j = 0;
		j = j | ADDO;
		ADCLK = 1;
		_nop_();
		_nop_();
		ADCLK = 0;
		_nop_();
		_nop_();
		j = j << 7;
		ndat = ndat | j;
		if (i < 7) ndat >>= 1;
	}
	ADCS = 1;
	ADCLK = 0;
	ADDO = 1;
	dat <<= 8;
	dat |= ndat;
	return(dat);
}
void main(void)
{
	while (1)
	{
		unsigned int temp;
		float press;
		getdata = ADC0832(0);   //使用CH0 
		if (14 < getdata < 243)   //压力测量范围 
		{
			int vary = getdata;
			/*压力传感器将压力转换为电压量，AD将这个电压值进行数模转换，但是电压值并不直接意味着压力的大小
			例如5V并不意味着5kPa,因此还需要一步转换*/ 
			press = ((10.0 / 23.0)*vary) + 9.3; //press即是电压 
			temp = (int)(press * 10);
			//显示buff，一共四位 
			dispbuf[3] = temp / 1000;  //最高位 
			dispbuf[2] = (temp % 1000) / 100;
			dispbuf[1] = ((temp % 1000) % 100) / 10;
			dispbuf[0] = ((temp % 1000) % 100) % 10;  //最低位 
			display();
		}
	}
}

/*
输入  15--115kPA压力信号
         输出  00h--ffh数字信号（adc0832）
         在LCD上显示实际的压力值，如果超限则报警

线性区间标度变换公式：    y=(115-15)/(243-13)*X+9.3kpa   
*/ 
