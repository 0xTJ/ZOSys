#include "process.h"
#include <stdlib.h>

volatile p_list_t proc_list;
volatile struct process *current_proc = NULL;
volatile pid_t next_pid = 0;

void process_init(void) {
    p_list_init(&proc_list);
}

struct process *process_new(void) {
    struct process *new_proc = malloc(sizeof(struct process));
    if (!new_proc)
        return NULL;
    new_proc->pid = next_pid++;
    p_list_push_back(&proc_list, new_proc);
    return new_proc;
}
