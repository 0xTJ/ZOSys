#include "mutex.h"
#include "process.h"
#include <cpu.h>
#include <intrinsic.h>

void mutex_init(mutex_t *mtx) {
    p_list_init(&mtx->blocked_list);
    mtx->locked = false;
}

void mutex_lock(mutex_t *mtx) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    while (mtx->locked) {
        current_proc->state = BLOCKED;
        p_list_push_back(&mtx->blocked_list, current_proc);
        process_schedule();
    }

    mtx->locked = true;
    
    cpu_set_int_state(int_state);
}

bool mutex_trylock(mutex_t *mtx) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    if (mtx->locked) {
        cpu_set_int_state(int_state);
        return false;
    }

    mtx->locked = true;
    
    cpu_set_int_state(int_state);
    return true;
}

void mutex_unlock(mutex_t *mtx) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    mtx->locked = false;

    struct process *unlocked_process = p_list_pop_front(&mtx->blocked_list);
    if (unlocked_process) {
        unlocked_process->state = READY;
        p_list_push_back(&process_ready_list, unlocked_process);
    }
    
    cpu_set_int_state(int_state);
}
