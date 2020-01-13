#include "asci.h"
#include <cpu.h>

#pragma portmode z180

struct device_driver asci_driver = {
    NULL,
    NULL,
    NULL,
    asci_write
};

struct device *asci_0;

void asci_0_init(void) {
    CNTLA0 = __IO_CNTLA0_TE | __IO_CNTLA0_RTS0 | __IO_CNTLA0_MODE_8N1;
    CNTLB0 = __IO_CNTLB0_PS;

    asci_0 = device_new(&asci_driver);

    if (!asci_0)
        return;

    asci_0->data_ui = 0;

    mutex_unlock(&asci_0->mtx);
}

void asci_0_putc(char c) {
    while (!(STAT0 & 0x02))
        ;
    TDR0 = c;
}

ssize_t asci_write(struct device *dev, const char *buf, size_t count, unsigned long pos) {
    (void) pos;

    ssize_t result = -1;

    mutex_lock(&dev->mtx);

    if (dev->data_ui == 0) {
        for (size_t i = 0; i < count; ++i) {
            asci_0_putc(buf[i]);
        }
        result = count;
    } else {
        // Not ASCI0
    }

    mutex_unlock(&dev->mtx);

    return result;
}
