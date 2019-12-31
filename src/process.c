#include "process.h"
#include <cpu.h>
#include <stdlib.h>

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

    // Add process to list
    p_list_push_back(&proc_list, new_proc);

    // Restore interrupts before returning
    cpu_set_int_state(int_state);
    return new_proc;
}
