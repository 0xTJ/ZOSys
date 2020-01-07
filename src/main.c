#include "mem.h"
#include "process.h"
#include "prt.h"
#include "dma.h"
#include "asci.h"
#include "ds1302.h"
#include "context.h"
#include <cpu.h>
#include <stdint.h>
#include <string.h>
#include <z88dk.h>
#include <z180.h>
#include <malloc.h>
#include <arch/scz180.h>

#pragma portmode z180

unsigned char _malloc_block[0x1000];
unsigned char *_malloc_heap = _malloc_block;

int fork(void) __naked;
pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus, int options) __naked;
void init(void);

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

    int status = 0;

    asci_0_put_ui(wait(&status));
    asci_0_putc('\n');
    asci_0_put_ui(status);
    asci_0_putc('\n');

    while (1)
        ;
}

void init(void) {
    while (1) {
        cpu_delay_ms(250);
        struct tm time;
        ds1302_time_get(&time);
        asci_0_put_uc(time.tm_hour);
        asci_0_putc(':');
        asci_0_put_uc(time.tm_min);
        asci_0_putc(':');
        asci_0_put_uc(time.tm_sec);
        asci_0_putc('\n');
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

void trap(uintptr_t pc) {
    bool byte_3 = ITC & __IO_ITC_UFO;
    pc -= (byte_3 ? 2 : 1);
    asci_0_puts("Undefined Op Code fetch occured in PID ");
    asci_0_put_uc(current_proc->pid);
    asci_0_puts(" at address ");
    asci_0_put_ui(pc);
    asci_0_puts(": ");
    asci_0_put_uc(cpu_bpeek(pc));
    asci_0_putc(' ');
    asci_0_put_uc(cpu_bpeek(pc + 1));
    asci_0_putc(' ');
    if (!byte_3)
        asci_0_putc('(');
    asci_0_put_uc(cpu_bpeek(pc + 2));
    if (!byte_3)
        asci_0_putc(')');
    asci_0_putc('\n');
    while (1)
        ;
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
    current_proc->sp = interrupt_sp;
    current_proc->cbar = interrupt_cbar;
    current_proc->cbr = CBR;
    current_proc = next_proc;
    CBR = current_proc->cbr;
    interrupt_cbar = current_proc->cbar;
    interrupt_sp = current_proc->sp;
    io_led_output = current_proc->pid;
}
