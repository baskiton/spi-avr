#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>

#include <defines.h>

#define SPI_SS      PORTB2
#define SPI_MOSI    PORTB3
#define SPI_MISO    PORTB4
#define SPI_SCK     PORTB5
#define SPI_PORT    &PORTB  // this for Atmega328 and compatible. Not recommended to use

static uint8_t spi_sreg;

/* SPI Enable */
#define spi_on() (bit_set(SPCR, SPE))

/* SPI Disable */
#define spi_off() (bit_clear(SPCR, SPE))

typedef struct spi_device_s {
    struct avr_pin_s cs;    // chip select
    struct avr_pin_s rst;   // reset
    struct avr_pin_s intr;  // interrupt
    struct avr_pin_s a0;    // a0
    void *priv_data;        // device private data pointer
} spi_dev_t;

/*!
 * @brief Set private data to SPI device
 */
static inline void spi_set_priv(spi_dev_t *spi_dev, void *data) {
    spi_dev->priv_data = data;
}

/*!
 * @brief Get pointer to private data
 * @return Pointer to data
 */
static inline void *spi_get_priv(spi_dev_t *spi_dev) {
    return spi_dev->priv_data;
}

/* Select the specific SPI device. CS - spi_dev_t->cs */
static inline void chip_select(struct avr_pin_s *cs) {
    spi_sreg = SREG;
    cli();
    bit_clear(*cs->port, cs->pin_num);
}

/* Deselect the specific SPI device. CS - spi_dev_t->cs */
static inline void chip_desel(struct avr_pin_s *cs) {
    bit_set(*cs->port, cs->pin_num);
    SREG = spi_sreg;
}

/* One SPI Clock pulse */
inline void spi_pulse(void) {
    bit_set(PORTB, SPI_SCK);
    nop();
    bit_clear(PORTB, SPI_SCK);
}

void spi_init(void);
void spi_set_speed(uint32_t freq);

void spi_write(uint8_t data);
void spi_write16(uint16_t data);
void spi_write24(uint32_t data);
void spi_write32(uint32_t data);
void spi_write_buf(const uint8_t *buf, uint16_t count);

void spi_write_no_check(uint8_t data);
void spi_write16_no_check(uint16_t data);

uint8_t spi_read_8(void);
uint16_t spi_read_16(void);
uint32_t spi_read_24(void);
uint32_t spi_read_32(void);
void spi_read_buf(uint8_t *buf, uint16_t count);

int8_t spi_device_register(spi_dev_t *spi_dev,
                           uint8_t cs_num, volatile uint8_t *cs_port,
                           uint8_t rst_num, volatile uint8_t *rst_port,
                           uint8_t intr_num, volatile uint8_t *intr_port,
                           uint8_t a0_num, volatile uint8_t *a0_port);

#endif  /* SPI_H */
