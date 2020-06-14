#include <atmel_start.h>
#include <stdint.h>
#include <stdbool.h>
#include <spi_nor_flash.h>
#include <hal_delay.h>
#include "driver_examples.h"
#include "driver_init.h"
#include "atmel_start_pins.h"
#include "hri_sercom_d51.h"
#include "hpl_spi.h"
#include "swd.h"

uint8_t i2c_buf[RX_LENGTH]; 


int main(void)
{	
	atmel_start_init();
	
	while (1) 
	{
		
		I2C_read(i2c_buf, RX_LENGTH);
		
		nor_flash_qspi_init();
		
		if(i2c_buf[0] == 0xCC) //start byte CC 
		{
			if(i2c_buf[1] == 0xBB) //target cmd BB (data address, data length, target flash address, and chip type to follow, atmel begins programming)
			{
				uint32_t data_addr = i2c_buf[2] << 24 | i2c_buf[3] << 16 | i2c_buf[4] << 8 | i2c_buf[5];
				uint32_t data_len = i2c_buf[6] << 24 | i2c_buf[7] << 16 | i2c_buf[8] << 8 | i2c_buf[9];
				
				//TODO: if data length is bigger than RAM program in chunks
				
				//uint8_t qspi_buff[data_len]; // depleting stack 
				//spi_nor_flash_read(SPI_NOR_FLASH_0, qspi_buff, data_addr, data_len); // TODO: test that data is being received
				
				
				//------------START - SWD VIA SPI------------//
				//TODO: hard reset and sense 
				swd_reset(); 
				uint32_t idcode = swd_read(req_read_idcode);
				uint32_t _idcode = swd_read(req_read_idcode);
				if(idcode == _idcode) // idcode match
				{
					uint32_t ctrl_reg = swd_read(req_read_ctrl_stat);
				}

				//TODO: may need to bitbang 4-5bit ACK read when data following is outbound 
				//NOTE: host must clock for 8 cycles after data phase with swdio low = 0x00
				
				
			}	
		}	
		
	}
}
