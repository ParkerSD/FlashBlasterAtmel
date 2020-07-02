#include <atmel_start.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <spi_nor_flash.h>
#include <hal_delay.h>
#include "driver_examples.h"
#include "driver_init.h"
#include "atmel_start_pins.h"
#include "hri_sercom_d51.h"
#include "hpl_spi.h"
#include "swd.h"
#include "hpl_spi.h"
#include "flash.h"


uint8_t i2c_buf[RX_LENGTH]; 

void sleep_standby(void) //~1.5mA draw 
{
	PM->SLEEPCFG.bit.SLEEPMODE = PM_SLEEPCFG_SLEEPMODE_STANDBY;
	while(PM->SLEEPCFG.bit.SLEEPMODE != PM_SLEEPCFG_SLEEPMODE_STANDBY);
	_go_to_sleep();
}

void sleep_hibernate(void)
{
	PM->SLEEPCFG.bit.SLEEPMODE = PM_SLEEPCFG_SLEEPMODE_HIBERNATE;
	while(PM->SLEEPCFG.bit.SLEEPMODE != PM_SLEEPCFG_SLEEPMODE_HIBERNATE);
	_go_to_sleep();
}

void system_off(void) // < 1mA draw
{
	PM->SLEEPCFG.bit.SLEEPMODE = PM_SLEEPCFG_SLEEPMODE_OFF;
	while(PM->SLEEPCFG.bit.SLEEPMODE != PM_SLEEPCFG_SLEEPMODE_OFF);
	_go_to_sleep();	
}

int main(void)
{	
	uint32_t ret; 
	atmel_start_init();
					
	//sleep_standby(); 
	
	/* //nordic resets atmel 
	if(!gpio) //pin low
	{
		system_off();
	}
	else if(gpio) //pin high -- redundant/not needed 
	{
		continue;
	}
	*/
	
	while (1) 
	{
		I2C_read(i2c_buf, RX_LENGTH);
				
		if(i2c_buf[0] == start_byte) //start byte CC 
		{
			if(i2c_buf[1] == target_cmd) //target cmd BB (data address, data length, target flash address, and SOC type to follow, atmel begins programming)
			{
				uint32_t data_addr = i2c_buf[2] << 24 | i2c_buf[3] << 16 | i2c_buf[4] << 8 | i2c_buf[5];
				uint32_t data_len = i2c_buf[6] << 24 | i2c_buf[7] << 16 | i2c_buf[8] << 8 | i2c_buf[9];
				uint32_t start_address = i2c_buf[10] << 24 | i2c_buf[11] << 16 | i2c_buf[12] << 8 | i2c_buf[13]; //flash start address
				uint32_t chip_type = i2c_buf[14] << 24 | i2c_buf[15] << 16 | i2c_buf[16] << 8 | i2c_buf[17];
				
				ret = flash_target(data_addr, data_len, chip_type, start_address);
				if(ret == f_ok || ret == f_err)
				{
					//sleep_standby();
				}
		
			}	
		}	
	}
}
