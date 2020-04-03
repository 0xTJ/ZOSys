INCLUDE "config_scz180_private.inc"
PUBLIC _syscall_leave
EXTERN kernel_stack_tail

MAX_SYSCALL_BYTES = 12

SECTION code_rom_resident

PUBLIC syscall
syscall:
    ; Stack on entry:
    ; | [Arguments] |
    ; +-------------+
    ; |             |
    ; +   Ignored   +
    ; |             |
    ; +-------------+
    ; |   Return    |
    ; +   Address   +
    ; |             |
    ; +-------------+

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
    ld (_syscall_sp), sp
    ; Save HL to stack
    push hl
    ; Load HL with base of function arguments
    ld hl, (_syscall_sp)
    inc hl
    inc hl
    inc hl
    inc hl
    ; Load DE with base of copied function arguments
    ld de, kernel_stack_tail - MAX_SYSCALL_BYTES
    ; Copy arguments to syscall stack
    ld bc, MAX_SYSCALL_BYTES
    ldir
    ; Pop HL from stack
    pop hl
    ; Load SP with base of copied function arguments
    ld sp, kernel_stack_tail - MAX_SYSCALL_BYTES
    ; Switch to kernel space
    ld a, 0xF1
    out0 (CBAR), a
    ; Call function in HL
    ld de, _syscall_leave
    push de
    jp (hl)
_syscall_leave:
    ; Switch to user space
    ld a, 0x11
    out0 (CBAR), a
    ; Restore SP
    ld sp, (_syscall_sp)
    ; Return
    ret
syscall_bad:
    ld hl, 0xFFFF
    ret


EXTERN _sys_fork
EXTERN _sys_waitpid
EXTERN _sys_exit
EXTERN _sys_open
EXTERN _sys_close
EXTERN _sys_read
EXTERN _sys_write
EXTERN _sys_execve
EXTERN _sys_chdir
EXTERN _sys_fchdir
syscall_table:
    DEFW _sys_fork
    DEFW _sys_waitpid
    DEFW _sys_exit
    DEFW _sys_open
    DEFW _sys_close
    DEFW _sys_read
    DEFW _sys_write
    DEFW _sys_execve
    DEFW _sys_chdir
    DEFW _sys_fchdir
    DEFS (0x100 - (ASMPC - syscall_table)) * 2, 0x00
    

SECTION user_tmp

PUBLIC _syscall_sp
_syscall_sp:
    DEFW 0

PUBLIC _user_buffer
_user_buffer:
    DEFS 0x100
