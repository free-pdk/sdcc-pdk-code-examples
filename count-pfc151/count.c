// A 4-bit counter for the Padauk PFC151, PFC154, PFC161, PFS154 and PFS173, to be compiled with SDCC.
// Counts seconds from 0 to 3 in binary using the 2 LED on the lowest and highest bits of Port A.
// Written by Philipp Klaus Krause 2019-2020.
// Source code under CC0 1.0.

#include <stdbool.h>

__sfr __at(0x03) clkmd;
__sfr __at(0x04) inten;
__sfr __at(0x05) intrq;
__sfr __at(0x0b) ihrcr;
__sfr __at(0x10) pa;
__sfr __at(0x11) pac;
#ifdef __SDCC_pdk15 // PFS173
__sfr __at(0x30) tm2c;
__sfr __at(0x32) tm2s;
__sfr __at(0x33) tm2b;
#else // PFC151, PFC154, PFC161 or PFS154
__sfr __at(0x1c) tm2c;
__sfr __at(0x17) tm2s;
__sfr __at(0x09) tm2b;
#endif

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
#ifdef __SDCC_pdk15
	ihrcr = *((const unsigned char*)(0x8bed)); // Use PFS173 factory calibration value for IHRC at 16 Mhz.
#else
	ihrcr = *((const unsigned char*)(0x87ed)); // Use PFC151 / PFC154 / PFC161 / PFS154 factory calibration value for IHRC at 16 Mhz.
#endif

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

	pac = 0x81;

	for(;;)
	{
		unsigned char s = (clock() / 1000) % 4;
		pa = (s & 1) + ((s & 2) << 6);
	}
}

