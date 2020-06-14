/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_examples.h"
#include "driver_init.h"
#include "utils.h"

static uint8_t src_data[512];
static uint8_t chk_data[512];
/**
 * Example of using FLASH_0 to read and write Flash main array.
 */
void FLASH_0_example(void)
{
	uint32_t page_size;
	uint16_t i;

	/* Init source data */
	page_size = flash_get_page_size(&FLASH_0);

	for (i = 0; i < page_size; i++) {
		src_data[i] = i;
	}

	/* Write data to flash */
	flash_write(&FLASH_0, 0x3200, src_data, page_size);

	/* Read data from flash */
	flash_read(&FLASH_0, 0x3200, chk_data, page_size);
}

/**
 * Example of using QUAD_SPI_0 to get N25Q256A status value,
 * and check bit 0 which indicate embedded operation is busy or not.
 */
void QUAD_SPI_0_example(void)
{
	uint8_t              status = 0xFF;
	struct _qspi_command cmd    = {
        .inst_frame.bits.inst_en  = 1,
        .inst_frame.bits.data_en  = 1,
        .inst_frame.bits.tfr_type = QSPI_READ_ACCESS,
        .instruction              = 0x05,
        .buf_len                  = 1,
        .rx_buf                   = &status,
    };

	qspi_sync_enable(&QUAD_SPI_0);
	while (status & (1 << 0)) {
		qspi_sync_serial_run_command(&QUAD_SPI_0, &cmd);
	}
	qspi_sync_deinit(&QUAD_SPI_0);
}

static struct timer_task TIMER_0_task1, TIMER_0_task2;
/**
 * Example of using TIMER_0.
 */
static void TIMER_0_task1_cb(const struct timer_task *const timer_task)
{
}

static void TIMER_0_task2_cb(const struct timer_task *const timer_task)
{
}

void TIMER_0_example(void)
{
	TIMER_0_task1.interval = 100;
	TIMER_0_task1.cb       = TIMER_0_task1_cb;
	TIMER_0_task1.mode     = TIMER_TASK_REPEAT;
	TIMER_0_task2.interval = 200;
	TIMER_0_task2.cb       = TIMER_0_task2_cb;
	TIMER_0_task2.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_0, &TIMER_0_task1);
	timer_add_task(&TIMER_0, &TIMER_0_task2);
	timer_start(&TIMER_0);
}


void SWD_tx(uint8_t* buffer, uint16_t length) //SPI MOSI
{	
	gpio_set_pin_level(MOSI_EN, true);
	gpio_set_pin_level(MISO_EN, false);
	
	struct io_descriptor *io;
	spi_m_sync_get_io_descriptor(&SPI_0, &io);

	spi_m_sync_enable(&SPI_0);
	io_write(io, buffer, length);
}

void SWD_rx(uint8_t* buffer, uint16_t length) //SPI MISO
{
	gpio_set_pin_level(MOSI_EN, false);
	gpio_set_pin_level(MISO_EN, true);
	
	struct io_descriptor *io;
	spi_m_sync_get_io_descriptor(&SPI_0, &io);

	spi_m_sync_enable(&SPI_0);
	io_read(io, buffer, length);
}

void SPI_transfer_tx(uint8_t* buffer, uint32_t size) // uses spi_m_sync_trans driver - hpl level 
{
		/*
			struct spi_msg {
			// Pointer to the output data buffer 
			uint8_t *txbuf;
			// Pointer to the input data buffer 
			uint8_t *rxbuf;
			// Size of the message data in SPI characters 
			uint32_t size;
			};
		*/
	struct spi_msg spi_istruct;
	spi_istruct.txbuf = buffer; 
	spi_istruct.rxbuf = NULL;
	spi_istruct.size = size; 
				//transfer message without interrupt 
	gpio_set_pin_level(MOSI_EN, true);
	gpio_set_pin_level(MISO_EN, false);
	spi_m_sync_enable(&SPI_0);
	_spi_m_sync_trans(&SPI_0.dev, &spi_istruct);//(struct _spi_m_sync_dev *dev, const struct spi_msg *msg);
}

void I2C_read(uint8_t* buffer, uint16_t length)
{
	struct io_descriptor *io;
	
	i2c_s_sync_get_io_descriptor(&I2C_0, &io);
	i2c_s_sync_set_addr(&I2C_0, I2C_ADDR);
	i2c_s_sync_enable(&I2C_0);

	io_read(io, buffer, length);
}

/**
 * Example of using WDT_0.
 */
void WDT_0_example(void)
{
	uint32_t clk_rate;
	uint16_t timeout_period;

	clk_rate       = 1000;
	timeout_period = 4096;
	wdt_set_timeout_period(&WDT_0, clk_rate, timeout_period);
	wdt_enable(&WDT_0);
}
