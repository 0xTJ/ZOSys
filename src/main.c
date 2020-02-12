#include "mem.h"
#include "process.h"
#include "prt.h"
#include "device.h"
#include "dma.h"
#include "kio.h"
#include "io_system.h"
#include "spi.h"
#include "ds1302.h"
#include "module.h"
#include "context.h"
#include "vfs.h"
#include <arch/scz180.h>
#include <cpu.h>
#include <intrinsic.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <z88dk.h>
#include <z180.h>
 
#pragma portmode z180

extern char init[0xE000];

extern uintptr_t syscall_sp;

extern struct module dev_asci_module;
extern struct module fs_dev_module;
extern struct module sd_module;

int main(void) {
    CMR = __IO_CMR_X2;

    // Reserve current page in use
    if (mem_alloc_page_block_specific(CBR) < 0)
        while (1)
            ;

    module_init(&dev_asci_module);
    dma_0_init();
    module_init(&fs_dev_module);
    io_system_init();
    spi_init();
    module_init(&sd_module);

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
        // Manually setup the user-space stack
        uintptr_t tmp_ptr = 0x0000; // Returning from init is currently an error, reset system
        dma_memcpy(pa_from_pfn(CBR) + 0xEFFE, pa_from_pfn(CBR) + (uintptr_t) &tmp_ptr, 2);
        tmp_ptr = 0x1000;
        dma_memcpy(pa_from_pfn(CBR) + 0xEFFC, pa_from_pfn(CBR) + (uintptr_t) &tmp_ptr, 2);
        dma_memcpy(pa_from_pfn(CBR) + 0x1000, pa_from_pfn(BBR) + (uintptr_t) init, sizeof(init));
        syscall_sp = 0xEFFC;
        syscall_leave();
        while (1)
            ;
    }

    while (sys_waitpid(1, 0, WNOHANG) <= 0) {
        // Loop while process 1 exists as a child of process 0
    }

    kio_puts("System is halting\n");

    return 0;
}

void trap(uintptr_t pc) {
    bool byte_3 = ITC & __IO_ITC_UFO;
    ITC &= ~__IO_ITC_TRAP;
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
    // TODO: Force process termination
    while (1)
        ;
}

void int_0(void) {
    kio_puts("INT0\n");
}

void int_1(void) {
    kio_puts("INT1\n");
}

void int_2(void) {
    kio_puts("INT2\n");
}

void int_no_vector(void) {
    kio_puts("Unhandled Interrupt Occured\n");
}
