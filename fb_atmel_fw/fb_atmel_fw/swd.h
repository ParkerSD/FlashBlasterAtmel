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

#define ACK_MASK 0b00000010 
#define PARITY_MASK 0b00010000 //NOTE parity is 1 if data contains odd # of 1's, 0 if even 
#define PWRUP_MASK 0xF0000000

//SWD request command set - Debug Port
#define req_read_idcode 0b10100101
#define req_write_abort 0b10000001
#define req_write_select 0b10110001
#define req_read_ctrl_stat 0b10001101//MSB 0b10110001
#define req_write_ctrl_stat 0b10101001

//SWD request command set - AHB-AP
#define req_read_idr 0b10011111 //port f, Cortex-M4 = 0x24770011 , M0+ = 0x04770031
#define req_write_csw 0b10100011

//SWD data command set
#define SYS_DBG_PWRUP 0x50000000
#define AP_PORT_0 0x00000000
#define AP_PORT_F 0x000000F0//(select AHB-AP, bank 0xF)

#define CSW_AUTOINC_ON 0x23000012 //32bit word
#define CSW_AUTOINC_OFF 0x23000002 //32bit word
#define IDR_DEBUG_LOCKED 0x16E60001

bool swd_calc_parity(uint32_t payload);
void swd_clear_abort_reg(void);
void swd_idcode_throwaway(void);
void jtag_to_swd(void);
void target_reset(bool hold);
void swd_reset(void);
uint32_t swd_read(uint8_t);
void swd_write(uint8_t cmd, uint32_t payload);

void i2c_send_error(uint8_t);


#endif /* SWD_H_ */