#include "mutex.h"

void mutex_lock(mutex_t *mtx) __z88dk_fastcall __naked {
    __asm__("\
lock_wait_loop: \n\
    ld a, 0xFF \n\
    tst (hl) \n\
    jr nz, lock_wait_loop \n\
    inc (hl) \n\
    ld a, (hl) \n\
    cp 0x01 \n\
    jr z, lock_locked \n\
    dec (hl) \n\
    jr lock_wait_loop \n\
lock_locked:\n\
    ret \n\
    ");
    (void) mtx;
}

void mutex_unlock(mutex_t *mtx) __z88dk_fastcall __naked {
    __asm__("\
    dec (hl) \n\
    ret \n\
    ");
    (void) mtx;
}
