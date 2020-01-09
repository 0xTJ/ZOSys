#include "ds1302.h"
#include <arch/scz180.h>
#include <cpu.h>

#define DOUT 0x80
#define SCLK 0x40
#define NWE 0x20
#define CE 0x10
#define PASSIVE 0x0F

const uint16_t bit_delay_ms = 1;

int ds1302_in_byte(void) {
    // Assumes SCLK is already high
    uint8_t receive_byte = 0;

    for (unsigned char i = 0; i < 8; ++i) {
        io_system = DOUT | NWE | CE | PASSIVE;
        cpu_delay_ms(bit_delay_ms);
        receive_byte >>= 1;
        receive_byte |= io_system & 0x01 ? 0x80 : 0x00;
        if (i < 7)
            io_system = DOUT | SCLK | NWE | CE | PASSIVE;
        cpu_delay_ms(bit_delay_ms);
    }

    // Returns with SCLK low
    return receive_byte;
}

int ds1302_out_byte(uint8_t send_byte) {
    // Assumes SCLK is already low

    for (unsigned char i = 0; i < 8; ++i) {
        io_system = (send_byte & 0x01 ? DOUT : 0) | CE | PASSIVE;
        cpu_delay_ms(bit_delay_ms);
        io_system = (send_byte & 0x01 ? DOUT : 0) | SCLK | CE | PASSIVE;
        cpu_delay_ms(bit_delay_ms);
        send_byte >>= 1;
    }

    // Returns with SCLK high
    return send_byte;
}

int ds1302_read_byte(uint8_t address) {
    if (ds1302_out_byte(address) < 0) {
        io_system = NWE | PASSIVE;
        return -1;
    }
    int result = ds1302_in_byte();
    io_system = DOUT | NWE | PASSIVE;
    return result;
}

int ds1302_write_byte(uint8_t address, uint8_t data) {
    if (ds1302_out_byte(address) < 0) {
        io_system = DOUT | NWE | PASSIVE;
        return -1;
    }
    io_system = DOUT | NWE | CE | PASSIVE;
    int result = ds1302_out_byte(data);
    io_system = DOUT | NWE | PASSIVE;
    return result;
}

int ds1302_time_get(struct tm *time) {
    int result;
    result = ds1302_read_byte(0x81);
    if (result < 0)
        return -1;
    time->tm_sec = ((result >> 4) & 0x07) * 10 + (result & 0x0F);
    result = ds1302_read_byte(0x83);
    if (result < 0)
        return -1;
    time->tm_min = ((result >> 4) & 0x07) * 10 + (result & 0x0F);
    result = ds1302_read_byte(0x85);
    if (result < 0)
        return -1;
    time->tm_hour = (result & 0x0F);
    time->tm_hour += (result & 0x10) ? 10 : 0;
    time->tm_hour += (result & 0x20) ? (result & 0x80 ? 12 : 20) : 0;
    result = ds1302_read_byte(0x87);
    if (result < 0)
        return -1;
    time->tm_mday = ((result >> 4) & 0x03) * 10 + (result & 0x0F);
    result = ds1302_read_byte(0x89);
    if (result < 0)
        return -1;
    time->tm_mon = ((result >> 4) & 0x01) * 10 + (result & 0x0F);
    result = ds1302_read_byte(0x8B);
    if (result < 0)
        return -1;
    time->tm_wday = (result & 0x07) - 1;
    result = ds1302_read_byte(0x8D);
    if (result < 0)
        return -1;
    time->tm_year = 100 + ((result >> 4) & 0x0F) * 10 + (result & 0x0F);
    return 0;
}
