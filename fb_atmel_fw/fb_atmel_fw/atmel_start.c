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

void sleep_standby(void) //~1.5mA draw
{
	PM->SLEEPCFG.bit.SLEEPMODE = PM_SLEEPCFG_SLEEPMODE_STANDBY;
	while(PM->SLEEPCFG.bit.SLEEPMODE != PM_SLEEPCFG_SLEEPMODE_STANDBY);
	_go_to_sleep();
}

void sleep_hibernate(void)
{
	PM->SLEEPCFG.bit.SLEEPMODE = PM_SLEEPCFG_SLEEPMODE_HIBERNATE;
	while(PM->SLEEPCFG.bit.SLEEPMODE != PM_SLEEPCFG_SLEEPMODE_HIBERNATE);
	_go_to_sleep();
}

void system_off(void) // < 1mA draw
{
	PM->SLEEPCFG.bit.SLEEPMODE = PM_SLEEPCFG_SLEEPMODE_OFF;
	while(PM->SLEEPCFG.bit.SLEEPMODE != PM_SLEEPCFG_SLEEPMODE_OFF);
	_go_to_sleep();
}