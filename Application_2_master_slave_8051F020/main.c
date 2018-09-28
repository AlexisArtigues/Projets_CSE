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

#define MASTER

sbit LED = P1^6; // Led verte embarqu�e sur la carte

sbit NSS = P2^6;

void fct_tempo(int duree_us);
void clear_data_tab(char data_tab[],int dim);
void send_tab_char_uart0(char car_tab[],int dim);
unsigned int strlen(char *str);
void send_char_uart0(char car);
void itoa(int n, char s[]);
void Init_UART0(void);
void Init_XBar(void);
void Init_SPI(void);
void wait_one_sec(void);

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
//------------------------------------------------------------------------------------
// MAIN Routine
//------------------------------------------------------------------------------------
void main (void)
{
	Init_Device();
	Init_XBar();
	Init_UART0();
	Init_SPI();
	while(1){
		SPI0DAT = 0xFF;
		wait_one_sec();
	}
}

 

 