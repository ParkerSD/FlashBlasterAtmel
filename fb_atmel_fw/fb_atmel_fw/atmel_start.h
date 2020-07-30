#ifndef ATMEL_START_H_INCLUDED
#define ATMEL_START_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "driver_init.h"
#include "spi_nor_flash_main.h"

#define start_byte 0xCC 
#define target_cmd 0xBB // nordic -> atmel - followed by uint32_t data_addr and uint32_t data_len 
#define error_cmd 0xEE // followed by error type byte
#define progress_cmd 0xAA // followed by programming progress indicator byte 0-255
#define shutdwn_cmd 0xF1 

//error type defines 
#define error_no_target 0x00
#define error_no_dbg_pwr 0x01
#define error_dbg_locked 0x02

void system_off(void);
void sleep_hibernate(void);
void sleep_standby(void);
/**
 * Initializes MCU, drivers and middleware in the project
 **/
void atmel_start_init(void);

#ifdef __cplusplus
}
#endif
#endif
