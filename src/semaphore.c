#include "semaphore.h"
#include <cpu.h>
#include <intrinsic.h>

void semaphore_init(semaphore_t *smphr) {
    p_list_init(&smphr->blocked_list);
    smphr->count = 1;
}

void semaphore_wait(semaphore_t *smphr) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    smphr->count -= 1;

    while (smphr->count < 0) {
        current_proc->state = BLOCKED;
        p_list_push_back(&smphr->blocked_list, current_proc);
        process_schedule();
    }
    
    cpu_set_int_state(int_state);
}

bool semaphore_trywait(semaphore_t *smphr) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    if (smphr->count == 0) {
        cpu_set_int_state(int_state);
        return false;
    }

    smphr->count -= 1;
    
    cpu_set_int_state(int_state);
    return true;
}

void semaphore_signal(semaphore_t *smphr) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    smphr->count += 1;

    struct process *unlocked_process = p_list_pop_front(&smphr->blocked_list);
    if (unlocked_process) {
        unlocked_process->state = READY;
        p_list_push_back(&process_ready_list, unlocked_process);
    }
    
    cpu_set_int_state(int_state);
}
