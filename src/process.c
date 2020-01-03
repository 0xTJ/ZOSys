#include "process.h"
#include "context.h"
#include "dma.h"
#include <cpu.h>
#include <stdlib.h>

#include "asci.h"

volatile p_list_t proc_list;
volatile struct process *current_proc = NULL;
volatile pid_t next_pid = 0;

void process_init(void) {
    p_list_init(&proc_list);
}

struct process *process_new(void) {
    // Allocate new process struct
    struct process *new_proc = malloc(sizeof(struct process));
    if (!new_proc)
        return NULL;

    // Save and disable interrupts
    uint8_t int_state = cpu_get_int_state();
    cpu_set_int_state(0);

    // Setup process fields
    new_proc->pid = next_pid++;
    new_proc->state = EMPTY;

    // Add process to list
    p_list_push_back(&proc_list, new_proc);

    // Restore interrupts before returning
    cpu_set_int_state(int_state);
    return new_proc;
}

int sys_fork(void) {
    struct process *child_proc = process_new();
    if (!child_proc)
        return -1;

    pid_t parent_pid = current_proc->pid;
    pid_t child_pid = child_proc->pid;

    unsigned char parent_cbr = current_proc->cbr;
    unsigned char child_cbr = 0x80 + 0x10 * child_proc->pid;

    unsigned long parent_addr_base = (unsigned long) parent_cbr << 12;
    unsigned long child_addr_base = (unsigned long) child_cbr << 12;

    child_proc->cbr = child_cbr;

    __asm__("di");

    dma_addr_0(parent_addr_base, child_addr_base, 0);
    dma_mode_0(MEMORY_INC, MEMORY_INC, true);

    __asm__("EXTERN context_save\nld hl, child_reentry\npush hl\ncall context_save");   // From this point until the restore, can't use stack variables

    dma_enable_0();

    __asm__("EXTERN context_restore\ncall context_restore\npop hl");

    child_proc->cbar = interrupt_cbar;
    child_proc->sp = interrupt_sp;

    __asm__("ei\n");

    child_proc->state = READY;

    __asm__("child_reentry:");

    if (current_proc->pid == parent_pid)
        return child_pid;
    else
        return 0;
}
