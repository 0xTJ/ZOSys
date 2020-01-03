INCLUDE "config_scz180_private.inc"
EXTERN reset
EXTERN _context_init, _context_save, _context_restore
EXTERN syscall
EXTERN _trap

SECTION interrupt_table

    DEFS (0x00 - ASMPC), 0xFF   ; RST 0 / RESET
    jp reset
    DEFS (0x08 - ASMPC), 0xFF   ; RST 8 / SYSCALL
    jmp syscall
    DEFS (0x10 - ASMPC), 0xFF   ; RST 10
    ret
    DEFS (0x18 - ASMPC), 0xFF   ; RST 18
    ret
    DEFS (0x20 - ASMPC), 0xFF   ; RST 20
    ret
    DEFS (0x28 - ASMPC), 0xFF   ; RST 28
    ret
    DEFS (0x30 - ASMPC), 0xFF   ; RST 30
    ret
    DEFS (0x38 - ASMPC), 0xFF   ; RST 38 / IM1 INT0
    jmp int_0

    DEFS (0x40 - ASMPC), 0xFF
PUBLIC vector_table
vector_table:
    DEFW int_no_vector          ; INT1
    DEFW int_no_vector          ; INT2
    DEFW int_prt0               ; PRT0
    DEFW int_no_vector          ; PRT1
    DEFW int_no_vector          ; DMA0
    DEFW int_no_vector          ; DMA1
    DEFW int_no_vector          ; CSIO
    DEFW int_no_vector          ; ASCI0
    DEFW int_no_vector          ; ASCI1

    DEFS (0x66 - ASMPC), 0xFF   ; NMI
    retn


SECTION code_rom_resident

PUBLIC trap
trap:
    in0 a, (ITC)
    and 0x7F
    out0 (ITC), a
    call _trap
    

EXTERN _int_0
int_0:
    call _context_save
    call _int_0
    call _context_restore
    ei
    reti

EXTERN _int_prt0
int_prt0:
    call _context_save
    call _int_prt0
    call _context_restore
    ei
    reti

EXTERN _int_no_vector
int_no_vector:
    ei
    reti
