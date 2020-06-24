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
		
		if(i2c_buf[0] == start_byte) //start byte CC 
		{
			if(i2c_buf[1] == target_cmd) //target cmd BB (data address, data length, target flash address, and SOC type to follow, atmel begins programming)
			{
				uint32_t data_addr = i2c_buf[2] << 24 | i2c_buf[3] << 16 | i2c_buf[4] << 8 | i2c_buf[5];
				uint32_t data_len = i2c_buf[6] << 24 | i2c_buf[7] << 16 | i2c_buf[8] << 8 | i2c_buf[9];
				//flash start address
				//SOC type
				
				//TODO: if data length is bigger than RAM program in chunks
				
				uint8_t qspi_buff[data_len]; 
				spi_nor_flash_read(SPI_NOR_FLASH_0, qspi_buff, data_addr, data_len); 
				
				//------------START - SWD VIA SPI------------//
				
				//target_reset(false); //hard reset needed for non-debug flash write? 
								
				jtag_to_swd();			
				uint32_t idcode = swd_read(req_read_idcode); 
				if(idcode == 0x00000000 || idcode == 0xFFFFFFFF)
				{
					i2c_send_error(error_no_target);
				}
		
				uint32_t ctrl_reg = swd_read(req_read_ctrl_stat); // should return 0x00
		
				swd_write(req_write_ctrl_stat, SYS_DBG_PWRUP); // pwrup debug and sys
				ctrl_reg = swd_read(req_read_ctrl_stat);
				if((ctrl_reg & PWRUP_MASK) != PWRUP_MASK) // check pwrup status
				{
					i2c_send_error(error_no_dbg_pwr);
				}
				
				swd_write(req_write_csw, CSW_AUTOINC_OFF);
				swd_write(req_write_select, AP_PORT_F);//select AP port 0
				uint32_t idr_value = swd_read(req_read_idr);
				if(idr_value == IDR_DEBUG_LOCKED)
				{
					i2c_send_error(error_dbg_locked);
				}
				
				//once programming begins send progress bytes 1-124
				
				//NOTE: Driving the line low results in a small current drain at target
				
				
			}	
		}	
	}
}
