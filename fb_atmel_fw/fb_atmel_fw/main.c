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


int main(void)
{
	atmel_start_init();
	
	uint8_t i2c_buf[RX_LENGTH]; 
	uint8_t rst_cmd[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00}; // min of 50 bits clocked with swdio high
	uint8_t swd_rx_buf[5];
	
	while (1) 
	{
		I2C_read(i2c_buf, RX_LENGTH);
		
		nor_flash_qspi_init();
		
		if(i2c_buf[0] == 0xCC) //start byte CC 
		{
			if(i2c_buf[1] == 0xBB) //target cmd BB
			{
				uint32_t data_addr = i2c_buf[2] << 24 | i2c_buf[3] << 16 | i2c_buf[4] << 8 | i2c_buf[5];
				uint32_t data_len = i2c_buf[6] << 24 | i2c_buf[7] << 16 | i2c_buf[8] << 8 | i2c_buf[9];
				
				//TODO: if data length is bigger than RAM program in chunks
				
				uint8_t qspi_buff[data_len];
				spi_nor_flash_read(SPI_NOR_FLASH_0, qspi_buff, data_addr, data_len); // TODO: test that data is being received
				
				
				
				//begin swd over spi
				
				//TODO: hard reset and sense 
				
				for(int i = 0; i < 3; i++)
				{
					SWD_tx(rst_cmd, 8); // reset device,
				}
				
				SWD_tx(&req_dbg_read_idcode, 1); //TODO: reinit spi -- MOSI data = falling edge / MISO data = rising edge
				SWD_rx(swd_rx_buf, 5); 
				
				if(swd_rx_buf[0] & ACK_MASK) // check ACK
				{
					uint32_t Wdata;
										
					swd_rx_buf[4] = (swd_rx_buf[4]  << 4 )| (swd_rx_buf[3] >> 4); //first byte
					swd_rx_buf[3] = (swd_rx_buf[3]  << 4 )| (swd_rx_buf[2] >> 4); //second byte
					swd_rx_buf[2] = (swd_rx_buf[2]  << 4 )| (swd_rx_buf[1] >> 4); //third byte
					swd_rx_buf[1] = (swd_rx_buf[1]  << 4 )| (swd_rx_buf[0] >> 4); //fourth byte
					
					Wdata = swd_rx_buf[4] << 24 | swd_rx_buf[3] << 16 | swd_rx_buf[2] << 8 | swd_rx_buf[1];
					
					//if(swd_rx_buf[4] & PARITY_MASK) // TODO: check parity, confirm odd number of 1's
					//else confirm even 1's 
					
				}
				//else ACK wait or FAULT was received 
				
				
				//TODO: may need to bitbang 4-5bit ACK read when data following is outbound 
				//NOTE: host must clock for 8 cycles after data phase with swdio low = 0x00
				
				//lower level register write
				//hri_sercomspi_write_DATA_reg(const void *const hw, hri_sercomspi_data_reg_t data);
				
			}
			
		}
		
	}
}
