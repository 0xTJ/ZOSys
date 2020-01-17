#include "asci.h"
#include <cpu.h>

#pragma portmode z180

struct device_char_driver asci_driver = {
    dummy_char_open,
    dummy_char_close,
    dummy_char_read,
    asci_write
};

struct device_char *asci_0;
struct device_char *asci_1;

void asci_0_init(void) {
    CNTLA0 = __IO_CNTLA0_TE | __IO_CNTLA0_RTS0 | __IO_CNTLA0_MODE_8N1;
    CNTLB0 = __IO_CNTLB0_PS;

    asci_0 = device_char_new(&asci_driver);

    if (!asci_0)
        return;

    asci_0->data_ui = 0;

    mutex_unlock(&asci_0->mtx);
}

void asci_1_init(void) {
    CNTLA1 = __IO_CNTLA1_TE | __IO_CNTLA1_CKA1D | __IO_CNTLA1_MODE_8N1;
    CNTLB1 = __IO_CNTLB1_PS;

    asci_1 = device_char_new(&asci_driver);

    if (!asci_1)
        return;

    asci_1->data_ui = 1;

    mutex_unlock(&asci_1->mtx);
}

void asci_0_putc(char c) {
    while (!(STAT0 & 0x02))
        ;
    TDR0 = c;
}

void asci_1_putc(char c) {
    while (!(STAT1 & 0x02))
        ;
    TDR1 = c;
}

ssize_t asci_write(struct device_char *dev, const char *buf, size_t count, unsigned long pos) {
    (void) pos;

    ssize_t result = -1;

    mutex_lock(&dev->mtx);

    if (dev->data_ui == 0) {
        for (size_t i = 0; i < count; ++i) {
            asci_0_putc(buf[i]);
        }
        result = count;
    } else if (dev->data_ui == 1) {
        for (size_t i = 0; i < count; ++i) {
            asci_1_putc(buf[i]);
        }
        result = count;
    } else {
        // Not ASCI0 or ASCI1
    }

    mutex_unlock(&dev->mtx);

    return result;
}

void int_asci0(void) {
    // TODO: Implement this
}

void int_asci1(void) {
    // TODO: Implement this
}
