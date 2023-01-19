// A port of the Mini-C Watchdog example code from https://github.com/free-pdk/simple-pdk-code-examples to the SDCC-based toolchain
// Written by Philipp Klaus Krause in 2019.
// Source code under MIT, like the original.

/* Original Mini-C code
void FPPA0 (void)
{
  .ADJUST_IC  SYSCLK=IHRC/16, IHRC=16MHz, VDD=3.3V

  CLKMD.En_ILRC = 1;                 //enable Internal Low speed RC (required for watchdog to run)
                                     // ILRC runs at around 70kHz +/- 10kHz on PMx154 (depending on VDD and ambient temperature)

  $ MISC = WDT_256K;                 //watchdog timeout is 262144 ILRC periods = apx. 3.8 seconds (Slow_WakeUp,LCD_Disabled,LVR_Enabled)
                                     // -> wdreset must be called at least every 3.8 seconds, otherwise reset is triggered
                                     //    NOTE: ICE watchdog timeouts are completely different, check datasheet

  wdreset;                           //reset watchdog
  CLKMD.En_WatchDog = 1;             //enable watchdog


  PAC.3 = 1;                         //PA.3 as output
  PA.3 = 1;                          //PA.3 high (LED on)

  .delay 1000000;                    //wait 1 second

  wdreset;                           //reset watchdog
  PA.3 = 0;                          //PA.3 low (LED off)

  while(1) {}                        //endless loop which does not 'feeds' the watch 'dog' 
                                     // -> reset will be triggered (cpu will restart, LED will go on again for 1 second and so on...)
                                     //    without watchdog beeing triggered the LED would stay off for evers
}
*/

__sfr __at(0x03) clkmd;
__sfr __at(0x08) misc;
__sfr __at(0x10) pa;
__sfr __at(0x11) pac;

// This function is called before initializing global and static variables.
// Place stuff here that needs to be done immediately after reset (such as disabling a watchdog).
// For small examples it doesn't matter, but for large programs with many global or static variables,
// a watchdog that is active after reset otherwise might bite before main() gets executed.
#if __SDCC_REVISION >= 13762
unsigned char __sdcc_external_startup(void)
#else
unsigned char _sdcc_external_startup(void)
#endif
{
	clkmd = 0x08; // Use IHRC / 16 = 1 Mhz for system clock, disable watchdog.

	return 0; // perform normal initialization
}

void main(void)
{
	misc = 0x03;

__asm
	wdreset
__endasm;

	clkmd |= 0x02; // Enable watchdog

	pac |= 0x08;
	pa |= 0x08;

	for(unsigned long int i = 0; i < 50000; i++); // Wait approx. 1s.

__asm
	wdreset
__endasm;

	pa &= 0xf7;

	while (1);
}

