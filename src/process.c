#include "process.h"
#include "context.h"
#include "dma.h"
#include "mem.h"
#include "mutex.h"
#include <arch/scz180.h>
#include <cpu.h>
#include <errno.h>
#include <intrinsic.h>
#include <stdbool.h>
#include <stdlib.h>

#pragma portmode z180

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
    new_proc->ppid = -1;
    new_proc->state = EMPTY;

    // Add process to list
    p_list_push_back(&proc_list, new_proc);

    // Restore interrupts before returning
    cpu_set_int_state(int_state);
    return new_proc;
}

pid_t sys_fork(void) {
    struct process *child_proc = process_new();
    if (!child_proc)
        return -1;

    pid_t parent_pid = current_proc->pid;
    pid_t child_pid = child_proc->pid;

    unsigned char parent_cbr = current_proc->cbr;
    // TODO: Allow more than 7 processes
    unsigned char child_cbr = 0x80 + 0x10 * child_proc->pid;

    unsigned long parent_addr_base = (unsigned long) parent_cbr << 12;
    unsigned long child_addr_base = (unsigned long) child_cbr << 12;

    child_proc->ppid = parent_pid;
    child_proc->cbr = child_cbr;

    mutex_lock(&dma_0_mtx);
    dma_0_addr(parent_addr_base, child_addr_base, 0);
    dma_0_mode(MEMORY_INC, MEMORY_INC, true);

    intrinsic_di();

    // From this point until the restore, can't use non-globals
    __asm__("ld hl, sys_fork_child_reentry\npush hl");
    context_save();
    dma_0_enable();
    context_restore();
    __asm__("pop hl");

    child_proc->cbar = interrupt_cbar;
    child_proc->sp = interrupt_sp;

    intrinsic_ei();

    mutex_unlock(&dma_0_mtx);

    child_proc->state = READY;

intrinsic_label(sys_fork_child_reentry)

    if (current_proc->pid == parent_pid)
        return child_pid;
    else
        return 0;
}

pid_t sys_waitpid(pid_t pid, uintptr_t /* int * */ wstatus, int options) {
    if (options & ~(WNOHANG))
        return -1;
    bool nohang = options & WNOHANG;

    pid_t current_pid = current_proc->pid;

    struct process *found_process = NULL;
    bool child_exists = false;

    do {
        struct process *search_proc = p_list_front(&proc_list);
        while (search_proc) {
            if (search_proc->ppid == current_pid && (pid == -1 || search_proc->pid == pid)) {
                child_exists = true;
                if (search_proc->state == ZOMBIE) {
                    found_process = search_proc;
                    break;
                }
            }
            search_proc = p_list_next(search_proc);
        }
    } while (!found_process && !nohang);

    if (found_process) {
        if (wstatus)
            dma_memcpy(pa_from_pfn(CBR) + wstatus, pa_from_pfn(BBR) + (uintptr_t) &found_process->status, sizeof(found_process->status));
        return found_process->pid;
    } else if (child_exists) {
        return 0;
    } else {
        return -1;
    }
}
