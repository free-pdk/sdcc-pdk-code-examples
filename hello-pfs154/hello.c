// "Hello, world!" for the Padauk PFS154, to be compiled with SDCC.
// Source code under CC0 1.0.

// Output on PA0 at 9600 baud.

#include <stdbool.h>
#include <stdio.h>

volatile unsigned char sendcounter;
volatile unsigned int senddata;
volatile bool sending;

__sfr __at(0x03) clkmd;
__sfr __at(0x04) inten;
__sfr __at(0x05) intrq;
__sfr __at(0x10) pa;
__sfr __at(0x11) pac;
__sfr __at(0x1c) tm2c;
__sfr __at(0x17) tm2s;
__sfr __at(0x09) tm2b;

void send_bit(void) __interrupt(0)
{
	if(!(intrq & 0x40))
		return;

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

	sending = true;

	inten |= 0x40;

	return (c);
}

unsigned char _sdcc_external_startup(void)
{
	clkmd = 0x34; // Use IHRC / 2 = 8 Mhz for system clock
	clkmd = 0x30; // Disable ILRC, watchdog

	return 0; // perform normal initialization
}

void main(void)
{
	// Set timer 2 for interrupt at 9600 baud.
	tm2c = 0x10; // Use CLK (8 Mhz)
	tm2s = 0x61; // Divide by 64 ~> 125 kHz
	tm2b = 104;  // Divide by 104 ~> 9600 Hz
	inten = 0x00;
__asm
	engint
__endasm;

	pac = 0x01;

	for(;;)
	{
		printf("Hello World!\n");
		for(unsigned long int i = 0; i < 150000; i++); // Wait approx. 3s.
	}
}

