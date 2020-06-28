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
				uint32_t start_address = i2c_buf[10] << 24 | i2c_buf[11] << 16 | i2c_buf[12] << 8 | i2c_buf[13]; //flash start address
				uint32_t chip_type = i2c_buf[14] << 24 | i2c_buf[15] << 16 | i2c_buf[16] << 8 | i2c_buf[17];
				
				//TODO: BRANCH ON CHIP TYPE - swd_flash(chip_type, start_address, data_len, data_addr)
				//TODO: if data length is bigger than RAM program in chunks
				
				uint8_t* qspi_buff = malloc(sizeof(data_len)); //data_len
				spi_nor_flash_read(SPI_NOR_FLASH_0, qspi_buff, data_addr, data_len); //data_len
				
				//------------START - SWD VIA SPI------------//
				// target_reset(false); //hard reset needed for non-debug flash write? 
								
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
				swd_write(req_write_select, AP_PORT_F);// select AP port 0
				swd_read(req_read_idr);
				uint32_t idr_value = swd_read(req_read_idr);
				if(idr_value == IDR_DEBUG_LOCKED)
				{
					i2c_send_error(error_dbg_locked);
				}
				
				
				//halt core
				swd_clear_abort_reg();
				swd_write(req_write_select, AP_PORT_0);
				swd_write(req_write_tar, _DHCSR);
				swd_write(req_write_csw, CSW_AUTOINC_ON);
				swd_write(req_write_drw, HALT_CORE);
				
				//enable halt on reset
				swd_clear_abort_reg();
				swd_write(req_write_select, AP_PORT_0);
				swd_write(req_write_tar, _DEMCR);
				swd_write(req_write_csw, CSW_AUTOINC_ON);
				swd_write(req_write_drw, EN_HALT_ON_RST);
				
				//reset core
				swd_clear_abort_reg();
				swd_write(req_write_select, AP_PORT_0);
				swd_write(req_write_tar, _AIRCR);
				swd_write(req_write_csw, CSW_AUTOINC_ON);
				swd_write(req_write_drw, RESET_CORE);
				
				//below nrf52840 specific 
				swd_clear_abort_reg();
				swd_write(req_write_select, AP_PORT_0);
				swd_write(req_write_tar, NRF52_NVMC_CONFIG);
				swd_write(req_write_csw, CSW_AUTOINC_ON);
				swd_write(req_write_drw, ERASE_ENABLE); //enable erase cmd
				
				swd_clear_abort_reg();
				swd_write(req_write_select, AP_PORT_0);
				swd_write(req_write_tar, NRF52_NVMC_ERASE_ALL);
				swd_write(req_write_csw, CSW_AUTOINC_ON);
				swd_write(req_write_drw, ERASE_ALL); //erase all cmd
				delay_ms(200); //should take 167ms to erase all 
				
				
				swd_clear_abort_reg();
				swd_write(req_write_select, AP_PORT_0);
				swd_write(req_write_tar, NRF52_NVMC_READY);
				swd_write(req_write_csw, CSW_AUTOINC_OFF);
				swd_read(req_read_drw);
				uint32_t ready_val = swd_read(req_read_drw);
				while(!ready_val) //not ready
				{
					delay_ms(50);
					ready_val = swd_read(req_read_drw);
				}
				
				swd_clear_abort_reg();
				swd_write(req_write_select, AP_PORT_0);
				swd_write(req_write_tar, NRF52_NVMC_CONFIG);
				swd_write(req_write_csw, CSW_AUTOINC_ON);
				swd_write(req_write_drw, WRITE_ENABLE); //enable write cmd
				
								
				uint32_t progress_track = 0;
				uint32_t progress_chunk = (data_len/WORD_SIZE)/124; 
				uint8_t packet_count = 0;
				
				for(int i=0; i<(data_len/WORD_SIZE); i++) //will there be remainder / non-full word sent?
				{
					
					uint32_t word = *(qspi_buff +(i+3)) << 24 | *(qspi_buff + (i+2)) << 16 | *(qspi_buff + (i+1)) << 8 | *(qspi_buff + i); //LSB
					
					swd_clear_abort_reg();
					swd_write(req_write_select, AP_PORT_0);
					swd_write(req_write_tar, start_address + (i*WORD_SIZE)); //set flash address
					swd_write(req_write_csw, CSW_AUTOINC_OFF);
					swd_write(req_write_drw, word); 
					
					
					swd_clear_abort_reg();
					swd_write(req_write_select, AP_PORT_0);
					swd_write(req_write_tar, NRF52_NVMC_READY_NEXT);
					swd_write(req_write_csw, CSW_AUTOINC_OFF);
					swd_read(req_read_drw);
					uint32_t ready_val = swd_read(req_read_drw);
					while(!ready_val) //not ready
					{
						delay_ms(1);
						ready_val = swd_read(req_read_drw);
					}
					
					
					/*
					progress_track++;
					if(progress_track >= progress_chunk) //chunk size reached notify nordic	
					{
						i2c_send_progress(packet_count);//send progress to nordic in 124 increments
						packet_count++; 
						progress_track = 0;
					}		
					*/
				}
				
				//reset core
				swd_clear_abort_reg();
				swd_write(req_write_select, AP_PORT_0);
				swd_write(req_write_tar, _AIRCR);
				swd_write(req_write_csw, CSW_AUTOINC_ON);
				swd_write(req_write_drw, RESET_CORE);
				
				
				// once programming begins send progress bytes 1-124
				//NOTE 1: AP reads return result from previous read so it's necessary to read twice and discard first
				//NOTE 2: Driving the line low results in a small current drain at target
				
				
			}	
		}	
	}
}
