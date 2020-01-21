#include "mem.h"
#include "process.h"
#include "prt.h"
#include "device.h"
#include "dma.h"
#include "kio.h"
#include "io_system.h"
#include "spi.h"
#include "sd.h"
#include "ds1302.h"
#include "context.h"
#include <cpu.h>
#include <intrinsic.h>
#include <stdint.h>
#include <string.h>
#include <z88dk.h>
#include <z180.h>
#include <malloc.h>
#include <arch/scz180.h>

#pragma portmode z180

int fork(void);
pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus, int options);
int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, char *buf, size_t count, unsigned long pos);
ssize_t write(int fd, const char *buf, size_t count, unsigned long pos);

void init(void);

extern uintptr_t syscall_sp;

int main(void) {
    CMR = __IO_CMR_X2;

    // Reserve current page in use
    if (mem_alloc_page_block_specific(CBR) < 0)
        while (1)
            ;

    kio_init();
    dma_0_init();
    io_system_init();
    spi_init();
    sd_init();

    // Create process information structures
    kio_puts("Initializing processes\n");
    process_init();
    kio_puts("Initialized processes\n");

    // Start PRT0
    kio_puts("Initializing PRT0\n");
    const uint16_t clock_count = __CPU_CLOCK / __CPU_TIMER_SCALE / __CLOCKS_PER_SECOND;
    prt_interrupt_routine_0(&process_tick);
    prt_start_0(clock_count, true);
    kio_puts("Initialized PRT0\n");
    
    // Start init process at PID 1
    kio_puts("Starting init process\n");
    pid_t pid = sys_fork();
    if(pid == 0) {
        uintptr_t ret_addr = (uintptr_t) init;
        dma_memcpy(pa_from_pfn(CBR) + 0xEFFE, pa_from_pfn(CBR) + (uintptr_t) &ret_addr, 2);
        syscall_sp = 0xEFFE;
        syscall_leave();
        while (1)
            ;
    }
    kio_puts("Started init process\n");

    kio_puts("Started ZOSYS\n");

    int status = 0;

    sys_waitpid(-1, (uintptr_t) &status, WNOHANG);

    while (1) {
        kio_puts("Running\n");
        cpu_delay_ms(250);
    }
}

void init(void) {
    int stdin = open("Z:asci0", 0);
    int stdout = open("Z:asci0", 0);
    int stderr = open("Z:asci0", 0);

    write(stdout, "hello", 5, 0);

    while (1) {
        cpu_delay_ms(250);
        struct tm time;
        ds1302_time_get(&time);
        kio_put_uc(time.tm_hour);
        kio_putc(':');
        kio_put_uc(time.tm_min);
        kio_putc(':');
        kio_put_uc(time.tm_sec);
        kio_putc('\n');
    }
}

pid_t fork(void) __naked {
    __asm__("ld a, 0\nrst 8\nret");
}

pid_t wait(int *wstatus) {
    return waitpid(-1, wstatus, 0);
}

pid_t waitpid(pid_t pid, int *wstatus, int options) __naked {
    __asm__("ld a, 1\nrst 8\nret");
    (void) pid, (void) wstatus, (void) options;
}

int open(const char *pathname, int flags) __naked {
    __asm__("ld a, 2\nrst 8\nret");
    (void) pathname, (void) flags;
}

int close(int fd) __naked {
    __asm__("ld a, 3\nrst 8\nret");
    (void) fd;
}

ssize_t read(int fd, char *buf, size_t count, unsigned long pos) __naked {
    __asm__("ld a, 4\nrst 8\nret");
    (void) fd, (void) buf, (void) count, (void) pos;
}

ssize_t write(int fd, const char *buf, size_t count, unsigned long pos) __naked {
    __asm__("ld a, 5\nrst 8\nret");
    (void) fd, (void) buf, (void) count, (void) pos;
}

void trap(uintptr_t pc) {
    bool byte_3 = ITC & __IO_ITC_UFO;
    ITC &= 0x7F;
    pc -= (byte_3 ? 2 : 1);
    kio_puts("Undefined Op Code fetch occured in PID ");
    kio_put_uc(current_proc->pid);
    kio_puts(" at address ");
    kio_put_ui(pc);
    kio_puts(": ");
    kio_put_uc(cpu_bpeek(pc));
    kio_putc(' ');
    kio_put_uc(cpu_bpeek(pc + 1));
    kio_putc(' ');
    if (!byte_3)
        kio_putc('(');
    kio_put_uc(cpu_bpeek(pc + 2));
    if (!byte_3)
        kio_putc(')');
    kio_putc('\n');
    while (1)
        ;
}

void int_0(void) {
    kio_puts("INT0\n");
    intrinsic_ei();
}

void int_1(void) {
    kio_puts("INT1\n");
    intrinsic_ei();
}

void int_2(void) {
    kio_puts("INT2\n");
    intrinsic_ei();
}

void int_no_vector(void) {
    kio_puts("Unhandled Interrupt Occured\n");
    intrinsic_ei();
}
