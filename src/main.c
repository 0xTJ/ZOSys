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
    asci_0_puts("Starting ZOSYS\n");

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
    new_proc->bbr = context_temp_bbr;
    context_temp_sp = new_proc->sp;
    context_init(loop);
    new_proc->sp = context_temp_sp;
    context_temp_sp = (uint16_t) 0x10000;

    asci_0_puts("Started ZOSYS\n");

    while (1) {
        io_led_output = (a ^= (1 << 4));
        for (volatile int i = 0; i < 8000; ++i) {

        }
    }
}

void int0(void) {
    asci_0_puts("INT0\n");
}

void loop(void) {
    while (1) {
        io_led_output = (a ^= (1 << 5));
        for (volatile int i = 0; i < 8000; ++i) {

        }
    }
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
    // asci_0_puts("Saved SP: ");
    // asci_0_put_ul(context_temp_sp);
    // asci_0_putc('\n');
    // asci_0_puts("Saved BBR: ");
    // asci_0_put_ul(context_temp_bbr);
    // asci_0_putc('\n');
    // asci_0_puts("Saved PC: ");
    // asci_0_put_ul(((uint16_t *) context_temp_sp)[10]);
    // asci_0_putc('\n');
    current_proc->sp = context_temp_sp;
    current_proc->bbr = context_temp_bbr;
    context_temp_sp = next_proc->sp;
    context_temp_bbr = next_proc->bbr;
    current_proc = next_proc;
    // asci_0_puts("Restored SP: ");
    // asci_0_put_ul(context_temp_sp);
    // asci_0_putc('\n');
    // asci_0_puts("Restored BBR: ");
    // asci_0_put_ul(context_temp_bbr);
    // asci_0_putc('\n');
    // asci_0_puts("Restored PC: ");
    // asci_0_put_ul(((uint16_t *) context_temp_sp)[10]);
    // asci_0_putc('\n');
    io_led_output = (a = ((a & 0xF0) | current_proc->pid));
}
