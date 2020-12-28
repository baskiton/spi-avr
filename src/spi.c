#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>
#include <stddef.h>

#include <defines.h>

#include "spi.h"


/*!
 * @brief Initialize Hardware SPI sequence
 */
void spi_init(void) {
    uint8_t sreg = SREG;
    cli();  // Protect from a scheduler

    /* Set ~SS as output and HIGH (deselect) */
    set_output(*(SPI_PORT - 1), SPI_SS);
    bit_set(*SPI_PORT, SPI_SS);
    
    /* Warning: if the SS pin ever becomes a LOW INPUT then SPI
    automatically switches to Slave, so the data direction of
    the SS pin MUST be kept as OUTPUT.
    */
    bit_set(SPCR, MSTR);     // set SPI master
    bit_set(SPCR, SPE);      // SPI enable

    /* Set  MOSI and SCK output; MISO automaticly input
    By doing this AFTER enabling SPI, we avoid accidentally
    clocking in a single bit since the lines go directly
    from "input" to SPI control.
    */
    set_output(DDRB, SPI_MOSI);
    set_output(DDRB, SPI_SCK);

    /* set SPI max speed */
    bit_clear(SPCR, SPR1);
    bit_clear(SPCR, SPR0);
    bit_set(SPSR, SPI2X);

    /* clearing SPIF by reading it */
    bit_is_set(SPSR, SPIF);

    SREG = sreg;
}

/*!
 * @brief Set SPI speed
 * @param freq Target SPI frequency.
 */
void spi_set_speed(uint32_t freq) {
    uint32_t tmp = (F_CPU / 2);
    uint8_t c_div = 0;
    while ((c_div < 6) && (tmp > freq)) {
        tmp /= 2;
        c_div++;
    }
    if (c_div == 6)
        c_div = 7;
    /*
    c_div is clock divider:
    0 - \c F_CPU / 2;
    1 - \c F_CPU / 4;
    2 - \c F_CPU / 8;
    3 - \c F_CPU / 16;
    4 - \c F_CPU / 32;
    5 - \c F_CPU / 64;
    6 - \c F_CPU / 128;
    */
    bit_write(SPSR, SPI2X, ~(c_div & 0b001));
    bit_write(SPCR, SPR0, c_div & 0b010);
    bit_write(SPCR, SPR1, c_div & 0b100);
}

/*!
 * @brief Sending data from SPI
 * @param data Send 8-bit data
 */
inline void spi_write(uint8_t data) {
    SPDR = data;
    /* The following NOP introduces a small delay that can prevent the wait
     * loop form iterating when running at the maximum speed. This gives
     * about 10% more speed, even if it seems counter-intuitive. At lower
     * speeds it is unnoticed.
     */
    nop();
    while (!(SPSR & _BV(SPIF)));
}

/*!
 * @brief Sending data from SPI
 * @param data Send 16-bit data
 */
inline void spi_write16(uint16_t data) {
    union {
        uint16_t val;
        struct {
            uint8_t lsb;
            uint8_t msb;
        };
    } in = {data};

    SPDR = in.msb;
    nop();
    while (!(SPSR & _BV(SPIF)));

    SPDR = in.lsb;
    nop();
    while (!(SPSR & _BV(SPIF)));
}

/*!
 * @brief Sending data from SPI
 * @param data Send 24-bit data
 */
inline void spi_write24(uint32_t data) {
    union {
        uint32_t val;
        struct {
            uint8_t lsb;
            uint8_t lmsb;
            uint8_t mlsb;
            uint8_t msb;
        };
    } in = {data};
    
    SPDR = in.mlsb;
    nop();
    while (!(SPSR & _BV(SPIF)));
    
    SPDR = in.lmsb;
    nop();
    while (!(SPSR & _BV(SPIF)));

    SPDR = in.lsb;
    nop();
    while (!(SPSR & _BV(SPIF)));
}

/*!
 * @brief Sending data from SPI
 * @param data Send 32-bit data
 */
inline void spi_write32(uint32_t data) {
    union {
        uint32_t val;
        struct {
            uint8_t lsb;
            uint8_t lmsb;
            uint8_t mlsb;
            uint8_t msb;
        };
    } in = {data};
    
    SPDR = in.msb;
    nop();
    while (!(SPSR & _BV(SPIF)));
    
    SPDR = in.mlsb;
    nop();
    while (!(SPSR & _BV(SPIF)));
    
    SPDR = in.lmsb;
    nop();
    while (!(SPSR & _BV(SPIF)));

    SPDR = in.lsb;
    nop();
    while (!(SPSR & _BV(SPIF)));
}

/*!
 * @brief Sending data sequence from SPI
 * @param buf Data sequence to send
 * @param count Number of bytes in \p buf to send
 */
inline void spi_write_buf(const uint8_t *buf, uint16_t count) {
    if (count == 0) return;

    const uint8_t *ptr = buf;
    for (int i = 0; i < count; i++) {
        SPDR = *ptr++;
        nop();
        while (!(SPSR & _BV(SPIF)));
    }
}

/*!
 * @brief Sending data from SPI without check for available
 * @param data Send 8-bit data
 */
inline void spi_write_no_check(uint8_t data) {
    SPDR = data;
}

/*!
 * @brief Sending data from SPI without check for available
 * @param data Send 16-bit data
 */
inline void spi_write16_no_check(uint16_t data) {
    union {
        uint16_t val;
        struct {
            uint8_t lsb;
            uint8_t msb;
        };
    } in = {data};
    SPDR = in.msb;
    while (!(SPSR & _BV(SPIF)));
    SPDR = in.lsb;
}

/*!
 * @brief Read 8-bit data from SPI
 * @return 8-bit data
 */
inline uint8_t spi_read_8(void) {
    uint8_t result;
    SPDR = 0xFF;
    nop();
    while (!(SPSR & _BV(SPIF)));
    result = SPDR;

    return result;
}

/*!
 * @brief Read 16-bit data from SPI
 * @return 16-bit data
 */
inline uint16_t spi_read_16(void) {
    union {
        uint32_t val;
        struct {
            uint8_t lsb;
            uint8_t msb;
        };
    } result;

    SPDR = 0xFF;
    nop();
    while (!(SPSR & _BV(SPIF)));
    result.msb = SPDR;

    SPDR = 0xFF;
    nop();
    while (!(SPSR & _BV(SPIF)));
    result.lsb = SPDR;

    return result.val;
}

/*!
 * @brief Read 24-bit data from SPI
 * @return 24-bit data
 */
inline uint32_t spi_read_24(void) {
    union {
        uint32_t val;
        struct {
            uint8_t lsb;
            uint8_t lmsb;
            uint8_t mlsb;
            uint8_t msb;
        };
    } result;

    result.msb = 0;

    SPDR = 0xFF;
    nop();
    while (!(SPSR & _BV(SPIF)));
    result.mlsb = SPDR;

    SPDR = 0xFF;
    nop();
    while (!(SPSR & _BV(SPIF)));
    result.lmsb = SPDR;

    SPDR = 0xFF;
    nop();
    while (!(SPSR & _BV(SPIF)));
    result.lsb = SPDR;

    return result.val;
}

/*!
 * @brief Read 32-bit data from SPI
 * @return 32-bit data
 */
inline uint32_t spi_read_32(void) {
    union {
        uint32_t val;
        struct {
            uint8_t lsb;
            uint8_t lmsb;
            uint8_t mlsb;
            uint8_t msb;
        };
    } result;

    SPDR = 0xFF;
    nop();
    while (!(SPSR & _BV(SPIF)));
    result.msb = SPDR;

    SPDR = 0xFF;
    nop();
    while (!(SPSR & _BV(SPIF)));
    result.mlsb = SPDR;

    SPDR = 0xFF;
    nop();
    while (!(SPSR & _BV(SPIF)));
    result.lmsb = SPDR;

    SPDR = 0xFF;
    nop();
    while (!(SPSR & _BV(SPIF)));
    result.lsb = SPDR;

    return result.val;
}

/*!
 * @brief Read data from SPI
 * @param buf Data buffer to write receiving data
 * @param count Number of bytes to read
 * @return Receiving data return in to the \p buf
 */
inline void spi_read_buf(uint8_t *buf, uint16_t count) {
    if (count == 0) return;

    uint8_t *ptr = buf;
    for (uint16_t i = 0; i < count; i++) {
        SPDR = 0xFF;
        nop();
        while (!(SPSR & _BV(SPIF)));
        *ptr++ = SPDR;
    }
}

/*!
 * @brief Initialize a new SPI device
 * @param cs_num Chip Select (SS) pin number
 * @param cs_port Chip Select port pointer. If NULL set by default
 * @param rst_num Reset (RST) pin number
 * @param rst_port Reset port pointer
 * @param intr_num Interrupt input signal pin number
 * @param intr_port Interrupt input signal port pointer
 * @param a0_num Advanced pin number
 * @param a0_port Advanced port pointer
 * @return 0 if success; errno if error
 */
int8_t spi_device_register(spi_dev_t *spi_dev,
                           uint8_t cs_num, volatile uint8_t *cs_port,
                           uint8_t rst_num, volatile uint8_t *rst_port,
                           uint8_t intr_num, volatile uint8_t *intr_port,
                           uint8_t a0_num, volatile uint8_t *a0_port) {
    if (!cs_port) {
        cs_port = SPI_PORT;
        cs_num = SPI_SS;
    }
    spi_dev->cs.pin_num = cs_num;
    spi_dev->cs.port = cs_port;
    set_output(*(cs_port - 1), cs_num);
    chip_desel(&spi_dev->cs);

    spi_dev->rst.pin_num = rst_num;
    spi_dev->rst.port = rst_port;
    if (rst_port)
        set_output(*(rst_port - 1), rst_num);

    spi_dev->intr.pin_num = intr_num;
    spi_dev->intr.port = intr_port;
    if (intr_port)
        set_input(*(intr_port - 1), intr_num);

    spi_dev->a0.pin_num = a0_num;
    spi_dev->a0.port = a0_port;
    if (a0_port)
        set_output(*(a0_port - 1), a0_num);

    return 0;
}
