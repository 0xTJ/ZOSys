#include "process.h"
#include "context.h"
#include "dma.h"
#include "mutex.h"
#include <arch/scz180.h>
#include <cpu.h>
#include <z180.h>
#include <errno.h>
#include <intrinsic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#pragma portmode z180

volatile p_list_t process_ready_list;
volatile p_list_t process_zombie_list;
volatile p_list_t process_wait_list;

volatile struct process *current_proc = NULL;
volatile pid_t next_pid = 0;

void process_init(void) {
    p_list_init(&process_ready_list);
    p_list_init(&process_zombie_list);

    current_proc = process_new();
    current_proc->state = RUNNING;
    current_proc->cbr = CBR;
}

struct process *process_new(void) {
    // Allocate new process struct
    struct process *new_proc = malloc(sizeof(struct process));
    if (!new_proc)
        return NULL;

    // Setup process fields
    new_proc->pid = next_pid++;
    new_proc->ppid = 0;
    new_proc->state = EMPTY;
    memset(new_proc->open_files, 0, sizeof(new_proc->open_files));

    return new_proc;
}

/*
+--------+
| Return |
+--------+
|   IX   |
+--------+
|   IY   |
+--------+
*/
// Must enter with interrupts disabled
// Task to be switched from must already have set to not running
void process_switch(struct process *next_proc) __z88dk_fastcall {
    __asm__("\
        ld bc, hl \n\
        push ix \n\
        push iy \n\
        ld iy, (_current_proc) \n\
        in0 a, (_CBR) \n\
        ld (iy+" PROCESS_OFFSETOF_CBR_STR "), a \n\
        ld hl, 0 \n\
        add hl, sp \n\
        ld (iy+" PROCESS_OFFSETOF_SP_STR "), l \n\
        ld (iy+" PROCESS_OFFSETOF_SP_STR "+1), h \n\
        ");
    __asm__("\
        ld (_current_proc), bc \n\
        ");
    __asm__("\
        ld iy, (_current_proc) \n\
        ld l, (iy+" PROCESS_OFFSETOF_SP_STR ") \n\
        ld h, (iy+" PROCESS_OFFSETOF_SP_STR "+1) \n\
        ld sp, hl \n\
        ld a, (iy+" PROCESS_OFFSETOF_CBR_STR ") \n\
        out0 (_CBR), a \n\
        ld a, " PROC_STATE_RUNNING_STR "\n\
        ld (iy+" PROCESS_OFFSETOF_STATE_STR "), a \n\
        pop iy \n\
        pop ix \n\
        ");
    io_led_output = current_proc->pid;
    (void) next_proc;
}

// Must enter with interrupts disabled
void process_schedule(void) {
    struct process *next_proc = p_list_pop_front(&process_ready_list);

    if (next_proc->pid == 0) {
        // Is idle process
        if (!p_list_empty(&process_ready_list)) {
            // Another process can run
            p_list_push_back(&process_ready_list, next_proc);
            next_proc = p_list_pop_front(&process_ready_list);
        }
    }

    if (next_proc) {
        process_switch(next_proc);
    } else {
        // TODO: Panic, idle task is not available
        while (1)
            ;
    }
}

// Must enter with interrupts disabled
void process_tick(void) {
    // Add current process to ready list
    current_proc->state = READY;
    p_list_push_back(&process_ready_list, current_proc);

    process_schedule();
}

uintptr_t sys_fork_helper(unsigned char child_cbr) __z88dk_fastcall __naked {
    __asm__("\
        di \n\
        \n\
        pop de \n\
        \n\
        in0 a, (_CBR) \n\
        out0 (_CBR), l \n\
        \n\
        ld hl, sys_fork_child_reentry \n\
        push de \n\
        push hl \n\
        push ix \n\
        push iy \n\
        \n\
        ld hl, 0 \n\
        add hl, sp \n\
        \n\
        out0 (_CBR), a \n\
        \n\
        pop bc \n\
        pop bc \n\
        pop bc \n\
        pop bc \n\
        \n\
        push de \n\
        \n\
sys_fork_child_reentry: \n\
        ei \n\
        ret \n\
        ");
    (void) child_cbr;
}

pid_t sys_fork(void) {
    struct process *child_proc = process_new();
    if (!child_proc)
        return -1;

    pid_t parent_pid = current_proc->pid;
    pid_t child_pid = child_proc->pid;

    unsigned char parent_cbr = CBR;
    unsigned char child_cbr = mem_alloc_page_block();

    unsigned long parent_addr_base = pa_from_pfn(parent_cbr);
    unsigned long child_addr_base = pa_from_pfn(child_cbr);

    child_proc->ppid = parent_pid;
    child_proc->cbr = child_cbr;

    dma_memcpy(child_addr_base, parent_addr_base, 0);
    // Bad things may happen if anything important on the stack changes between these lines
    child_proc->sp = sys_fork_helper(child_cbr);

    if (current_proc->pid == parent_pid) {
        // Add child to ready list
        uint8_t int_state = cpu_get_int_state();
        intrinsic_di();
        child_proc->state = READY;
        p_list_push_back(&process_ready_list, child_proc);
        cpu_set_int_state(int_state);

        return child_pid;
    } else {
        return 0;
    }
}

pid_t sys_waitpid(pid_t pid, USER_PTR(int) wstatus, int options) {
    if (options & ~(WNOHANG))
        return -1;
    bool nohang = options & WNOHANG;

    pid_t current_pid = current_proc->pid;

    struct process *found_process = NULL;
    bool child_exists = false;

    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();
    struct process *search_proc = p_list_front(&process_zombie_list);

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

    if (!found_process && !nohang) {
        current_proc->wait_pid = pid;
        current_proc->wait_options = options;
        current_proc->wait_process = NULL;

        // Add to list waiting
        current_proc->state = BLOCKED;
        p_list_push_back(&process_wait_list, current_proc);
        process_schedule();

        found_process = current_proc->wait_process;
    }

    cpu_set_int_state(int_state);

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
