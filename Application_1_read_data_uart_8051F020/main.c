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


sbit LED = P1^6; // Led verte embarquée sur la carte

void fct_tempo(int duree_us);

//------------------------------------------------------------------------------------
// MAIN Routine
//------------------------------------------------------------------------------------
void main (void)
{
	unsigned char i = 0;
  Init_Device();
  P1 |= 0x40; 
  while (1)
  {
		LED = 1;
		for(i=0;i<20;i++){
			fct_tempo(30000);
		}
		SBUF0 = 'a';
		LED = 0;
				for(i=0;i<20;i++){
			fct_tempo(30000);
		}
	}	 	 
}

 

 