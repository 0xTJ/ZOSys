#include "kio.h"

int asci_0_putc(char c);

void kio_putc(char c) {
    asci_0_putc(c);
}

void kio_puts(const char *s) {
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
