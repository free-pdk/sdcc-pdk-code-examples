// A 2-bit counter for the Padauk PFC161, to be compiled with SDCC.
// Counts seconds from 0 to 3 in binary using the 4 LED on the lowest bit of port b and highest bit of port A.
// Written by Philipp Klaus Krause 2019-2020.
// Source code under CC0 1.0.

#include <stdbool.h>
#include <stdint.h>

__sfr __at(0x03) clkmd;
__sfr __at(0x04) inten;
__sfr __at(0x05) intrq;
__sfr __at(0x0b) ihrcr;
__sfr __at(0x10) pa;
__sfr __at(0x11) pac;
__sfr __at(0x14) pb;
__sfr __at(0x15) pbc;
__sfr __at(0x1c) tm2c;
__sfr __at(0x1e) tm2s;
__sfr __at(0x1f) tm2b;

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

#if __SDCC_REVISION >= 13762
unsigned char __sdcc_external_startup(void)
#else
unsigned char _sdcc_external_startup(void)
#endif
{
	//ihrcr = *((const unsigned char*)(0x87ed)); // Use PFS154 factory calibration value for IHRC at 16 Mhz.

	//clkmd = 0x34; // Use IHRC / 2 = 8 Mhz for system clock, disable watchdog.
	//clkmd = 0x30; // Disable ILRC

	return 0; // perform normal initialization
}

void main(void)
{
	// Set timer 2 for interrupt at 1 kHz.
	//tm2c = 0x10; // Use CLK (8 Mhz)
	//tm2s = 0x54; // Divide by 16 * 25 ~> 20 kHz
	//tm2b = 19;  // Divide by 19 + 1 ~> 1 kHz
	//inten = 0x40;

__asm
	engint
__endasm;

	pbc = 0x01;
	pac = 0x80;pbc= 0xff; pac = 0xff;
pa = 0xff; pb = 0xff;
	for(;;)
	{
		/*uint_fast8_t s = (clock() / 1000) % 4;
		pb = s & 0x01;
		pa = s << 6 & 0x80;*/
	}
}

