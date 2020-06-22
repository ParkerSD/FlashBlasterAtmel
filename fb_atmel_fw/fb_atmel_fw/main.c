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
			if(i2c_buf[1] == target_cmd) //target cmd BB (data address, data length, target flash address, and chip type to follow, atmel begins programming)
			{
				uint32_t data_addr = i2c_buf[2] << 24 | i2c_buf[3] << 16 | i2c_buf[4] << 8 | i2c_buf[5];
				uint32_t data_len = i2c_buf[6] << 24 | i2c_buf[7] << 16 | i2c_buf[8] << 8 | i2c_buf[9];
				
				//TODO: if data length is bigger than RAM program in chunks
				
				uint8_t qspi_buff[data_len]; 
				spi_nor_flash_read(SPI_NOR_FLASH_0, qspi_buff, data_addr, data_len); // TODO: test that data is being received
				
				//------------START - SWD VIA SPI------------//
				
				//target_reset();
				//swd_reset(); 
				//jtag_to_swd(); 
				
				
				swd_reset();
				uint32_t idcode = swd_read(req_read_idcode); // Error here, //NOTE: if sync lost (ex: no stop bit) is leaves line undriven and waits for host to retry (after some clk cycles w/ line low), or signals a reset by not driving line 
				if(idcode == 0x00000000 || idcode == 0xFFFFFFFF)
				{
					uint8_t error_packet[3] = {start_byte, error_cmd, error_no_target};
					I2C_write(error_packet, 3);
				} 
				
				//swd_reset(); 
				uint32_t _idcode = swd_read(req_read_idcode); // immediate xfer second read not working, try host holding line low after between xfers or 50 clk cycle reset followed by read-id as new re-connect sequence											
				if(idcode == _idcode) // idcode match - neither value should be 0x00
				{
					uint32_t ctrl_reg = swd_read(req_read_ctrl_stat);
				}
				
				
				//TODO: may need to bitbang 4-5bit ACK read when data following is outbound 
				
				//NOTE: After any data xfer, host must clock for 8 cycles after data phase with swdio low = 0x00
				// or immediately start a new SW-DP operation 
				// or continue to clock until next operation 
				
				//NOTE: Driving the line low results in a small current drain at target
				
				
			}	
		}	
	}
}
