#include <atmel_start.h>
#include "driver_examples.h"


int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	uint8_t buffer[RX_LENGTH]; 
	
	while (1) {
		I2C_Read(buffer, RX_LENGTH);
	}
}
