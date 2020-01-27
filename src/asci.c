#include "asci.h"
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

struct device_char_driver asci_driver = {
    dummy_char_open,
    dummy_char_close,
    asci_read,
    asci_write
};

struct device_char *asci_0;
struct device_char *asci_1;

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

void asci_0_init(void) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    CNTLA0 = __IO_CNTLA0_RE | __IO_CNTLA0_TE | __IO_CNTLA0_MODE_8N1;
    CNTLB0 = __IO_CNTLB0_SS_DIV_1;
    STAT0 = __IO_STAT0_RIE;

    cpu_set_int_state(int_state);

    asci_0 = device_char_new(&asci_driver);

    if (!asci_0)
        return;

    asci_0->data_ui = 0;

    mutex_unlock(&asci_0->mtx);
}

void asci_1_init(void) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    CNTLA1 = __IO_CNTLA1_TE | __IO_CNTLA1_CKA1D | __IO_CNTLA1_MODE_8N1;
    CNTLB1 = __IO_CNTLB1_SS_DIV_1;
    STAT1 = __IO_STAT1_RIE;

    cpu_set_int_state(int_state);

    asci_1 = device_char_new(&asci_driver);

    if (!asci_1)
        return;

    asci_1->data_ui = 1;

    mutex_unlock(&asci_1->mtx);
}

int asci_0_putc(char c) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    if (circular_buffer_put(&asci0_tx_circ_buf, c) < 1) {
        cpu_set_int_state(int_state);
        return -1;
    }

    STAT0 |= __IO_STAT0_TIE;

    cpu_set_int_state(int_state);
    return (unsigned char) c;
}

int asci_0_getc(void) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    int c;
    if ((c = circular_buffer_get(&asci0_rx_circ_buf)) < 1) {
        cpu_set_int_state(int_state);
        return -1;
    }

    cpu_set_int_state(int_state);
    return (unsigned char) c;
}

int asci_1_putc(char c) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    if (circular_buffer_put(&asci1_tx_circ_buf, c) < 1) {
        cpu_set_int_state(int_state);
        return -1;
    }

    STAT1 |= __IO_STAT1_TIE;

    cpu_set_int_state(int_state);
    return (unsigned char) c;
}

int asci_1_getc(void) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    int c;
    if ((c = circular_buffer_get(&asci1_rx_circ_buf)) < 1) {
        cpu_set_int_state(int_state);
        return -1;
    }

    cpu_set_int_state(int_state);
    return (unsigned char) c;
}

ssize_t asci_read(struct device_char *dev, char *buf, size_t count, unsigned long pos) {
    (void) pos;

    ssize_t result = -1;
    size_t i;

    mutex_lock(&dev->mtx);

    if (dev->data_ui == 0) {
        for (i = 0; i < count; ++i) {
            int tmp;
            if ((tmp = asci_0_getc()) < 0)
                break;
            buf[i] = tmp;
        }
        result = i;
    } else if (dev->data_ui == 1) {
        for (i = 0; i < count; ++i) {
            int tmp;
            if ((tmp = asci_1_getc()) < 0)
                break;
            buf[i] = tmp;
        }
        result = i;
    } else {
        // Not ASCI0 or ASCI1
    }

    mutex_unlock(&dev->mtx);

    return result;
}

ssize_t asci_write(struct device_char *dev, const char *buf, size_t count, unsigned long pos) {
    (void) pos;

    ssize_t result = -1;
    size_t i;

    mutex_lock(&dev->mtx);

    if (dev->data_ui == 0) {
        for (i = 0; i < count; ++i) {
            if (asci_0_putc(buf[i]) < 0)
                break;
        }
        result = i;
    } else if (dev->data_ui == 1) {
        for (i = 0; i < count; ++i) {
            if (asci_1_putc(buf[i]) < 0)
                break;
        }
        result = i;
    } else {
        // Not ASCI0 or ASCI1
    }

    mutex_unlock(&dev->mtx);

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
