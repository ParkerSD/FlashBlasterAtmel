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


int main(void)
{	
	uint32_t ret; 
	atmel_start_init();
	
	QUAD_SPI_0_PORT_deinit(); //float qspi pins, if atmel is not held in reset
	
	//TIMER_0_example(); //shutoff timer not working
	
	while (gpio_get_pin_level(BOOT_PIN)) 
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
					system_off();  //NOTE: red light indicates atmel is in off state
				}
			}
		}
	}
	
	system_off();
	
}
