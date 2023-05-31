/*******************************************
*  SPI minimal library public definitions  *
********************************************/

#pragma once

#define	SPI_DDR		DDRB
#define	SPI_PORT	PORTB
#define SPI_SS		0
#define SPI_SCK		1
#define SPI_MOSI	2
#define SPI_MISO	3

void spi_init(void);
uint8_t spi_exch(uint8_t output);
