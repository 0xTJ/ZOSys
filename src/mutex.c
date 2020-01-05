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

int mutex_trylock(mutex_t *mtx) __z88dk_fastcall __naked {
    __asm__("\
    ld a, 0xFF \n\
    tst (hl) \n\
    jr nz, trylock_failed \n\
    inc (hl) \n\
    ld a, (hl) \n\
    cp 0x01 \n\
    jr z, trylock_locked \n\
    dec (hl) \n\
    jr trylock_failed \n\
trylock_locked: \n\
    ld hl, 0 \n\
    ret \n\
trylock_failed: \n\
    ld hl, 0xFFFF \n\
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
