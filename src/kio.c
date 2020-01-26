#include "kio.h"
#include "asci.h"
#include "device.h"

#define kio_device asci_0

extern struct device_char *kio_device;

void kio_init(void) {}

void kio_putc(char c) {
    device_char_write(kio_device, &c, 1, 0);
}

void kio_puts(char *s) {
    while (*s) {
        kio_putc(*(s++));
    }
}

void kio_put_uc(unsigned char n) {
    for (signed char i = 2 * sizeof(n) - 1; i >= 0; --i) {
        char digit = (n >> (i * 4)) & 0xF;
        if (digit <= 9)
            digit += '0';
        else
            digit += 'A' - 0xA;
        kio_putc(digit);
    }
}

void kio_put_us(unsigned short n) {
    for (signed char i = 2 * sizeof(n) - 1; i >= 0; --i) {
        char digit = (n >> (i * 4)) & 0xF;
        if (digit <= 9)
            digit += '0';
        else
            digit += 'A' - 0xA;
        kio_putc(digit);
    }
}

void kio_put_ui(unsigned int n) {
    for (signed char i = 2 * sizeof(n) - 1; i >= 0; --i) {
        char digit = (n >> (i * 4)) & 0xF;
        if (digit <= 9)
            digit += '0';
        else
            digit += 'A' - 0xA;
        kio_putc(digit);
    }
}

void kio_put_ul(unsigned long n) {
    for (signed char i = 2 * sizeof(n) - 1; i >= 0; --i) {
        char digit = (n >> (i * 4)) & 0xF;
        if (digit <= 9)
            digit += '0';
        else
            digit += 'A' - 0xA;
        kio_putc(digit);
    }
}
