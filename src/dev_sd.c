#include "block_buf.h"
#include "module.h"
#include "device.h"
#include "spi.h"
#include <cpu.h>
#include <string.h>

#pragma portmode z180

// TODO: Handle all errors properly

#define SD_NCR_MAX 8
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

int dev_sd_init(void);
void dev_sd_exit(void);

struct module dev_sd_module = {
    dev_sd_init,
    dev_sd_exit
};

struct file_ops sd_driver = {
    .read = sd_read,
    .write = sd_write,
};

struct device_block *sd_0;

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

void sd_power_up_seq(void) {
    sd_sel(0);
    cpu_delay_ms(1);
    for (unsigned char i = 0; i < 10; ++i)
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
    while (r1 & 0x80) { // TODO timeout after 8
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

uint8_t sd_write_single_block(uint32_t addr, const uint8_t *buf, uint8_t *token) {
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

int dev_sd_init(void) {
    int result = -1;

    mutex_lock(&spi_mtx);

    sd_power_up_seq();

    if (sd_go_idle_state() > 0x01) {
        result = -1;
        goto done;
    }

    uint8_t command_version;
    uint8_t reserved_bits;
    uint8_t voltage_accepted;
    uint8_t check_pattern;
    if (sd_send_if_cond(&command_version, &reserved_bits, &voltage_accepted, &check_pattern) > 0x01) {
        result = -1;
        goto done;
    }

    uint8_t send_op_cond_result = 0xFF;
    for (unsigned char init_attempts = 0; init_attempts < 100; ++init_attempts) {
        uint8_t send_app_result = sd_send_app();
        if (!(send_app_result & 0xFE)) {
            send_op_cond_result = sd_send_op_cond();
            if (send_op_cond_result == 0x00)
                break;
        }
        cpu_delay_ms(10);
    }
    if (send_op_cond_result != 0x00) {
        result = -1;
        goto done;
    }

    uint32_t ocr;
    if (sd_read_ocr(&ocr) > 0x01) {
        result = -1;
        goto done;
    }

    result = 0;

done:
    mutex_unlock(&spi_mtx);

    device_register_driver(3, &sd_driver);

    return result;
}

void dev_sd_exit(void) {
    // TODO: Clean up
    return;
}

// TODO: Do error checking
ssize_t sd_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    if (file_ptr->special.minor != 0) {
        return -1;
    }

    ssize_t result = 0;
    uint8_t token = 0xFF;
    size_t count_done = 0;

    while (count > 0) {
        // Set bytes_to_do to have a partial or whole block
        size_t bytes_to_do = SD_BLOCK_LEN;
        unsigned long pos_in_block = pos % SD_BLOCK_LEN;
        if (pos_in_block != 0) {
            bytes_to_do = SD_BLOCK_LEN - pos_in_block;
        }
        if (count < bytes_to_do) {
            bytes_to_do = count;
        }

        char *block_buffer;
        if (bytes_to_do < SD_BLOCK_LEN) {
            #if SD_BLOCK_LEN != 512
                #error "SD hardcoded to use 512 as block size"
            #endif
            block_buffer = block_512_alloc();
        } else {
            block_buffer = buf;
        }

        sd_read_single_block(pos / SD_BLOCK_LEN * SD_BLOCK_LEN, block_buffer, &token);

        if (bytes_to_do < SD_BLOCK_LEN) {
            memcpy(buf, &block_buffer[pos_in_block], bytes_to_do);
            block_512_free(block_buffer);
        }

        pos += bytes_to_do;
        buf += bytes_to_do;
        result += bytes_to_do;
        count -= bytes_to_do;
    }

    return result;
}

// TODO: Do error checking
ssize_t sd_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos) {
    if (file_ptr->special.minor != 0) {
        return -1;
    }

    ssize_t result = 0;
    uint8_t token = 0xFF;
    size_t count_done = 0;

    while (count > 0) {
        // Set bytes_to_do to have a partial or whole block
        size_t bytes_to_do = SD_BLOCK_LEN;
        unsigned long pos_in_block = pos % SD_BLOCK_LEN;
        if (pos_in_block != 0) {
            bytes_to_do = SD_BLOCK_LEN - pos_in_block;
        }
        if (count < bytes_to_do) {
            bytes_to_do = count;
        }

        const char *block_buffer;
        if (bytes_to_do < SD_BLOCK_LEN) {
            #if SD_BLOCK_LEN != 512
                #error "SD hardcoded to use 512 as block size"
            #endif
            block_buffer = block_512_alloc();
            sd_read_single_block(pos / SD_BLOCK_LEN * SD_BLOCK_LEN, (char *) block_buffer, &token);
        } else {
            block_buffer = buf;
        }

        if (bytes_to_do < SD_BLOCK_LEN) {
            memcpy((char *) &block_buffer[pos_in_block], buf, bytes_to_do);
        }

        sd_write_single_block(pos / SD_BLOCK_LEN * SD_BLOCK_LEN, block_buffer, &token);

        if (bytes_to_do < SD_BLOCK_LEN) {
            block_512_free((char *) block_buffer);
        }

        pos += bytes_to_do;
        buf += bytes_to_do;
        result += bytes_to_do;
        count -= bytes_to_do;
    }

    return result;
}
