// A 4-bit counter for the Padauk PFS154 and PFS173, to be compiled with SDCC.
// Written by Philipp Klaus Krause in 2019.
// Source code under CC0 1.0.

#include <stdbool.h>

__sfr __at(0x03) clkmd;
__sfr __at(0x04) inten;
__sfr __at(0x05) intrq;
__sfr __at(0x14) pb;
__sfr __at(0x15) pbc;
__sfr __at(0x1c) tm2c;
__sfr __at(0x17) tm2s;
__sfr __at(0x09) tm2b;

typedef unsigned long int clock_t;
#define CLOCKS_PER_SEC 1000ul

volatile static clock_t clocktime;
volatile static bool clockupdate;

void tick(void) __interrupt(0)
{
	if(!(intrq & 0x40))
		return;

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
	clkmd = 0x3c; // Use IHRC / 2 = 8 Mhz for system clock
	clkmd = 0x38; // Disable ILRC, watchdog

	return 0; // perform normal initialization
}

void main(void)
{
	// Set timer 2 for interrupt at 1 kHz.
	tm2c = 0x10; // Use CLK (8 Mhz) by 2 ~> 4 Mhz
	tm2s = 0x54; // Divide by 16 * 25 ~> 10 kHz
	tm2b = 9;  // Divide by 9 + 1 ~> 1 kHz
	inten = 0x00;
__asm
	engint
__endasm;

	pbc = 0x0f;

	for(;;)
		pb = clock() % 4;
}

