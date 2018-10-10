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

#define POWER_CTL 0x2D
#define INT_ENABLE 0x2E  
#define DATA_FORMAT 0x31
#define DATAX0 0x32
#define DATAX1 0x33
#define DATAY0 0x34
#define DATAY1 0x35
#define DATAZ0 0x36
#define DATAZ1 0x37

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
char send_data_spi(char c,char enable_cs);
void wait_500_ms(void);
void reverse(char s[]);
void itoa(int n, char s[]);
char send_cmd_adxl(char rw,char mb,char adress,char data_value,char get_data_return);
char read_register_adxl(char addr);
void write_register_adxl(char addr,char data_value);
void create_xyz_adxl_vector(char data_value[6], int* x, int* y, int* z);

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
	SPI0CKR = 0xFE;
	SPI0CN |= 0x03;
	P0MDOUT |= 0x14;
	P0MDOUT &= ~0x08;
	P2MDOUT |=0x40; 
	NSS = 1;
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

char send_data_spi(char c,char enable_cs){
	char return_value;
	NSS = 0;
	SPI0DAT = c;
	while((SPI0CN & 0x80) != (0x80)){
		// Wait for flag to be set at 0
	}
	SPI0CN &= ~0x80; // Reset flag
	return_value = SPI0DAT;
	if(enable_cs>0) {
		NSS = 1;
	}
	return return_value;
}

char send_cmd_adxl(char rw,char mb,char adress,char data_value,char get_data_return){
	unsigned char msb = 0;
	char return_value = 0;
	char temp_value = 0;
	if(rw==0){
		msb = 0x80; 
	}else{
		msb = 0;
	}
	if(mb==1){
		msb |= 0x40;
	}
	msb |= adress;
	send_data_spi(msb,0);
	if(data_value!=0){
		temp_value = send_data_spi(data_value,0);
	}
	if(get_data_return>0){
		SPI0DAT = 0xFF; // Send fake data on SPI bus
		while((SPI0CN & 0x80) != (0x80)){
			// Wait for flag to be set at 0
		}
		SPI0CN &= ~0x80; // Reset flag
		return_value = SPI0DAT;
	}else{
		return_value = temp_value;
	}

	NSS = 1;
	return return_value;
}

char get_data_adxl(char adress){
	return send_cmd_adxl(0,0,adress,0,1);
}

char read_register_adxl(char addr){
	char return_value = 0;
	char addr_tram = 0x80; // R/W bit set to 1
	addr_tram |= addr; 
	NSS = 0;
	// 1st transaction => send addr_tram to ADXL
	SPI0DAT = addr_tram;
	while((SPI0CN & 0x80) != (0x80)){
		// Wait for flag to be set at 0
	}
	SPI0CN &= ~0x80; // Reset flag
	//2nd transaction=> receive data
	SPI0DAT=0xAA;
	while((SPI0CN & 0x80) != (0x80)){
		// Wait for flag to be set at 0
	}
	SPI0CN &= ~0x80; // Reset flag
	return_value = SPI0DAT;
	NSS = 1;
	return return_value;
}

void write_register_adxl(char addr,char data_value){
	char addr_tram = addr;
	addr_tram &= ~0x80; // Force R/W bit to 0
	NSS = 0;

	// 1st transaction => send addr_tram to ADXL
	SPI0DAT = addr_tram;
	while((SPI0CN & 0x80) != (0x80)){
		// Wait for flag to be set at 0
	}
	SPI0CN &= ~0x80; // Reset flag

	//2nd transaction=> send data
	SPI0DAT=data_value;
	while((SPI0CN & 0x80) != (0x80)){
		// Wait for flag to be set at 0
	}
	SPI0CN &= ~0x80; // Reset flag

	NSS = 1;
}

void Init_ADXL(void){
	fct_tempo(1100); // Wait 1,1 ms
	write_register_adxl(DATA_FORMAT,0x0b);
	write_register_adxl(POWER_CTL,0x08);
	write_register_adxl(INT_ENABLE,0x80);
}

void create_xyz_adxl_vector(char data_value[6], int* x, int* y, int* z){
	*x = data_value[1]<<8;
	*x |= data_value[0];
	*y = data_value[3]<<8;
	*y |= data_value[2];
	*z = data_value[5]<<8;
	*z |= data_value[4];
}



//------------------------------------------------------------------------------------
// MAIN Routine
//------------------------------------------------------------------------------------
void main (void)
{
	unsigned char bcl=0;
	unsigned char data_axdl[6];
	int x = 0;
	int y = 0;
	int z = 0;
	unsigned char data_x_string[3];
	unsigned char data_y_string[3];
	unsigned char data_z_string[3];
	Init_Device();
	Init_XBar();
	Init_SPI();
	Init_ADXL();
	Init_UART0();
	while(1){
		clear_data_tab(data_axdl,6);
		clear_data_tab(data_x_string,3);
		clear_data_tab(data_y_string,3);
		clear_data_tab(data_z_string,3);
		bcl=0;
		
		data_axdl[0] = read_register_adxl(DATAX0);
		data_axdl[1]  = read_register_adxl(DATAX1);
		data_axdl[2]  = read_register_adxl(DATAY0);
		data_axdl[3]  = read_register_adxl(DATAY1);
		data_axdl[4]  = read_register_adxl(DATAZ0);
		data_axdl[5]  = read_register_adxl(DATAZ1);

		create_xyz_adxl_vector(data_axdl,&x,&y,&z);

		itoa(x,data_x_string);
		itoa(y,data_y_string);
		itoa(z,data_z_string);

		send_tab_char_uart0(data_x_string,3);
		send_char_uart0(' ');
		send_char_uart0('-');
		send_char_uart0(' ');

		send_tab_char_uart0(data_y_string,3);
		send_char_uart0(' ');
		send_char_uart0('-');
		send_char_uart0(' ');

		send_tab_char_uart0(data_z_string,3);

		send_char_uart0('\r');
		send_char_uart0('\n');
		wait_500_ms();

	}
}

 

 