INCLUDE "config_scz180_private.inc"

SECTION code_ca0_2

PUBLIC _context_init

; Only to be done with interrupts disabled, while in kernel space
; void context_init(void (*pc)());
_context_init:
    ; Copy pc argument to DE
    pop hl
    pop de
    push de
    push hl
    ; Copy _context_temp_sp to HL and SP to _context_temp_sp
    ld hl, (_interrupt_sp)
    ld (_interrupt_sp), sp
    ; Use top of reserved user space as stack to prevent overwriting
    ld sp, nmi_clobberable_tail
    ; Put kernel out of address space
    ld a, 0x11
    out0 (CBAR), a
    ; Load original _context_temp_sp to SP
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
    ; Copy _context_temp_sp to HL and SP to _context_temp_sp
    ld hl, (_interrupt_sp)
    ld (_interrupt_sp), sp
    ; Use top of reserved user space as stack to prevent overwriting
    ld sp, nmi_clobberable_tail
    ; Bring kernel into address space
    ld a, 0xF1
    out0 (CBAR), a
    ; Restore original SP
    ld sp, hl
    ; Return
    ret


PUBLIC context_save

; Only to be done with interrupts disabled, while in user space
context_save:
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
    ld (_interrupt_sp), sp
    ; Use top of reserved user space as stack to prevent overwriting
    ld sp, nmi_clobberable_tail
    ; Bring kernel into address space
    ld a, 0xF1
    out0 (CBAR), a
    ; Use stack at top of kernel space
    ld sp, interrupt_stack_tail
    ; Push return address from IX
    push ix
    ; Return
    ret


PUBLIC context_restore

; Only to be done with interrupts disabled, while in kernel space
context_restore:
    ; Pop return address to IX
    pop ix
    ; Use top of reserved user space as stack to prevent overwriting
    ld sp, nmi_clobberable_tail
    ; Put kernel out of address space
    ld a, 0x11
    out0 (CBAR), a
    ; Restore stack location
    ld sp, (_interrupt_sp)
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


SECTION user_tmp

PUBLIC _interrupt_sp
_interrupt_sp:
    DEFW 0

; Used as a stack when no consistent stack will be available
; SP must always point to a valid stack in case an NMI occurs
PUBLIC nmi_clobberable, nmi_clobberable_tail
nmi_clobberable:
    DEFS 0x100
nmi_clobberable_tail:


SECTION kernel

; Interrupt stack exists at the top of this section
PUBLIC interrupt_stack_tail
interrupt_stack_tail = 0xF000
