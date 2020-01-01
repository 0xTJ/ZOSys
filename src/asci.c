#include "asci.h"
#include <cpu.h>

#pragma portmode z180

void asci_0_setup(void) {
    CNTLA0 = __IO_CNTLA0_TE | __IO_CNTLA0_RTS0 | __IO_CNTLA0_MODE_8N1;
    CNTLB0 = __IO_CNTLB0_PS;
}

void asci_0_putc(char c) {
    while (!(STAT0 & 0x02))
        ;
    TDR0 = c;
}

void asci_0_puts(char *s) {
    while (*s) {
        asci_0_putc(*(s++));
    }
}

void asci_0_put_uc(unsigned char n) {
    for (signed char i = 2 * sizeof(n) - 1; i >= 0; --i) {
        char digit = (n >> (i * 4)) & 0xF;
        if (digit <= 9)
            digit += '0';
        else
            digit += 'A' - 0xA;
        asci_0_putc(digit);
    }
}

void asci_0_put_u(unsigned n) {
    for (signed char i = 2 * sizeof(n) - 1; i >= 0; --i) {
        char digit = (n >> (i * 4)) & 0xF;
        if (digit <= 9)
            digit += '0';
        else
            digit += 'A' - 0xA;
        asci_0_putc(digit);
    }
}

void asci_0_put_ul(unsigned long n) {
    for (signed char i = 2 * sizeof(n) - 1; i >= 0; --i) {
        char digit = (n >> (i * 4)) & 0xF;
        if (digit <= 9)
            digit += '0';
        else
            digit += 'A' - 0xA;
        asci_0_putc(digit);
    }
}
