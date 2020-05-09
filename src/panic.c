#include "panic.h"
#include "kio.h"
#include <cpu.h>
#include <intrinsic.h>

#pragma portmode z180

inline void disable_all_interrupts(void) __critical {
    ITC &= ~(__IO_ITC_ITE2 | __IO_ITC_ITE1 | __IO_ITC_ITE0);
    TCR &= ~(__IO_TCR_TIE1 | __IO_TCR_TIE0);
    DSTAT &= ~(__IO_DSTAT_DIE1 | __IO_DSTAT_DIE0);
    CNTR &= ~(__IO_CNTR_EIE);
    STAT0 &= ~(__IO_STAT0_RIE | __IO_STAT0_TIE);
    STAT1 &= ~(__IO_STAT1_RIE | __IO_STAT1_TIE);
}

void process_dump_all(void);

void panic_no_message(const char *file, const char *line) {
    disable_all_interrupts();
    intrinsic_ei();
    kio_puts("\n***\nKernel panic at ");
    kio_puts(line);
    kio_puts(":");
    kio_puts(file);
    kio_puts("\n***\n");
    process_dump_all();
    while (1) {
        // Infinite loop
    }
}

void panic_message(const char *file, const char *line, const char *err_message) {
    disable_all_interrupts();
    intrinsic_ei();
    kio_puts("\n***\nKernel panic at ");
    kio_puts(line);
    kio_puts(":");
    kio_puts(file);
    kio_puts(":\n");
    kio_puts(err_message);
    kio_puts("\n***\n");
    while (1) {
        // Infinite loop
    }
}
