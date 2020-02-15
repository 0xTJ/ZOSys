#include "semaphore.h"
#include <cpu.h>
#include <intrinsic.h>

void semaphore_init(semaphore_t *smphr, signed int initial_value) {
    p_list_init(&smphr->blocked_list);
    smphr->count = initial_value;
}

void semaphore_wait(semaphore_t *smphr) __critical {
    smphr->count -= 1;

    while (smphr->count < 0) {
        current_proc->state = BLOCKED;
        p_list_push_back(&smphr->blocked_list, current_proc);
        process_schedule();
    }
}

bool semaphore_trywait(semaphore_t *smphr) __critical {
    if (smphr->count == 0) {
        return false;
    }

    smphr->count -= 1;

    return true;
}

void semaphore_signal(semaphore_t *smphr) __critical {
    smphr->count += 1;

    struct process *unlocked_process = p_list_pop_front(&smphr->blocked_list);
    if (unlocked_process) {
        unlocked_process->state = READY;
        p_list_push_back(&process_ready_list, unlocked_process);
    }
}
