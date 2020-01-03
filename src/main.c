#include "mem.h"
#include "process.h"
#include "prt.h"
#include "dma.h"
#include "asci.h"
#include "context.h"
#include <arch/scz180.h>
#include <cpu.h>
#include <stdint.h>
#include <string.h>
#include <z88dk.h>
#include <z180.h>
#include <malloc.h>

#pragma portmode z180

unsigned char _malloc_block[0x1000];
unsigned char *_malloc_heap = _malloc_block;

int fork(void) __naked;
void init(void);

volatile unsigned char a;

int main(void) {
    CMR = __IO_CMR_X2;
    heap_init(_malloc_heap, sizeof(_malloc_block));

    CBAR = 0x11;

    asci_0_setup();
    asci_0_puts("\nStarting ZOSYS\n");

    // Create process information structures
    asci_0_puts("Initializing processes\n");
    process_init();
    current_proc = process_new();
    current_proc->state = RUNNING;
    current_proc->cbr = 0x80;
    asci_0_puts("Initialized processes\n");

    // Start PRT0
    asci_0_puts("Initializing PRT0\n");
    const uint16_t clock_count = __CPU_CLOCK / __CPU_TIMER_SCALE / __CLOCKS_PER_SECOND;
    prt_start_0(clock_count, true);
    asci_0_puts("Initialized PRT0\n");
    
    // Start init process at PID 1
    asci_0_puts("Starting init process\n");
    pid_t pid = fork();
    if(pid == 0) {
        init();
        while (1)
            ;
    }
    asci_0_puts("Started init process\n");

    asci_0_puts("Started ZOSYS\n");

    while (1)
        ;
}

void init(void) {
    while (1)
        ;
}

int fork(void) __naked {
    __asm__("ld a, 0\nrst 8\nret");
}

void sys_1(unsigned int n) {
    asci_0_puts("syscall 1: ");
    asci_0_put_ui(n);
    asci_0_putc('\n');
}

void int_0(void) {
    asci_0_puts("INT0\n");
}

void int_prt0(void) {
    (void) TCR;
    (void) TMDR0L;
    current_proc->state = READY;
    struct process *next_proc = current_proc;
    do {
        next_proc = p_list_next(next_proc);
        if (!next_proc) {
            next_proc = p_list_front(&proc_list);
        }
        if (!next_proc) {
            // Process list is empty
            // TODO: This is fatal, treat is as such
            current_proc->state = RUNNING;
            return;
        }
    } while (next_proc->state != READY);
    // asci_0_puts("Saved PID: ");
    // asci_0_put_ui(current_proc->pid);
    // asci_0_putc('\n');
    // asci_0_puts("Saved SP: ");
    // asci_0_put_ui(interrupt_sp);
    // asci_0_putc('\n');
    // asci_0_puts("Saved PC: ");
    // asci_0_put_ui(((struct context_stack *) interrupt_sp)->pc);
    // asci_0_putc('\n');
    // asci_0_puts("Saved CBAR: ");
    // asci_0_put_uc(interrupt_cbar);
    // asci_0_putc('\n');
    // asci_0_puts("Saved CBR: ");
    // asci_0_put_uc(CBR);
    // asci_0_putc('\n');
    current_proc->sp = interrupt_sp;
    current_proc->cbar = interrupt_cbar;
    current_proc->cbr = CBR;
    CBR = next_proc->cbr;
    interrupt_cbar = next_proc->cbar;
    interrupt_sp = next_proc->sp;
    current_proc = next_proc;
    // asci_0_puts("Restored PID: ");
    // asci_0_put_ui(current_proc->pid);
    // asci_0_putc('\n');
    // asci_0_puts("Restored SP: ");
    // asci_0_put_ui(interrupt_sp);
    // asci_0_putc('\n');
    // asci_0_puts("Restored PC: ");
    // asci_0_put_ui(((struct context_stack *) interrupt_sp)->pc);
    // asci_0_putc('\n');
    // asci_0_puts("Restored CBAR: ");
    // asci_0_put_uc(interrupt_cbar);
    // asci_0_putc('\n');
    // asci_0_puts("Restored CBR: ");
    // asci_0_put_uc(CBR);
    // asci_0_putc('\n');
    io_led_output = (a = ((a & 0xF0) | current_proc->pid));
}
