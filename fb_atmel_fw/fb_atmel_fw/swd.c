/*
 * swd.c
 *
 * Created: 6/14/2020 7:14:27 AM
 *  Author: Parker
 */ 
#include "swd.h"
#include "driver_init.h"
#include "atmel_start.h"
#include "driver_examples.h"
#include "atmel_start_pins.h"
#include <hal_delay.h>
#include <stdbool.h>

void swd_clear_abort_reg(void)
{
	uint8_t clear_err_buf[8] ={0x00, 0x81, 0xd3, 0x03, 0x00, 0x00, 0x00, 0x00}; // includes request byte, ignores ack response for byte alignment
	SWD_tx(clear_err_buf, 8);
}

void jtag_to_swd(void) //JTAG-to-SWD switch with 16bit value, MSB = 0x79E7, LSB = 0xE79E, then min 5 clock cycles with swdio high 
{
	uint8_t swd_seq_jtag_to_swd[] = {
		/* At least 50 TCK/SWCLK cycles with TMS/SWDIO high */
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		/* Switching sequence from JTAG to SWD */
		0x9e, 0xe7,
		/* At least 50 TCK/SWCLK cycles with TMS/SWDIO high */
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		/* At least 2 idle (low) cycles */
		0x00,
	};
	
	SWD_tx(swd_seq_jtag_to_swd, 17);
}

void swd_idcode_throwaway(void)
{
	/// SWD Sequence to Read Register 0 (IDCODE), pre-padded with 2 null bits bits to fill up 6 bytes. Byte-aligned, will not cause overrun error.
	/// A transaction must be followed by another transaction or at least 8 idle cycles to ensure that data is clocked through the AP.
	/// After clocking out the data parity bit, continue to clock the SW-DP serial interface until it has clocked out at least 8 more clock rising edges, before stopping the clock.
	uint8_t swd_read_idcode_prepadded[]   = { 0x00, 0x94, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 };  //  With null byte (8 cycles idle) before and after
	SWD_tx(swd_read_idcode_prepadded, 8);

}

void target_reset(bool hold)
{
	gpio_set_pin_level(RST, true);
	delay_ms(5); // hold in reset
	if(!hold)
	{
		gpio_set_pin_level(RST, false); //release from reset
		delay_ms(1);	
	}
	//TODO: sense input 
}

void swd_reset(void)
{
	uint8_t rst_cmd[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00}; // min of 50 bits clocked with swdio high
	for(int i = 0; i < 3; i++)
	{
		SWD_tx(rst_cmd, 8); // reset device,
	}
}

bool swd_calc_parity(uint32_t n)
{
	
	uint8_t count = 0; 
    while (n) 
	{ 
        count += n & 1; 
        n >>= 1; 
    } 
	if(count % 2) // if non-zero n has odd number of 1's 
	{
		return true; //odd 
	}
	else
	{
		return false; //even 
	}
}

void swd_write(uint8_t cmd, uint32_t payload)
{
	//uint8_t clear_err_buf[8] ={0x00, 0x81, 0xd3, 0x03, 0x00, 0x00, 0x00, 0x00}; // for test
	//SWD_tx(clear_err_buf, 8);
	
	uint8_t payload_lsbs = (payload & 0x00000007); //take three lsbs of payload 
	uint8_t ack_plus_lsbs = (payload_lsbs<<5) + 0x13; //value needed to fake ack read 
	
	bool parity = swd_calc_parity(payload); 
	payload >>= 3; //shift payload over by 3 bytes to compensate for lsbs already used
	uint8_t data_buf[4];                              
    data_buf[0] = (payload >> 24) & 0xFF; //bit shift 32bit address into 8bit array 
    data_buf[1] = (payload >> 16) & 0xFF;
    data_buf[2] = (payload >> 8) & 0xFF;
    data_buf[3] = (payload) & 0xFF; // 3 msbs of this byte include parity bit and trailing bits
	
	if(parity)	//NOTE: sixth bit of data_buf[0] holds parity
	{
		data_buf[0] |= 0b00100000; //odd number of 1's, set parity to 1
	}
	else
	{
		data_buf[0] &= 0b11011111; //even number of 1's, set parity to 0
	}
	
	uint8_t swd_write_buf[8] ={0x00, cmd, ack_plus_lsbs, data_buf[3], data_buf[2], data_buf[1], data_buf[0], 0x00}; //3 lsbs of payload become msbs of byte after cmd
	SWD_tx(swd_write_buf, 8);
}
/*
NOTE: if sync lost (ex: no stop bit) target leaves line un-driven and waits for host to retry 
(after some clk cycles w/ line low), or signals a reset by not driving line 
*/
uint32_t swd_read(uint8_t cmd)
{
	uint32_t Wdata = 0;	
	uint8_t swd_rx_buf[5];
	
	//hri_sercomspi_write_CTRLA_CPHA_bit(SERCOM0, false); //no effect/not needed
		
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
	
	//NOTE: this sequence must be repeated after every read to correct for bit alignment errors
	swd_clear_abort_reg(); //error flagged due to misaligned bit during previous read
	jtag_to_swd();
	swd_idcode_throwaway(); //fakes a byte-aligned read of idcode, as is required by the arm spec after reset
	
	return Wdata; 
}


void i2c_send_error(uint8_t err_id) //TODO: use gpio to signal i2c master to read
{
	uint8_t error_packet[3] = {start_byte, error_cmd, err_id};
	I2C_write(error_packet, 3); //use gpio to signal master read (new boards)
}

void i2c_send_progress(uint8_t progress)
{
	uint8_t progress_packet[3] = {start_byte, progress_cmd, progress};
	I2C_write(progress_packet, 3);
}





/*
	// PIN 4 becomes SWDIO and is re-initted at every direction change
	
	//PA04 is MOSI
	gpio_set_pin_level(PA04, false);  
	gpio_set_pin_direction(PA04, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(PA04, PINMUX_PA04D_SERCOM0_PAD0);
	gpio_set_pin_direction(PA07, GPIO_DIRECTION_IN); //MISO
	gpio_set_pin_pull_mode(PA07, GPIO_PULL_OFF);
	gpio_set_pin_function(PA07, PINMUX_PA07D_SERCOM0_PAD3);
	
	//PA04 is MISO
	gpio_set_pin_direction(PA04, GPIO_DIRECTION_IN); //MISO
	gpio_set_pin_pull_mode(PA04, GPIO_PULL_OFF);
	gpio_set_pin_function(PA04, PINMUX_PA04D_SERCOM0_PAD0);	
	gpio_set_pin_level(PA07, false);
	gpio_set_pin_direction(PA07, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(PA07, PINMUX_PA07D_SERCOM0_PAD3);
*/