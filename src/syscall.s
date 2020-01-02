INCLUDE "config_scz180_private.inc"

SECTION code_ca0_2

PUBLIC syscall
syscall:
    ; Load A * 2 to HL
    ld h, 0
    sla a
    rl h
    ld l, a

    ; Add syscall_table to HL
    add hl, syscall_table

    ; Load function address to HL
    ld e, (hl)
    inc hl
    ld d, (hl)
    ld hl, de

    ; Return with error if HL is 0x0000
    ld a, 0
    or l
    or h
    jp z, syscall_bad

    ; Save SP and load
    ld (syscall_sp), sp
    ld sp, syscall_stack_tail

    ; Switch to kernel space
    ld a, 0xF1
    out0 (CBAR), a

    ; Call function in HL
    ld de, syscall_ret
    push de
    jp (hl)
syscall_ret:

    ; Switch to user space
    ld a, 0x11
    out0 (CBAR), a

    ; Restore SP
    ld sp, (syscall_sp)

    ; Return
    ret

syscall_bad:
    ld hl, 0xFFFF
    ret


EXTERN _sys_0
EXTERN _sys_1
syscall_table:
    DEFW _sys_0
    DEFW _sys_1
    DEFS (256 - (ASMPC - syscall_table)) * 2, 0x00
    

SECTION user_tmp

syscall_sp:
    DEFW 0

syscall_stack:
    DEFS 0x100
syscall_stack_tail:
