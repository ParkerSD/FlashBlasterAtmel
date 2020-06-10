#include <atmel_start.h>
#include <stdint.h>
#include <stdbool.h>
#include <spi_nor_flash.h>
#include <hal_delay.h>
#include "driver_examples.h"
#include "driver_init.h"
#include "atmel_start_pins.h"


int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	SWD_GPIO_init();
	
	uint8_t buffer[RX_LENGTH]; 
	
	while (1) 
	{
		
		/*
		for(int i = 0; i<1000; i++)
		{
			gpio_set_pin_level(SWCLK, false);
			delay_ms(50);
			gpio_set_pin_level(SWCLK, true);
			delay_ms(50);
		}
		*/
		

		
		I2C_read(buffer, RX_LENGTH);
		
		nor_flash_qspi_init();
		
		if(buffer[0] == 0xCC) //start byte CC 
		{
			if(buffer[1] == 0xBB) //target cmd BB
			{
				uint32_t data_addr = buffer[2] << 24 | buffer[3] << 16 | buffer[4] << 8 | buffer[5]; 
				uint32_t data_len = buffer[6] << 24 | buffer[7] << 16 | buffer[8] << 8 | buffer[9];
				//if data length is bigger than RAM program in chunks
				
				uint8_t qspi_buff[data_len];
				spi_nor_flash_read(SPI_NOR_FLASH_0, qspi_buff, data_addr, data_len);
				
				//bit bang swd over gpio 
				
			}
			
		}
		
		
	}
}
