#include <8051.h>
      
#define BOOT_ADDR  0x3800

/* This program just execute bootloader rom directly */
 
void main( void ) 
{
	EA = 0;/* Disable all interrupts */
	
	__asm
		LJMP BOOT_ADDR /* Jump to bootloader */
	__endasm;
	while(1); 
}
