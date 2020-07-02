/*
 * flash.h
 *
 * Created: 6/30/2020 9:40:07 AM
 *  Author: Parker
 */ 


#ifndef FLASH_H_
#define FLASH_H_

#define f_err 1
#define f_ok 0

enum target
{
	NRF52840,
	NRF52832,
	ATSAMD51,
	ATSAMD21,
	STM32F0,
	STM32F1
};

uint32_t flash_target(uint32_t data_addr, uint32_t data_len, uint32_t chip_type, uint32_t start_address);
uint32_t branch_target(uint8_t* qspi_buff, uint32_t chip_type, uint32_t start_address);

uint32_t swd_m4_startup(void);
void swd_m4_reset(void);
uint32_t NRF52840_program(uint8_t* qspi_buff, uint32_t start_address);


#endif /* FLASH_H_ */