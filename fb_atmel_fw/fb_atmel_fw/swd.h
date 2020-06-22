/*
 * swd.h
 *
 * Created: 6/12/2020 7:10:24 AM
 *  Author: Parker
 */ 

#include <stdint.h>

#ifndef SWD_H_
#define SWD_H_

#define ACK_MASK 0b00000010 
#define PARITY_MASK 0b00010000 //NOTE parity is 1 if data contains odd # of 1's, 0 if even 

//SWD command set
#define req_read_idcode 0b10100101
#define req_read_ctrl_stat 0b10110001

void jtag_to_swd(void);
void target_reset(void);
void swd_reset(void);
uint32_t swd_read(uint8_t);


#endif /* SWD_H_ */