INCLUDE "config_scz180_private.inc"
EXTERN reset
EXTERN _context_init, interrupt_enter, interupt_leave
EXTERN syscall
EXTERN _trap

SECTION interrupt_table

    DEFS (0x00 - ASMPC), 0xFF   ; RST 0 / RESET / TRAP
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
    DEFW int_1                  ; INT1
    DEFW int_2                  ; INT2
    DEFW int_prt0               ; PRT0
    DEFW int_prt1               ; PRT1
    DEFW int_dma0               ; DMA0
    DEFW int_dma1               ; DMA1
    DEFW int_csio               ; CSIO
    DEFW int_asci0              ; ASCI0
    DEFW int_asci1              ; ASCI1

    DEFS (0x66 - ASMPC), 0xFF   ; NMI
    retn


SECTION code_rom_resident

; Only to be done with interrupts disabled, while in kernel space
; void context_init(void (*pc)());
; PUBLIC _context_init
_context_init:
    ; Copy pc argument to DE
    pop hl
    pop de
    push de
    push hl
    ; Copy interrupt_sp to HL and SP to interrupt_sp
    ld hl, (interrupt_sp)
    ld (interrupt_sp), sp
    ; Put kernel out of address space
    ld a, 0x11
    out0 (CBAR), a
    ; Load original interrupt_sp to SP
    ld sp, hl
    ; Clear HL
    ld hl, 0
    ; Push return address
    push de
    ; Push IX
    push hl
    ; Push AF, AF'
    push hl
    push hl
    ; Push BC, DE, HL
    push hl
    push hl
    push hl
    ; Push BC', DE', HL'
    push hl
    push hl
    push hl
    ; Push IY
    push hl
    ; Copy interrupt_sp to HL and SP to interrupt_sp
    ld hl, (interrupt_sp)
    ld (interrupt_sp), sp
    ; Bring kernel into address space
    ld a, 0xF1
    out0 (CBAR), a
    ; Restore original SP
    ld sp, hl
    ; Return
    ret


interrupt_enter:
    ; Pop return address to IX while pushing IX
    ex (sp), ix
    ; Push AF, AF'
    push af
    ex af,af'
    push af
    ; Push BC, DE, HL
    push bc
    push de
    push hl
    ; Push BC', DE', HL'
    exx
    push bc
    push de
    push hl
    ; Push IY
    push iy
    ; Save SP
    ld (interrupt_sp), sp
    ; Use stack in user temporary
    ld sp, interrupt_stack_tail
    ; Save CBAR and load with kernel CBAR
    in0 a, (CBAR)
    ld (interrupt_cbar), a
    ld a, 0xF1
    out0 (CBAR), a
    ; Push return address from IX
    push ix
    ; Return
    ret


interupt_leave:
    ; Pop return address to IX
    pop ix
    ; Restore CBAR
    ld a, (interrupt_cbar)
    out0 (CBAR), a
    ; Restore stack location
    ld sp, (interrupt_sp)
    ; Pop IY
    pop iy
    ; Pop HL', DE', BC'
    pop hl
    pop de
    pop bc
    exx
    ; Pop HL, DE, BC
    pop hl
    pop de
    pop bc
    ; Pop AF', AF
    pop af
    ex af,af'
    pop af
    ; Push return address while popping to IX
    ex (sp),ix
    ; Return
    ret


; TODO: Get UFO ASAP, before another process can TRAP and change it
PUBLIC trap
trap:
    in0 c, (ITC)
    ld b, 0
    pop de
    ld hl, sp
    push de
    ld sp, trap_stack_tail
    push hl ; Push TRAP SP
    push bc ; Push ITC
    push de ; Push TRAP PC
    in0 l, (CBAR)
    ld h, 0
    push hl ; Push CBAR
    ld a, 0xF1
    out0 (CBAR), a
    call _trap
trap_loop:
    jmp trap_loop

EXTERN _int_0
int_0:
    call interrupt_enter
    call _int_0
    call interupt_leave
    ei
    reti

EXTERN _int_1
int_1:
    call interrupt_enter
    call _int_1
    call interupt_leave
    ei
    reti

EXTERN _int_2
int_2:
    call interrupt_enter
    call _int_2
    call interupt_leave
    ei
    reti

EXTERN _int_prt0
int_prt0:
    call interrupt_enter
    call _int_prt0
    call interupt_leave
    ei
    reti

EXTERN _int_prt1
int_prt1:
    call interrupt_enter
    call _int_prt1
    call interupt_leave
    ei
    reti

EXTERN _int_dma0
int_dma0:
    call interrupt_enter
    call _int_dma0
    call interupt_leave
    ei
    reti

EXTERN _int_dma1
int_dma1:
    call interrupt_enter
    call _int_dma1
    call interupt_leave
    ei
    reti

EXTERN _int_csio
int_csio:
    call interrupt_enter
    call _int_csio
    call interupt_leave
    ei
    reti

EXTERN _int_asci0
int_asci0:
    call interrupt_enter
    call _int_asci0
    call interupt_leave
    ei
    reti

EXTERN _int_asci1
int_asci1:
    call interrupt_enter
    call _int_asci1
    call interupt_leave
    ei
    reti

EXTERN _int_no_vector
int_no_vector:
    call interrupt_enter
    call _int_no_vector
    call interupt_leave
    ei
    reti
    

SECTION user_tmp

interrupt_sp:
    DEFW 0

interrupt_cbar:
    DEFB 0

interrupt_stack:
    DEFS 0x400
interrupt_stack_tail:

trap_stack:
    DEFS 0x100
trap_stack_tail:
