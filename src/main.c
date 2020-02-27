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

extern uintptr_t syscall_sp;

extern unsigned char trap_cbar;

extern struct module dev_asci_module;
extern struct module dev_sd_module;
extern struct module fs_dev_module;
extern struct module fs_initrd_module;

char init_path[] = "Y:init";
char *argv[] = { "init", NULL };
char *envp[] = { "PATH=\"Y:\"", NULL };

int main(void) {
    CMR = __IO_CMR_X2;

    // Reserve current page in use
    if (mem_alloc_page_block_specific(CBR) < 0)
        while (1)
            ;

    module_init(&dev_asci_module);
    dma_0_init();
    module_init(&fs_dev_module);
    module_init(&fs_initrd_module);
    io_system_init();
    spi_init();
    module_init(&dev_sd_module);

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

        // Copy init binary to run location
        // This only work because parameters is also copied to user space on fork
        sys_execve((uintptr_t) init_path, (uintptr_t) argv, (uintptr_t) envp);

        syscall_leave();
        while (1)
            ;
    }

    while (sys_waitpid(1, 0, WNOHANG) <= 0) {
        // Loop while process 1 exists as a child of process 0
        // Busy loop because this is the lowest priority process and
        // must remain runnable
    }

    kio_puts("System is halting\n");

    return 0;
}

void trap(uintptr_t pc) {
    bool byte_3 = ITC & __IO_ITC_UFO;
    ITC &= ~__IO_ITC_TRAP;
    uintptr_t op_pc = pc - (byte_3 ? 2 : 1);
    bool in_kernel = false;
    if (trap_cbar == 0xF1) {
        // Kernel space
        in_kernel = true;
    }
    kio_puts("Undefined Op Code fetch caused by ");
    kio_puts(in_kernel ? "kernel" : "user");
    kio_puts(" in PID ");
    kio_put_uc(current_proc->pid);
    kio_puts(" at address ");
    kio_put_ui(op_pc);
    kio_puts(": ");
    kio_put_uc(cpu_bpeek(op_pc));
    kio_putc(' ');
    if (!byte_3)
        kio_putc('*');
    kio_put_uc(cpu_bpeek(op_pc + 1));
    if (!byte_3)
        kio_putc('*');
    kio_putc(' ');
    if (!byte_3)
        kio_putc('(');
    else
        kio_putc('*');
    kio_put_uc(cpu_bpeek(op_pc + 2));
    if (!byte_3)
        kio_putc(')');
    else
        kio_putc('*');
    kio_putc('\n');
    if (in_kernel) {
        // panic();
        // TODO: Add panic()
    } else {
        // TODO: Set the value here correctly
        sys_exit(129);
    }
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
