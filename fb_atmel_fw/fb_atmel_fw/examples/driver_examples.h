/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include <stdint.h>

#ifndef DRIVER_EXAMPLES_H_INCLUDED
#define DRIVER_EXAMPLES_H_INCLUDED

#define I2C_ADDR 0x10
#define RX_LENGTH 10

#ifdef __cplusplus
extern "C" {
#endif

void EXTERNAL_IRQ_0_example(void);

void FLASH_0_example(void);

void QUAD_SPI_0_example(void);

void TIMER_0_example(void);

void I2C_Read(uint8_t* buffer, uint16_t length);

void WDT_0_example(void);

#ifdef __cplusplus
}
#endif
#endif // DRIVER_EXAMPLES_H_INCLUDED
