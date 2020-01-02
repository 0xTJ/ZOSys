#include "mem.h"
#include "process.h"
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

char stack2[0x1000];

unsigned char _malloc_block[0x1000];
unsigned char *_malloc_heap = _malloc_block;

void loop();

volatile unsigned char a;

int main(void) {
    CMR = __IO_CMR_X2;
    heap_init(_malloc_heap, sizeof(_malloc_block));

    asci_0_setup();
    asci_0_puts("\nStarting ZOSYS\n");

    // Create process information structures
    asci_0_puts("Initializing processes\n");
    process_init();
    current_proc = process_new();
    asci_0_puts("Initialized processes\n");

    // Start PRT0
    asci_0_puts("Initializing PRT0\n");
    const uint16_t clock_count = __CPU_CLOCK / __CPU_TIMER_SCALE / __CLOCKS_PER_SECOND;
    TMDR0L = clock_count & 0xFF;
    TMDR0H = (clock_count >> 8) & 0xFF;
    RLDR0L = (clock_count - 1) & 0xFF;
    RLDR0H = ((clock_count - 1) >> 8) & 0xFF;
    TCR = __IO_TCR_TIE0 | __IO_TCR_TDE0;
    asci_0_puts("Initialized PRT0\n");

    struct process *new_proc = process_new();
    new_proc->sp = (uint16_t) stack2 + 0x1000;
    new_proc->cbr = 0x80;
    interrupt_sp = new_proc->sp;
    context_init(loop);
    new_proc->sp = interrupt_sp;

    asci_0_puts("Started ZOSYS\n");

    while (1) {
        io_led_output = (a ^= (1 << 4));
        cpu_delay_ms(500);
    }
}

void loop(void) {
    __asm__("ld a, 0\nrst 8");
    __asm__("ld hl, 0x35A\npush hl\nld a, 1\nrst 8");
    __asm__("ld a, 2\nrst 8");
    while (1) {
        io_led_output = (a ^= (1 << 5));
        cpu_delay_ms(500);
    }
}

void sys_0(void) {
    asci_0_puts("syscall 0\n");
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
    struct process *next_proc = p_list_next(current_proc);
    if (!next_proc) {
        next_proc = p_list_front(&proc_list);
    }
    if (!next_proc) {
        return;
    }
    current_proc->sp = interrupt_sp;
    current_proc->cbr = CBR;
    interrupt_sp = next_proc->sp;
    CBR = next_proc->cbr;
    current_proc = next_proc;
    io_led_output = (a = ((a & 0xF0) | current_proc->pid));
}
