// A 2-bit counter for the Padauk PFS172, to be compiled with SDCC.
// Counts seconds from 0 to 3 in binary using the 4 LED on the second-lowest and highest bit of port B.
// Written by Philipp Klaus Krause 2019-2020.
// Source code under CC0 1.0.

#include <stdbool.h>
#include <stdint.h>

__sfr __at(0x03) clkmd;
__sfr __at(0x04) inten;
__sfr __at(0x05) intrq;
__sfr __at(0x0b) ihrcr;
__sfr __at(0x15) pb;
__sfr __at(0x16) pbc;

typedef unsigned long int clock_t;
#define CLOCKS_PER_SEC 1000ul

volatile static clock_t clocktime;
volatile static bool clockupdate;

void tick(void) __interrupt(0)
{
	if(!(intrq & 0x40))
		return;
	intrq = 0x00;

	clocktime++;
	clockupdate = true;
}

clock_t clock(void)
{
	clock_t ctmp;

	do
	{
		clockupdate = false;
		ctmp = clocktime;
	} while (clockupdate);
	
	return(ctmp);
}

unsigned char _sdcc_external_startup(void)
{
	ihrcr = *((const unsigned char*)(0x87ed)); // Use PFS154 factory calibration value for IHRC at 16 Mhz.

	clkmd = 0x34; // Use IHRC / 2 = 8 Mhz for system clock, disable watchdog.
	clkmd = 0x30; // Disable ILRC

	return 0; // perform normal initialization
}

void main(void)
{
	// Set timer 2 for interrupt at 1 kHz.
	tm2c = 0x10; // Use CLK (8 Mhz)
	tm2s = 0x54; // Divide by 16 * 25 ~> 20 kHz
	tm2b = 19;  // Divide by 19 + 1 ~> 1 kHz
	inten = 0x40;

__asm
	engint
__endasm;

	pbc = 0x82;

	for(;;)
	{
		uint_fast8_t s = (clock() / 1000) % 4;
		pb = (s << 6 & 0x80) | (s << 1 & 0x01);
	}
}

