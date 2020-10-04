#ifndef SPI_H
#define SPI_H

#include <avr/io.h>

#include <stdint.h>

#include <defines.h>

#define SPI_SS      PORTB2
#define SPI_MOSI    PORTB3
#define SPI_MISO    PORTB4
#define SPI_SCK     PORTB5

/* SPI Enable */
#define spi_on() (bit_set(SPCR, SPE))

/* SPI Disable */
#define spi_off() (bit_clear(SPCR, SPE))

typedef struct spi_device_s {
    struct avr_pin_s cs;    // chip select
    struct avr_pin_s rst;   // reset
    struct avr_pin_s intr;  // interrupt
    struct avr_pin_s a0;    // a0
} spi_dev_t;

/* One SPI Clock pulse */
inline void spi_pulse(void) {
    bit_set(PORTB, SPI_SCK);
    nop();
    bit_clear(PORTB, SPI_SCK);
}

extern void spi_init(void);
extern void spi_set_speed(uint32_t freq);

extern void spi_write(uint8_t data);
extern void spi_write16(uint16_t data);
extern void spi_write24(uint32_t data);
extern void spi_write32(uint32_t data);
extern void spi_write_buf(const uint8_t *buf, uint16_t count);

extern void spi_write_no_check(uint8_t data);
extern void spi_write16_no_check(uint16_t data);

extern uint8_t spi_read_8(void);
extern uint16_t spi_read_16(void);
extern uint32_t spi_read_24(void);
extern uint32_t spi_read_32(void);
extern void spi_read_buf(uint8_t *buf, uint16_t count);

#endif  /* SPI_H */
