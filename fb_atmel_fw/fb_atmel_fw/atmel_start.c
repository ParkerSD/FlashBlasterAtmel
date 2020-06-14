#include <atmel_start.h>
#include "driver_init.h"
/**
 * Initializes MCU, drivers and middleware in the project
 **/
void atmel_start_init(void)
{
	system_init();
	SWD_GPIO_init();
}
