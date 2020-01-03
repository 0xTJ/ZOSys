INCLUDE "config_scz180_private.inc"

MAX_SYSCALL_BYTES = 12

SECTION code_rom_resident

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
    ; Save SP
    ld (syscall_sp), sp
    ; Save HL to stack
    push hl
    ; Load HL with base of function arguments
    ld hl, (syscall_sp)
    inc hl
    inc hl
    ; Load DE with base of copied function arguments
    ld de, syscall_stack_tail - MAX_SYSCALL_BYTES
    ; Copy arguments to syscall stack
    ld bc, MAX_SYSCALL_BYTES
    ldir
    ; Pop HL from stack
    pop hl
    ; Load SP with base of copied function arguments
    ld sp, syscall_stack_tail - MAX_SYSCALL_BYTES
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

EXTERN _sys_fork
EXTERN _sys_1
syscall_table:
    DEFW _sys_fork
    DEFW _sys_1
    DEFS (256 - (ASMPC - syscall_table)) * 2, 0x00
    

SECTION user_tmp


syscall_sp:
    DEFW 0
PUBLIC syscall_stack, syscall_stack_tail
syscall_stack:
    DEFS 0x100
syscall_stack_tail:
