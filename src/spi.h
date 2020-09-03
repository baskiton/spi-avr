#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <stdint.h>
#include <stddef.h>

#include <defines.h>

#define SPI_SS      PORTB2
#define SPI_MOSI    PORTB3
#define SPI_MISO    PORTB4
#define SPI_SCK     PORTB5

/* SPI Enable */
#define spi_on() (bit_set(SPCR, SPE))

/* SPI Disable */
#define spi_off() (bit_clear(SPCR, SPE))


extern void spi_init(void);

extern void spi_write(uint8_t data);
extern void spi_write16(uint16_t data);
extern void spi_write24(uint32_t data);
extern void spi_write32(uint32_t data);
extern void spi_write_buf(void *buf, size_t count);

extern void spi_write_precheck(uint8_t data);
extern void spi_write16_precheck(uint16_t data);

extern void spi_write_no_check(uint8_t data);
extern void spi_write16_no_check(uint16_t data);

extern uint8_t spi_read_8(void);
extern uint16_t spi_read_16(void);
extern uint32_t spi_read_24(void);
extern uint32_t spi_read_32(void);
extern void spi_read_buf(void *buf, size_t count);

#endif  /* SPI_H */
