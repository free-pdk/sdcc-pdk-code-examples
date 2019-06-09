// "Hello, world!" for the Padauk PFS154, to be compiled with SDCC.
// Repeatedly outputs the string "Hello, World!" at 2400 baud on pin 0 of Port A.
// Written by Philipp Klaus Krause in 2019.
// Source code under CC0 1.0.

// Output on PA0 at 1200 baud.

#include <stdbool.h>
#include <stdio.h>

volatile unsigned char sendcounter;
volatile unsigned int senddata;
volatile bool sending;

__sfr __at(0x03) clkmd;
__sfr __at(0x04) inten;
__sfr __at(0x05) intrq;
__sfr __at(0x0b) ihrcr;
__sfr __at(0x10) pa;
__sfr __at(0x11) pac;
#ifdef __SDCC_pdk15
__sfr __at(0x30) tm2c;
__sfr __at(0x31) tm2ct;
__sfr __at(0x32) tm2s;
__sfr __at(0x33) tm2b;
#else
__sfr __at(0x1c) tm2c;
__sfr __at(0x1d) tm2ct;
__sfr __at(0x17) tm2s;
__sfr __at(0x09) tm2b;
#endif

void send_bit(void) __interrupt(0)
{
	// Reset interrupt request, proceed only if we had a timer interrupt.
	if(!(intrq & 0x40))
	{
		intrq = 0x00;
		return;
	}
	intrq = 0x00;

	if(!sending)
		return;

	pa = senddata & 1;
	senddata >>= 1;

	if(!--sendcounter)
	{
		sending = false;

		inten &= ~0x40;
	}
}

int putchar(int c)
{
	while(sending);

	senddata = (c << 1) | 0x200;

	sendcounter = 10;

	tm2ct = 0;

	sending = true;

	inten |= 0x40;

	return (c);
}

unsigned char _sdcc_external_startup(void)
{
#ifdef __SDCC_pdk15
	ihrcr = *((const unsigned char*)(0x8bed)); // Use PFS173 factory calibration value for IHRC at 16 Mhz.
#else
	ihrcr = *((const unsigned char*)(0x87ed)); // Use PFS154 factory calibration value for IHRC at 16 Mhz.
#endif

	clkmd = 0x34; // Use IHRC / 2 = 8 Mhz for system clock, disable watchdog.
	clkmd = 0x30; // Disable ILRC

	return 0; // perform normal initialization
}

void main(void)
{
	// Set timer 2 for interrupt for 1200 baud.
	tm2c = 0x10; // Use CLK (8 Mhz)
	tm2s = 0x40; // Divide by 16 ~> 500 kHz
	tm2b = 207;  // Divide by 207 + 1 ~> 2403 Hz
	inten = 0x40;
__asm
	engint
__endasm;

	pac = 0x01;

	for(;;)
	{
		printf("Hello, World!\n");
		for(unsigned long int i = 0; i < 150000; i++); // Wait approx. 3s.
	}
}

