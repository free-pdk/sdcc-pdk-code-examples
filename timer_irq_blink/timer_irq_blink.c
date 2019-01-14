// A port of the Mini-C TimerIrqBlink example code from https://github.com/free-pdk/simple-pdk-code-examples to the SDCC-based toolchain
// Written by Philipp Klaus Krause in 2019.
// Source code under MIT, like the original.

/* Original Mini-C code
void FPPA0 (void)
{
	.ADJUST_IC	SYSCLK=IHRC/16		 // SYSCLK=IHRC/16 = 1MHz

	PAC.3 = 1;                       // set PA.3 as output

	$ T16M SYSCLK, /64, BIT12;       // timer base = SYSCLK/64 = 15625 Hz
                                     // generate INTRQ.T16 when overflow to bit 12 (4096)
	                                 // first interrupt triggered after 4096 cycles (13 bit 0 change to 1)
	                                 // next interrupt after next 8192 cycles (13 bit 1 needs to change to 0 first and then back to 1)

	WORD tmp = 0;                    // store 0 to timer value (first run)
    stt16 tmp;
	
	$ INTEN T16;                     // enable T16 interrupt

	INTRQ = 0;                       // clear any pending interrupt requests
	ENGINT;                          // enable global interrupts

	while (1) { }                    // endless loop (empty)
}


void Interrupt(void)
{
	PUSHAF;                          // save A and FLAGs

	if( INTRQ.T16 )                  // interrupt source = timer T16? 
	{
		PA ^= (1<<3);                // toggle bit of PA.3 output
		INTRQ.T16 = 0;               // clear interrupt request for T16
	}

	POPAF;                           // restore A and FLAGs
}
*/

__sfr __at(0x03) clkmd;
__sfr __at(0x04) inten;
__sfr __at(0x05) intrq;
__sfr __at(0x05) t16m;
__sfr __at(0x10) pa;
__sfr __at(0x11) pac;
__sfr16 t16cnt;

#define INT_T16 0x04

unsigned char _sdcc_external_startup(void)
{
	clkmd = 0x08; // Use IHRC / 16 = 1 Mhz for system clock, disable watchdog.

	return 0; // perform normal initialization
}

void main(void)
{
	pac |= (1 << 3);

	t16m = (1 << 5) | (3 << 3) | 4;

	t16cnt = 0;

	inten |= INT_T16;

	intrq = 0;

__asm
	engint
__endasm;

	while(1);
}

void handler(void) __interrupt
{
	if (intrq & INT_T16)
	{
		pa ^= (1 << 3);
                intrq &= ~INT_T16;
	}
}

