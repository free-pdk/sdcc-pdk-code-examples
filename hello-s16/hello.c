// "Hello, world!" for the Padauk PFC154, PFC232, PFS132, PFS154, PFS173, PGS134, and PGS152, to be compiled with SDCC.
// Repeatedly outputs the string "Hello, World!" at 9600 baud, 1 stop bit, no parity on pin 0 of Port A.
// Written by Philipp Klaus Krause 2019-2020.
// Source code under CC0 1.0.

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
#ifdef __SDCC_pdk15 // PFS173, or PGS134.
__sfr __at(0x30) tm2c;
__sfr __at(0x31) tm2ct;
__sfr __at(0x32) tm2s;
__sfr __at(0x33) tm2b;
#else // PFC154, PFC232, PFS132, PFS154, or PGS152.
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

#if __SDCC_REVISION >= 13762
unsigned char __sdcc_external_startup(void)
#else
unsigned char _sdcc_external_startup(void)
#endif
{
#ifdef __SDCC_pdk15
	ihrcr = *((const unsigned char*)(0x8bed)); // Use PFS173 factory calibration value for IHRC at 16 Mhz.
#else
	ihrcr = *((const unsigned char*)(0x87ed)); // Use PFC154 / PFC232 / PFS154 factory calibration value for IHRC at 16 Mhz.
#endif

	clkmd = 0x34; // Use IHRC / 2 = 8 Mhz for system clock, disable watchdog.
	clkmd = 0x30; // Disable ILRC

	return 0; // perform normal initialization
}

void main(void)
{
	// Set timer 2 for interrupt for 9600 baud.
	tm2c = 0x10; // Use CLK (8 Mhz)
	tm2s = 0x06; // Divide by 6 + 1 ~> 1142857 Hz
	tm2b = 118;  // Divide by 118 + 1 ~> 9604 Hz
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

