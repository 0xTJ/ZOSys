#ifndef INCLUDE_SPI_H
#define INCLUDE_SPI_H

#include "io_system.h"
#include <stdint.h>

#define spi_mtx io_system_mtx

void spi_init(void);
uint8_t spi_in_byte(void);
void spi_out_byte(uint8_t send_byte);
void spi_cs_none(void);
void spi_cs1(void);
void spi_cs2(void);

#endif
