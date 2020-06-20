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

//error type defines 
#define error_no_target 0x00

/**
 * Initializes MCU, drivers and middleware in the project
 **/
void atmel_start_init(void);

#ifdef __cplusplus
}
#endif
#endif
