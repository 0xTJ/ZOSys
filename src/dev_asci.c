#include "device.h"
#include "module.h"
#include "circular_buffer.h"
#include <cpu.h>
#include <intrinsic.h>

#pragma portmode z180

#define ASCI0_FULL_SPACE_LEFT 8
#define ASCI0_EMPTY_SPACE_LEFT 8

#define ASCI0_RX_BUF_SIZE 32
#define ASCI0_TX_BUF_SIZE 32
#define ASCI1_RX_BUF_SIZE 32
#define ASCI1_TX_BUF_SIZE 32

int dev_asci_init(void);
void dev_asci_exit(void);

void asci_0_init(void);
void asci_1_init(void);

struct module dev_asci_module = {
    dev_asci_init,
    dev_asci_exit
};

struct device_driver asci_driver = {
    "ASCI",
    dev_dummy_open,
    dev_dummy_close,
    asci_read,
    asci_write
};

unsigned char asci0_rx_buf[ASCI0_RX_BUF_SIZE];
struct circular_buffer asci0_rx_circ_buf = {
    asci0_rx_buf,
    sizeof(asci0_rx_buf)
};

char asci0_tx_buf[ASCI0_TX_BUF_SIZE];
struct circular_buffer asci0_tx_circ_buf = {
    asci0_tx_buf,
    sizeof(asci0_tx_buf)
};

unsigned char asci1_rx_buf[ASCI1_RX_BUF_SIZE];
struct circular_buffer asci1_rx_circ_buf = {
    asci1_rx_buf,
    sizeof(asci1_rx_buf)
};

char asci1_tx_buf[ASCI1_TX_BUF_SIZE];
struct circular_buffer asci1_tx_circ_buf = {
    asci1_tx_buf,
    sizeof(asci1_tx_buf)
};

int dev_asci_init(void) {
    asci_0_init();
    asci_1_init();

    device_register_driver(0, &asci_driver);

    return 0;
}

void dev_asci_exit(void) {
    // TODO: Unregister driver

    return;
}

void asci_0_init(void) __critical {
    CNTLA0 = __IO_CNTLA0_RE | __IO_CNTLA0_TE | __IO_CNTLA0_MODE_8N1;
    CNTLB0 = __IO_CNTLB0_SS_DIV_1;
    STAT0 = __IO_STAT0_RIE;
}

void asci_1_init(void) __critical {
    CNTLA1 = __IO_CNTLA1_TE | __IO_CNTLA1_CKA1D | __IO_CNTLA1_MODE_8N1;
    CNTLB1 = __IO_CNTLB1_SS_DIV_1;
    STAT1 = __IO_STAT1_RIE;
}

int asci_0_putc(char c) __critical {
    if (circular_buffer_put(&asci0_tx_circ_buf, c) < 1) {
        return -1;
    }

    STAT0 |= __IO_STAT0_TIE;

    return (unsigned char) c;
}

int asci_0_getc(void) __critical {
    int c;
    if ((c = circular_buffer_get(&asci0_rx_circ_buf)) < 1) {
        return -1;
    }

    return (unsigned char) c;
}

int asci_1_putc(char c) __critical {
    if (circular_buffer_put(&asci1_tx_circ_buf, c) < 1) {
        return -1;
    }

    STAT1 |= __IO_STAT1_TIE;

    return (unsigned char) c;
}

int asci_1_getc(void) __critical {
    int c;
    if ((c = circular_buffer_get(&asci1_rx_circ_buf)) < 1) {
        return -1;
    }

    return (unsigned char) c;
}

ssize_t asci_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    (void) pos;

    ssize_t result = -1;
    size_t i;

    if (file_ptr->special.minor == 0) {
        for (i = 0; i < count; ++i) {
            int tmp;
            if ((tmp = asci_0_getc()) < 0)
                break;
            buf[i] = tmp;
        }
        if (i != 0) {
            result = i;
        }
    } else if (file_ptr->special.minor == 1) {
        for (i = 0; i < count; ++i) {
            int tmp;
            if ((tmp = asci_1_getc()) < 0)
                break;
            buf[i] = tmp;
        }
        if (i != 0) {
            result = i;
        }
    } else {
        // Not ASCI0 or ASCI1
    }

    return result;
}

ssize_t asci_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos) {
    (void) pos;

    ssize_t result = -1;
    size_t i;

    if (file_ptr->special.minor == 0) {
        for (i = 0; i < count; ++i) {
            if (asci_0_putc(buf[i]) < 0)
                break;
        }
        result = i;
    } else if (file_ptr->special.minor == 1) {
        for (i = 0; i < count; ++i) {
            if (asci_1_putc(buf[i]) < 0)
                break;
        }
        result = i;
    } else {
        // Not ASCI0 or ASCI1
    }

    return result;
}

void int_asci0(void) {
    uint8_t stat = STAT0;

    if (stat & __IO_STAT0_RDRF) {
        // Received a byte
        char receive_byte = RDR0;
        circular_buffer_put(&asci0_rx_circ_buf, receive_byte);
        if (circular_buffer_space_left(&asci0_rx_circ_buf) < ASCI0_FULL_SPACE_LEFT)
            CNTLA0 = CNTLA0 | __IO_CNTLA0_RTS0 | __IO_CNTLA0_EFR;
    }

    if (stat & __IO_STAT0_TDRE && stat & __IO_STAT0_TIE) {
        // Ready to send a byte
        int send_byte = circular_buffer_get(&asci0_tx_circ_buf);
        if (circular_buffer_space_left(&asci0_rx_circ_buf) >= ASCI0_EMPTY_SPACE_LEFT)
            CNTLA0 = (CNTLA0 & ~__IO_CNTLA0_RTS0) | __IO_CNTLA0_EFR;
        if (send_byte < 0) {
            STAT0 &= ~__IO_STAT0_TIE;
        } else {
            TDR0 = send_byte;
        }
    }
}

void int_asci1(void) {
    uint8_t stat = STAT1;

    if (stat & __IO_STAT1_RDRF) {
        // Received a byte
        char receive_byte = RDR1;
        circular_buffer_put(&asci1_rx_circ_buf, receive_byte);
    }

    if (stat & __IO_STAT1_TDRE && stat & __IO_STAT1_TIE) {
        // Ready to send a byte
        int send_byte = circular_buffer_get(&asci1_tx_circ_buf);
        if (send_byte < 1) {
            STAT1 &= ~__IO_STAT1_TIE;
        } else {
            TDR1 = send_byte;
        }
    }
}
