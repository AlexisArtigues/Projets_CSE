//------------------------------------------------------------------------------------
// Base_8051
//------------------------------------------------------------------------------------
//
// AUTH: DF
// DATE: 18-09-2017
// Target: C8051F020
//
// Tool chain: KEIL UV4
//
// Application de base qui configure seulement l'oscillateur et le watchdog
//-------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------
#include "c8051f020.h" // SFR declarations
#include <intrins.h>  
#include <stdio.h>
#include "init.h"

//#define MASTER

sbit LED = P1^6; // Led verte embarqu�e sur la carte

sbit NSS = P2^6;

sbit RHT01 = P3^6; // Led verte embarqu�e sur la carte

void fct_tempo(int duree_us);
void clear_data_tab(char data_tab[],int dim);
void send_tab_char_uart0(char car_tab[],int dim);
unsigned int strlen(char *str);
void send_char_uart0(char car);
void Init_UART0(void);
void Init_XBar(void);
void Init_SPI(void);
void wait_one_sec(void);
void send_data_spi(char c);
void wait_500_ms(void);
void rht01_step1(void);
void rht01_step2(void);
void rht01_readvalue(void);
void reverse(char s[]);
void itoa(int n, char s[]);

void rht01_temperature_humidity_parser(char data_value[40]);

unsigned int humidity_integral;
unsigned int humidity_decimal;
unsigned int temperature_integral;
unsigned int temperture_decimal;
unsigned int checksum;

void rht01_step1(void){
	//P2MDOUT |= 0x40; // Push pull mode P2
	RHT01 = 0;
	fct_tempo(30000);
	RHT01 = 1;
	fct_tempo(20);
}
 
void rht01_step2(void){
	 char read_value;
	while(read_value != 0){ // Att passage a 0
		read_value = RHT01;
	}
	while(read_value != 1){ // Att passage a 1
		read_value = RHT01;
	}
	while(read_value != 0){ // Att passage a 0
		read_value = RHT01;
	}
}

char rht01_read_bit_value(void){
	char read_value;
	char return_value;
	while(read_value != 1){ // Att passage a 1
		read_value = RHT01;
	}
	fct_tempo(50); 
	return_value = RHT01;
	while(read_value != 0){ // Att passage a 0
		read_value = RHT01;
	}
	return return_value;
}

void rht01_temperature_humidity_parser(char data_value[40]){
	char internal_counter = 0;
	humidity_integral =0;
	humidity_decimal = 0;
	temperature_integral = 0;
	temperture_decimal = 0;
	checksum = 0;
	while (internal_counter<8){
		humidity_integral <<= 1;
		humidity_integral += data_value[internal_counter]; 
		internal_counter++;
	}
	while (internal_counter<16){
		humidity_decimal <<= 1;
		humidity_decimal += data_value[internal_counter]; 
		internal_counter++;
	}
	while (internal_counter<24){
		temperature_integral <<= 1;
		temperature_integral += data_value[internal_counter]; 
		internal_counter++;
	}
 
	while (internal_counter<32){
		temperture_decimal <<= 1;
		temperture_decimal += data_value[internal_counter];
		internal_counter++;
	}
	while (internal_counter<40){
		checksum <<=1;
		checksum += data_value[internal_counter];
		internal_counter++;
	}

}

/* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }
 
 /* itoa:  convert n to characters in s */
 void itoa(int n, char s[])
 {
     int i, sign;
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}

void Init_XBar(void){
	XBR0 = 0x06;
    XBR2 = 0x40;
}

void clear_data_tab(char data_tab[],int dim){
	char i=0;
	for(i=0;i<dim;i++){
		data_tab[i] = 0 ; 
	}
}
void send_char_uart0(char car){
	 TI0 = 0;
	 SBUF0 = car;
	 while (!TI0);
 }

void send_tab_char_uart0(char car_tab[],int dim){
	unsigned char i = 0;
	for(i=0;i<dim;i++){
		send_char_uart0(car_tab[i]);
	}
 }

void Init_UART0(void){
	CKCON |= 0x10;
	TCON |= 0x40;
	TMOD |= 0x20;
	TMOD &= ~ 0xD0;
	SCON0 |= 0x72;
	SCON0 &= ~ 0x81;
	T2CON &= ~ 0x30;
  	PCON |= 0x80;
	TH1 = 111;
}

void Init_SPI(void){
	SPI0CFG |= 0xC7;
	SPI0CN &= ~0x80;
	SPI0CKR = 0xFF;
	#ifdef MASTER
	SPI0CN |= 0x03;
	P0MDOUT |= 0x14;
	P0MDOUT &= ~0x08;
	P2MDOUT |=0x40; 
	NSS = 1;
	#else
	SPI0CN &= ~0x02;
	SPI0CN |= 0x01;
	P0MDOUT |= 0x04;
	P0MDOUT &= ~0x34; 
	#endif 
}

void wait_one_sec(void){
	unsigned char i=0;
	for(i=0;i<100;i++){
		fct_tempo(10000);
	}
}

void wait_500_ms(void){
	unsigned char i=0;
	for(i=0;i<50;i++){
		fct_tempo(10000);
	}
}

void send_data_spi(char c){
	NSS = 0;
	SPI0DAT = c;
	while((SPI0CN & 0x80) != (0x80)){
		// Wait for flag to be set at 0
	}
	SPI0CN &= ~0x80; // Reset flag
	NSS = 1;
}

void Init_RTH01(void){
	P3MDOUT &= ~0x40;
}

//------------------------------------------------------------------------------------
// MAIN Routine
//------------------------------------------------------------------------------------
void main (void)
{
	#ifdef MASTER
	char i = 0;
	char counter = 0;
	char rth_data_buffer[40];
	char humidity_string[3];
	#endif
	char temp_string[3];
	Init_Device();
	Init_XBar();
	Init_SPI();
	#ifdef MASTER
	Init_RTH01();
	while(1){
		clear_data_tab(rth_data_buffer,40);
		clear_data_tab(temp_string,3);
		clear_data_tab(humidity_string,3);
		rht01_step1();
		rht01_step2();
		 counter = 0;
		 while(counter<40){
		 	rth_data_buffer[counter] = rht01_read_bit_value();
			counter ++;
		 }
		rht01_temperature_humidity_parser(rth_data_buffer);
		send_data_spi(temperature_integral);
		//send_data_spi(humidity_integral);
		wait_one_sec();
	}
	#else
	Init_UART0();
	//P1MDOUT |= 0x40; // Met la sortie 6 en sortie
	while(1){
		if((SPI0CN & 0x80) == (0x80)){
		 temperature_integral = SPI0DAT;
		 send_char_uart0('T');
		 send_char_uart0(':');
		 send_char_uart0(' ');
		 itoa(temperature_integral,temp_string);
		 send_tab_char_uart0(temp_string,3);
		 send_char_uart0('\r');
		 send_char_uart0('\n');
		}	
	}
	#endif

}

 

 