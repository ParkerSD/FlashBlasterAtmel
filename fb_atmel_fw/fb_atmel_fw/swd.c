/*
 * swd.c
 *
 * Created: 6/14/2020 7:14:27 AM
 *  Author: Parker
 */ 
#include "swd.h"
#include "driver_examples.h"

void swd_reset(void)
{
	uint8_t rst_cmd[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00}; // min of 50 bits clocked with swdio high
		
	for(int i = 0; i < 3; i++)
	{
		SWD_tx(rst_cmd, 8); // reset device,
	}
}

uint32_t swd_read(uint8_t cmd)
{
	uint32_t Wdata;	
	uint8_t swd_rx_buf[5];
	
	SWD_tx(&cmd, 1);
	SWD_rx(swd_rx_buf, 5);
					
	if(swd_rx_buf[0] & ACK_MASK) // check ACK
	{				
		swd_rx_buf[4] = (swd_rx_buf[4]  << 4 ) | (swd_rx_buf[3] >> 4); //first byte
		swd_rx_buf[3] = (swd_rx_buf[3]  << 4 ) | (swd_rx_buf[2] >> 4); //second
		swd_rx_buf[2] = (swd_rx_buf[2]  << 4 ) | (swd_rx_buf[1] >> 4); //third
		swd_rx_buf[1] = (swd_rx_buf[1]  << 4 ) | (swd_rx_buf[0] >> 4); //fourth
					
		Wdata = swd_rx_buf[4] << 24 | swd_rx_buf[3] << 16 | swd_rx_buf[2] << 8 | swd_rx_buf[1];
						
		//if(swd_rx_buf[4] & PARITY_MASK) // TODO: check parity, confirm odd number of 1's
		//else confirm even 1's				
	}
	//else ACK WAIT or FAULT was received
	
	return Wdata; 
}
