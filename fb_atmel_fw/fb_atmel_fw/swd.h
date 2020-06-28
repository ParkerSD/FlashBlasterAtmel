/*
 * swd.h
 *
 * Created: 6/12/2020 7:10:24 AM
 *  Author: Parker
 */ 

#include <stdint.h>
#include <stdbool.h>

#ifndef SWD_H_
#define SWD_H_

#define WORD_SIZE 4 
#define ACK_MASK 0b00000010 
#define PARITY_MASK 0b00010000 //NOTE parity is 1 if data contains odd # of 1's, 0 if even 
#define PWRUP_MASK 0xF0000000

//SWD request command set - Debug Port
#define req_read_idcode 0b10100101
#define req_write_abort 0b10000001
#define req_write_select 0b10110001
#define req_read_rdbuff 0b10111101

//SWD request command set - AHB-AP
#define req_read_idr 0b10011111 //IDR = ID register, located in port F, M4 = 0x24770011 , M0+ = 0x04770031
#define req_write_csw 0b10100011
#define req_read_ctrl_stat 0b10001101//MSB 0b10110001
#define req_write_ctrl_stat 0b10101001
#define req_write_tar 0b10001011
#define req_write_drw 0b10111011
#define req_read_drw 0b10011111

//Core Debug Register Addr - Accessed with TAR and DRW
#define _DFSR 0xE000ED30 // Debug Fault Status Register
#define _DHCSR 0xE000EDF0 // Debug Halting Control and Status Register 
#define _DCRSR 0xE000EDF4 // Debug Core Register Selector Register 
#define _DCRDR 0xE000EDF8 // Debug Core Register Data Register 
#define _DEMCR 0xE000EDFC // Debug Exception and Monitor Control Register 

//System Control Registers 
#define _AIRCR 0xE000ED0C // Application Interrupt and Reset Control Register 


//SWD data command set - Cortex-M4
#define SYS_DBG_PWRUP 0x50000000
#define AP_PORT_0 0x00000000
#define AP_PORT_F 0x000000F0//(select AHB-AP, bank 0xF)
#define HALT_CORE 0xA05F0003 // halts core when sent to DHCSR 
#define EN_HALT_ON_RST 0x00000001 // enables halt on reset when sent to DEMCR
#define RESET_CORE 0x05FA0004 // resets core when sent to AIRCR 

#define CSW_AUTOINC_ON 0x23000012 //32bit word
#define CSW_AUTOINC_OFF 0x23000002 //32bit word
#define IDR_DEBUG_LOCKED 0x16E60001


//-----SOC Specific Registers-----// 

//NRF52840 NVMC Register Addresses
#define NRF52_NVMC_READY 0x4001E400//Ready flag; 0 = busy, 1 = ready 
#define NRF52_NVMC_READY_NEXT 0x4001E408 //Ready flag; 0 = busy, 1 = ready 
#define NRF52_NVMC_CONFIG 0x4001E504 // 0=read only access, 1=write enabled, 2=erase enabled
#define NRF52_NVMC_ERASE_PAGE 0x4001E508
#define NRF52_NVMC_ERASE_ALL 0x4001E50C //0=no operation, 1=erase all
#define NRF52_NVMC_ERASE_UICR 0x4001E514 //erase user info config registers
#define NRF52_NVMC_ERASE_PAGE_PARTIAL 0x4001E518
#define NRF52_NVMC_ERASE_PAGE_PARTIAL_CFG 0x4001E51C
#define NRF52_NVMC_ICACHE_CFG 0x4001E540 //instruction cache configuration register.
#define NRF52_NVMC_IHIT 0x4001E548 //instruction cache hit counter.

#define NRF52_FLASH_START 0x00000000

//NRF52840 NVMC Commands
#define WRITE_ENABLE 1 
#define ERASE_ENABLE 2 
#define ERASE_ALL 1


bool swd_calc_parity(uint32_t payload);
void swd_clear_abort_reg(void);
void swd_idcode_throwaway(void);
void jtag_to_swd(void);
void target_reset(bool hold);
void swd_reset(void);
uint32_t swd_read(uint8_t);
void swd_write(uint8_t cmd, uint32_t payload);

void i2c_send_error(uint8_t);
void i2c_send_progress(uint8_t);


#endif /* SWD_H_ */