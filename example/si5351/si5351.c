#include <8051.h>
#include <stdint.h>

#include "i2c.h"
#include "si5351.h"

      
#define BOOT_ADDR  0x3800

/* This function provided a way to the internal bootloader */
void jump_to_bootloader()
{
	EA = 0;/* Disable all interrupts */
	
	__asm
		LJMP BOOT_ADDR /* Jump to bootloader */
	__endasm;	
	while(1); 
}


bool si5351_write(uint8_t reg_address, uint8_t value)
{
	i2c_start();
	i2c_write(SI5351_AR);
	if(i2c_read_nak())
		goto fault;
		
	i2c_write(reg_address);
	if(i2c_read_nak())
		goto fault;
		
	i2c_write(value);
	if(i2c_read_nak())
		goto fault;
			
	i2c_stop();
	return 1;
fault:
	i2c_stop();
	return 0;
}

bool si5351_init()
{
	si5351a_revb_register_t __code *pair;
	int i;
	for(i = 0; i < SI5351A_REVB_REG_CONFIG_NUM_REGS; i++)
	{
		pair = &si5351a_revb_registers[i];
		if(!si5351_write(pair->address & 0xff, pair->value))
			return 0;
	}
	return 1;
}


void main( void ) 
{
	i2c_init();
	si5351_init();
	jump_to_bootloader();
}
