#include "sd.h"
#include "spi.h"
#include <cpu.h>

#include "kio.h"

#pragma portmode z180

#define NCR_MAX 8

#define SD_BLOCK_LEN 512

#define SD_START_TOKEN 0xFE

#define CMD0        0
#define CMD0_ARG    0x00000000
#define CMD0_CRC    0x94

#define CMD8        8
#define CMD8_ARG    0x0000001AA
#define CMD8_CRC    0x86

#define CMD58       58
#define CMD58_ARG   0x00000000
#define CMD58_CRC   0xFF

#define CMD55       55
#define CMD55_ARG   0x00000000
#define CMD55_CRC   0xFF

#define ACMD41      41
#define ACMD41_ARG  0x40000000
#define ACMD41_CRC  0xFF

#define CMD17                   17
#define CMD17_CRC               0x00
#define SD_MAX_READ_ATTEMPTS    1563

#define CMD24                   24
#define CMD24_CRC               0x00
#define SD_MAX_WRITE_ATTEMPTS   3907

void sd_sel(unsigned char card) {
    switch (card) {
    case 0:
        spi_cs_none();
        return;
    case 1:
        spi_cs1();
        return;
    case 2:
        spi_cs2();
        return;
    default:
        return;
    }
}

void sd_send_75_dummy_bits(void) {
    // Actually sends 80 dummy bits
    for (unsigned char i = 0; i < 8; ++i)
        spi_out_byte(0xFF);
}

void sd_command(uint8_t command, uint32_t argument, uint8_t crc) {
    spi_out_byte(command | 0x40);
    spi_out_byte(argument >> 24);
    spi_out_byte(argument >> 16);
    spi_out_byte(argument >> 8);
    spi_out_byte(argument >> 0);
    spi_out_byte(crc | 0x01);
    return;
}

uint8_t sd_response_r1(void) {
    uint8_t r1 = 0xFF;
    while (r1 & 0x80) {
        r1 = spi_in_byte();
    }
    return r1;
}

uint8_t sd_response_r1b(void) {
    uint8_t r1 = sd_response_r1();
    while (spi_in_byte() == 0)
        ;
    return r1;
}

uint8_t sd_response_r2(uint8_t *byte2) {
    uint8_t r1 = sd_response_r1();
    *byte2 = spi_in_byte();
    return r1;
}

uint8_t sd_response_r3(uint32_t *ocr) {
    uint8_t r1 = sd_response_r1();
    *ocr = 0;
    *ocr |= (uint32_t) spi_in_byte() << 24;
    *ocr |= (uint32_t) spi_in_byte() << 16;
    *ocr |= (uint32_t) spi_in_byte() << 8;
    *ocr |= (uint32_t) spi_in_byte() << 0;
    return r1;
}

uint8_t sd_response_r7(uint8_t *command_version, uint8_t *reserved_bits, uint8_t *voltage_accepted, uint8_t *check_pattern) {
    uint8_t r1 = sd_response_r1();
    *command_version = spi_in_byte();
    *reserved_bits = spi_in_byte();
    *voltage_accepted = spi_in_byte();
    *check_pattern = spi_in_byte();
    return r1;
}

uint8_t sd_go_idle_state(void) {
    // Assert CS
    spi_out_byte(0xFF);
    sd_sel(1);
    spi_out_byte(0xFF);

    // Send CMD0
    sd_command(CMD0, CMD0_ARG, CMD0_CRC);

    // Read response
    uint8_t r1 = sd_response_r1();

    // Deassert CS
    spi_out_byte(0xFF);
    sd_sel(0);
    spi_out_byte(0xFF);

    return r1;
}

uint8_t sd_send_if_cond(uint8_t *command_version, uint8_t *reserved_bits, uint8_t *voltage_accepted, uint8_t *check_pattern) {
    // Assert CS
    spi_out_byte(0xFF);
    sd_sel(1);
    spi_out_byte(0xFF);

    // Send CMD8
    sd_command(CMD8, CMD8_ARG, CMD8_CRC);

    // Read response
    uint8_t r1 = sd_response_r7(command_version, reserved_bits, voltage_accepted, check_pattern);

    // Deassert CS
    spi_out_byte(0xFF);
    sd_sel(0);
    spi_out_byte(0xFF);

    return r1;
}

uint8_t sd_read_ocr(uint32_t *ocr) {
    // Assert CS
    spi_out_byte(0xFF);
    sd_sel(1);
    spi_out_byte(0xFF);

    // Send CMD58
    sd_command(CMD58, CMD58_ARG, CMD58_CRC);

    // Read response
    uint8_t r1 = sd_response_r3(ocr);

    // Deassert CS
    spi_out_byte(0xFF);
    sd_sel(0);
    spi_out_byte(0xFF);

    return r1;
}

uint8_t sd_send_app(void) {
    // Assert CS
    spi_out_byte(0xFF);
    sd_sel(1);
    spi_out_byte(0xFF);

    // Send CMD55
    sd_command(CMD55, CMD55_ARG, CMD55_CRC);

    // Read response
    uint8_t r1 = sd_response_r1();

    // Deassert CS
    spi_out_byte(0xFF);
    sd_sel(0);
    spi_out_byte(0xFF);

    return r1;
}

uint8_t sd_send_op_cond(void) {
    // Assert CS
    spi_out_byte(0xFF);
    sd_sel(1);
    spi_out_byte(0xFF);

    // Send ACMD41
    sd_command(ACMD41, ACMD41_ARG, ACMD41_CRC);

    // Read response
    uint8_t r1 = sd_response_r1();

    // Deassert CS
    spi_out_byte(0xFF);
    sd_sel(0);
    spi_out_byte(0xFF);

    return r1;
}

uint8_t sd_read_single_block(uint32_t addr, uint8_t *buf, uint8_t *token) {
    // Assert CS
    spi_out_byte(0xFF);
    sd_sel(1);
    spi_out_byte(0xFF);

    // Send CMD17
    sd_command(CMD17, addr, CMD17_CRC);

    // Read response
    uint8_t r1 = sd_response_r1();

    if(r1 != 0xFF) {
        uint8_t read = 0xFF;

        // Wait for response token
        for(unsigned short read_attempts = 0; read_attempts < SD_MAX_READ_ATTEMPTS; ++read_attempts) {
            read = spi_in_byte();
            if(read != 0xFF)
                break;
        }

        if (read == SD_START_TOKEN) {
            // Read 512 byte block
            for(unsigned short i = 0; i < 512; i++)
                *buf++ = spi_in_byte();

            // Read 16-bit CRC
            spi_in_byte();
            spi_in_byte();
        }

        // Set token to card response
        *token = read;
    }

    // Deassert CS
    spi_out_byte(0xFF);
    sd_sel(0);
    spi_out_byte(0xFF);

    return r1;
}

uint8_t sd_write_single_block(uint32_t addr, uint8_t *buf, uint8_t *token) {
    *token = 0xFF;

    // Assert CS
    spi_out_byte(0xFF);
    sd_sel(1);
    spi_out_byte(0xFF);

    // Send CMD24
    sd_command(CMD24, addr, CMD24_CRC);

    // Read response
    uint8_t r1 = sd_response_r1();

    if(r1 == 0) {
        spi_out_byte(SD_START_TOKEN);

        uint8_t read = 0xFF;

        // Read SD_BLOCK_LEN byte block
        for(unsigned short i = 0; i < SD_BLOCK_LEN; i++)
            spi_out_byte(*buf++);

        // Wait for response token
        for(unsigned short read_attempts = 0; read_attempts < SD_MAX_WRITE_ATTEMPTS; ++read_attempts) {
            read = spi_in_byte();
            if(read != 0xFF)
                break;
        }

        if ((read & 0x1F) == 0x05) {
            *token = 0x05;

            // Wait for response token
            for(unsigned short read_attempts = 0; read_attempts < SD_MAX_WRITE_ATTEMPTS; ++read_attempts) {
                read = spi_in_byte();
                if(read != 0x00)
                    break;
            }

            if (read == 0x00)
                *token = 0x00;
        }
    }

    // Deassert CS
    spi_out_byte(0xFF);
    sd_sel(0);
    spi_out_byte(0xFF);

    return r1;
}

uint8_t sd_block_buffer[512];

void sd_init(void) {
    mutex_lock(&spi_mtx);

    kio_puts("SD Init\n");

    spi_cs_none();

    sd_send_75_dummy_bits();

    kio_put_uc(sd_go_idle_state());
    kio_putc('\n');

    uint8_t command_version;
    uint8_t reserved_bits;
    uint8_t voltage_accepted;
    uint8_t check_pattern;
    kio_put_uc(sd_send_if_cond(&command_version, &reserved_bits, &voltage_accepted, &check_pattern));
    kio_putc(' ');
    kio_put_uc(command_version);
    kio_putc(' ');
    kio_put_uc(reserved_bits);
    kio_putc(' ');
    kio_put_uc(voltage_accepted);
    kio_putc(' ');
    kio_put_uc(check_pattern);
    kio_putc('\n');

    unsigned short i;
    for (i = 0; i < 100; ++i) {
        uint8_t send_app_result = sd_send_app();
        kio_put_uc(send_app_result);
        uint8_t send_op_cond_result;
        if (!(send_app_result & 0xFE)) {
            kio_putc(' ');
            send_op_cond_result = sd_send_op_cond();
            kio_put_uc(send_op_cond_result);
            kio_putc('\n');
            if (!send_op_cond_result)
                break;
        } else {
            kio_putc('\n');
        }
        cpu_delay_ms(10);
    }

    uint32_t ocr;
    kio_put_uc(sd_read_ocr(&ocr));
    kio_putc(' ');
    kio_put_ul(ocr);
    kio_putc('\n');

    uint8_t token;
    kio_put_uc(sd_read_single_block(0, sd_block_buffer, &token));
    kio_putc(' ');
    kio_put_uc(token);
    kio_putc('\n');

    for (i = 0; i < 512 / 16; ++i) {
        for (unsigned short j = 0; j < 16; ++j) {
            kio_put_uc(sd_block_buffer[i * 16 + j]);
            kio_putc(' ');
        }
        kio_putc('\n');
    }

    kio_put_uc(sd_write_single_block(0, sd_block_buffer, &token));
    kio_putc(' ');
    kio_put_uc(token);
    kio_putc('\n');

    mutex_unlock(&spi_mtx);
}
