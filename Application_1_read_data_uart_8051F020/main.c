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

sbit LED = P1^6; // Led verte embarqu�e sur la carte

sbit RHT01 = P2^6; // Led verte embarqu�e sur la carte

void fct_tempo(int duree_us);
void clear_data_tab(char data_tab[],int dim);
void send_tab_char_uart0(char car_tab[],int dim);
void rht01_step1(void);
void rht01_step2(void);
void rht01_readvalue(void);
unsigned int strlen(char *str);
void send_char_uart0(char car);
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
		humidity_integral += data_value[internal_counter]; 
		humidity_integral <<= 1;
		internal_counter++;
	}
	while (internal_counter<16){
		humidity_decimal += data_value[internal_counter]; 
		humidity_decimal <<= 1;
		internal_counter++;
	}
	while (internal_counter<24){
		temperature_integral += data_value[internal_counter]; 
		temperature_integral <<= 1;
		internal_counter++;
	}
 
	while (internal_counter<32){
		temperture_decimal += data_value[internal_counter];
		temperture_decimal <<= 1;
		internal_counter++;
	}
	while (internal_counter<40){
		checksum += data_value[internal_counter];
		checksum <<=1;
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

unsigned int strlen(char *str) {
	unsigned int i = 0;
	
	while (str[i] != '\0') {
	i++;
	}
	return i;
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

//------------------------------------------------------------------------------------
// MAIN Routine
//------------------------------------------------------------------------------------
void main (void)
{
	char i = 0;
	char counter = 0;
	char rth_data_buffer[40];
	char temp_string[3];
	char humidity_string[3];
	Init_Device();
	P2MDOUT &= ~0x40;
	while(1){
		clear_data_tab(rth_data_buffer);
		clear_data_tab(temp_string);
		clear_data_tab(humidity_string);
		rht01_step1();
		rht01_step2();
		 counter = 0;
		 while(counter<40){
		 	rth_data_buffer[counter] = rht01_read_bit_value();
			counter ++;
		 }
		 rht01_temperature_humidity_parser(rth_data_buffer);
		 send_char_uart0('T');
		 send_char_uart0(':');
		 send_char_uart0(' ');
		 itoa(temperature_integral,temp_string);
		 send_tab_char_uart0(temp_string,3);
		 send_char_uart0(' ');
		 send_char_uart0('H');
		 send_char_uart0('-');
		 send_char_uart0(' ');
		 send_char_uart0(':');
		 itoa(humidity_integral,humidity_string);
		 send_tab_char_uart0(humidity_string,3);
		 send_char_uart0('\r');
		for(i=0;i<100;i++){
			fct_tempo(10000);
		}
	}
}

 

 