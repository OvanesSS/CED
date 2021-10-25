#include "C8051F360.h"
#include <stdio.h>
#include <string.h>

#define PP1_1 0x01
#define PP1_3 0x03
#define PP1_5 0x05
#define PP1_7 0x07
#define PP2_1 0x09
#define PP2_3 0x0B
#define NP1_0 0x00
#define NP1_2 0x02
#define NP1_4 0x04
#define NP1_6 0x06
#define NP2_0 0x08
#define NP2_2 0x0A

sbit DE = P0^6;						//driver tx/rx control, rs485, DE

unsigned int j = 0;				//counter for sending messages once per second
unsigned int k = 0;				//counter for sending an array byte by byte
bit ADCStartProcess = 1;
unsigned char result [24];

struct ADCdata {
	unsigned char High_byte_ch1;
	unsigned char Low_byte_ch1;
	unsigned char High_byte_ch2;
	unsigned char Low_byte_ch2;
	unsigned char High_byte_ch3;
	unsigned char Low_byte_ch3;
	unsigned char High_byte_ch4;
	unsigned char Low_byte_ch4;
	unsigned char High_byte_ch5;
	unsigned char Low_byte_ch5;
	unsigned char High_byte_ch6;
	unsigned char Low_byte_ch6;
} ADC;

void ADCdata_Init(void)
{
	ADC.High_byte_ch1 = 0;
	ADC.Low_byte_ch1 = 0;
	ADC.High_byte_ch2 = 0;
	ADC.Low_byte_ch2 = 0;
	ADC.High_byte_ch3 = 0;
	ADC.Low_byte_ch3 = 0;
	ADC.High_byte_ch4 = 0;
	ADC.Low_byte_ch4 = 0;
	ADC.High_byte_ch5 = 0;
	ADC.Low_byte_ch5 = 0;
	ADC.High_byte_ch6 = 0;
	ADC.Low_byte_ch6 = 0;
}

void PCA_Init()
{
    PCA0MD    = 0x00;
    PCA0CPM5  = 0x48;
}

void Timer_Init()
{
    TCON      = 0x40;
    TMOD      = 0x21;
    TH1       = 0x95;
		TR0       = 1;
		TR1       = 1;
	
}

void UART_Init()
{
    SCON0     = 0x10;
}

void ADC_Init()
{
    ADC0CF    = 0xA0;
    ADC0CN    = 0x80;
}

void Voltage_Reference_Init()
{
    REF0CN    = 0x07;
}

void Port_IO_Init()
{
    SFRPAGE   = CONFIG_PAGE;
    P0MDIN    = 0xFE;
    P1MDIN    = 0x00;
    P2MDIN    = 0xF0;
    P0MDOUT   = 0xFF;
    P0SKIP    = 0x01;
    P1SKIP    = 0xFF;
    P2SKIP    = 0x0F;
    XBR0      = 0x01;
    XBR1      = 0x40;
}

void Oscillator_Init()
{
    SFRPAGE   = CONFIG_PAGE;
    OSCICN    = 0x83;
}

void Interrupts_Init()
{
    SFRPAGE   = CONFIG_PAGE;
    IE        = 0x82;

}

void Init_Device(void)
{
    PCA_Init();
    Timer_Init();
    UART_Init();
    ADC_Init();
    Voltage_Reference_Init();
    Port_IO_Init();
    Oscillator_Init();
    Interrupts_Init();
		ADCdata_Init();
}

void UART0_transmit(unsigned char data0)
{ 
	char SFRPAGE_SAVE = SFRPAGE; 
	SFRPAGE = UART0_PAGE; 
 	SBUF0 = data0;
	while(!TI0);          
	TI0 = 0;
	SFRPAGE = SFRPAGE_SAVE;
}

void ADC_read(unsigned char x, unsigned char y, unsigned char *data1, unsigned char *data2)
{
	int i = 0, High = 0, Low = 0, figure = 0, Sum = 0;
	AD0BUSY = 0;
	AMX0N = x;
	AMX0P = y;
	for (i = 0; i < 7; i++)
		{
			if(AD0BUSY == 0)
			{
				AD0INT = 0;
				ADC0H = 0x00;
				ADC0L = 0x00;
				AD0BUSY = 1;
				while (AD0INT != 1) {}
				if (AD0INT == 1)
				{
					High = ADC0H;
					Low = ADC0L;
					figure = (High << 8) + Low;
					Sum = Sum + figure;
				}
			}
		}
	Sum = Sum/8;
	*data1 = (Sum >> 8);
	*data2 = Sum;
}

void timer0_ISR (void) interrupt 1 
{
	j++;
	if((j == 33) && (ADCStartProcess == 0))
	{
		j=0;
		TL0 = 0x00;
		TH0 = 0x00;       
		TL0 = 0xFF;       
		sprintf(result, "$01%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c!\r\n", 1, ADC.High_byte_ch1, ADC.Low_byte_ch1, 2, ADC.High_byte_ch2, ADC.Low_byte_ch2, 3, ADC.High_byte_ch3, ADC.Low_byte_ch3, 4, ADC.High_byte_ch4, ADC.Low_byte_ch4, 5, ADC.High_byte_ch5, ADC.Low_byte_ch5, 6, ADC.High_byte_ch6, ADC.Low_byte_ch6);
		for (k = 0; k < 24; k++)
		{
			UART0_transmit(result[k]);
		}
		ADCStartProcess = 1;
	}
}

void main()
{
	Init_Device();
	DE = 1;
	while(1)
	{
		if (ADCStartProcess == 1)
		{
		ADC_read(PP1_1, NP1_0, &ADC.High_byte_ch6, &ADC.Low_byte_ch6);
		ADC_read(PP1_3, NP1_2, &ADC.High_byte_ch5, &ADC.Low_byte_ch5);
		ADC_read(PP1_5, NP1_4, &ADC.High_byte_ch4, &ADC.Low_byte_ch4);
		ADC_read(PP1_7, NP1_6, &ADC.High_byte_ch3, &ADC.Low_byte_ch3);
		ADC_read(PP2_1, NP2_0, &ADC.High_byte_ch2, &ADC.Low_byte_ch2);
		ADC_read(PP2_3, NP2_2, &ADC.High_byte_ch1, &ADC.Low_byte_ch1);
		ADCStartProcess = 0;
		}
	}
}
