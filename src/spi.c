#include "spi.h"
#include <cpu.h>

#pragma portmode z180

#define CS2 0x08
#define CS1 0x04
#define PASSIVE (IO_SYSTEM_PASSIVE & 0xF3)

inline uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void spi_init(void) {
    mutex_lock(&spi_mtx);
    CNTR = 0x06;
    mutex_unlock(&spi_mtx);
}

uint8_t spi_in_byte(void) {
    while (CNTR & __IO_CNTR_RE)
        ;
    CNTR |= __IO_CNTR_RE;
    while (CNTR & __IO_CNTR_RE)
        ;
    uint8_t receive_byte = reverse(TRDR);
    return receive_byte;
}

void spi_out_byte(uint8_t send_byte) {
    while (CNTR & __IO_CNTR_TE)
        ;
    TRDR = reverse(send_byte);
    CNTR |= __IO_CNTR_TE;
    while (CNTR & __IO_CNTR_TE)
        ;
}

void spi_cs_none(void) {
    io_system = CS2 | CS1 | PASSIVE;
}

void spi_cs1(void) {
    io_system = CS2 | PASSIVE;
}

void spi_cs2(void) {
    io_system = CS1 | PASSIVE;
}
