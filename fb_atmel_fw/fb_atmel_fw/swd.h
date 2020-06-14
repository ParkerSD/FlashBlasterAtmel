/*
 * swd.h
 *
 * Created: 6/12/2020 7:10:24 AM
 *  Author: Parker
 */ 


#ifndef SWD_H_
#define SWD_H_

#define ACK_MASK 0b00000010 
#define PARITY_MASK 0b00010000 //NOTE parity is 1 if data contains odd # of 1's, 0 if even 

uint8_t req_dbg_read_idcode = 0b10100101;



#endif /* SWD_H_ */