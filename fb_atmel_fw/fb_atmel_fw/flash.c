/*
 * flash.c
 *
 * Created: 6/30/2020 9:33:54 AM
 *  Author: Parker
 */ 

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
#include "flash.h"


static uint32_t num_words;


uint32_t flash_target(uint32_t data_addr, uint32_t data_len, uint32_t chip_type, uint32_t start_address)
{
	//TODO: if data length is bigger than RAM program in chunks
	
	uint32_t ret = f_ok; 
	nor_flash_qspi_init();
	uint8_t* qspi_buff = malloc(sizeof(data_len)); 
	spi_nor_flash_read(SPI_NOR_FLASH_0, qspi_buff, data_addr, data_len); //data_len
	
	// target_reset(false); //hard reset needed for non-debug flash write?
	jtag_to_swd();			
	uint32_t idcode = swd_read(req_read_idcode); 
	if(idcode == 0x00000000 || idcode == 0xFFFFFFFF)
	{
		i2c_send_error(error_no_target);
		ret = f_err; 
	}
	
	num_words = data_len/WORD_SIZE; // global word count, accurate?
	
	if(ret == f_ok)
	{
		ret = branch_target(qspi_buff, chip_type, start_address);
	}

	free(qspi_buff);
	nor_flash_qspi_deinit(); // deinit after flashing to allow nordic to navigate
	
	return ret; 	
}

//core(swd start_up) -> soc specific (flash map & MSC)  
uint32_t branch_target(uint8_t* qspi_buff, uint32_t chip_type, uint32_t start_address)
{	
	uint32_t ret; 
	switch(chip_type)
	{
		case NRF52840:
			ret = swd_m4_startup(); 
			if(ret == f_ok)
			{
				NRF52840_program(qspi_buff, start_address);
				swd_m4_reset();
			}
			break;
			
		case NRF52832:
			break;
			
		case ATSAMD51:
			break;
		
		case ATSAMD21:
			break;
		
		case STM32F0:
			break;
		
		case STM32F1:
			break;
		
	}
	
	return ret; 
				
				
	//NOTE 1: AP reads return result from previous read so it's necessary to read twice and discard first
	//NOTE 2: Driving the line low results in a small current drain at target
}

void swd_m4_reset(void)
{
	//reset core
	swd_clear_abort_reg();
	swd_write(req_write_select, AP_PORT_0);
	swd_write(req_write_tar, _AIRCR);
	swd_write(req_write_csw, CSW_AUTOINC_ON);
	swd_write(req_write_drw, RESET_CORE);
}

uint32_t swd_m4_startup(void)
{
	uint32_t ctrl_reg = swd_read(req_read_ctrl_stat); // should return 0x00
		
	swd_write(req_write_ctrl_stat, SYS_DBG_PWRUP); // pwrup debug and sys
	ctrl_reg = swd_read(req_read_ctrl_stat);
	if((ctrl_reg & PWRUP_MASK) != PWRUP_MASK) // check pwrup status
	{
		i2c_send_error(error_no_dbg_pwr);
		return f_err; 
	}
				
	swd_write(req_write_csw, CSW_AUTOINC_OFF);
	swd_write(req_write_select, AP_PORT_F);// select AP port 0
	swd_read(req_read_idr);
	uint32_t idr_value = swd_read(req_read_idr);
	if(idr_value == IDR_DEBUG_LOCKED)
	{
		i2c_send_error(error_dbg_locked);
		return f_err; 
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
	
	return f_ok; 
}

uint32_t NRF52840_program(uint8_t* qspi_buff, uint32_t start_address)
{
	//-------------------------------NRF52840 specific-------------------------------// 
	
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
	uint32_t progress_chunk = num_words/124; 
	uint8_t packet_count = 0;
				
	for(int i=0; i<num_words; i++) //will there be remainder / non-full word sent?
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
		progress_track++; // Once programming begins send progress bytes 1-124
		if(progress_track >= progress_chunk) //chunk size reached notify nordic	
		{
			i2c_send_progress(packet_count);//send progress to nordic in 124 increments
			packet_count++; 
			progress_track = 0;
		}		
		*/
	}
}