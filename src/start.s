INCLUDE "config_scz180_public.inc"
EXTERN _main
EXTERN _context_init, context_save, context_restore
EXTERN interrupt_stack_tail
EXTERN syscall


SECTION rom_resident
ORG 0x0000

    DEFS (0x00 - ASMPC), 0xFF   ; RST 0 / RESET
    jp start
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


SECTION code_crt_init

start:
    di
    im 1

    ; Relocate internal registers
    ld a, #__IO_BASE_ADDRESS
    out0 (ICR), a

    ; Initialize Stack Pointer
    ld hl, #0x10000
    ld sp, hl

    ; Load interrupt vector table location
IF vector_table & 0x1F
    ERROR "Vector table not aligned at 0x20 boundary"
ENDIF
    ld a, #(vector_table & 0xE0)
    out0 (IL), a
    ld a, #((vector_table >> 8) & 0xFF)
    ld i, a

    ; Common Area 0 to fill 0x0000-0x0FFF
    ; Bank Area to fill 0x1000-0x7FFF
    ; Common Area 1 to fill 0x8000-0xFFFF
    ld a, #0x81
    out0 (CBAR), a

    ; Map Bank Area at 0x1000 to 0x01000
    ; Is redundant
    ; ld a, #(0x01000 - 0x1000) >> 12
    ; out0 (BBR), a

    ; Map Common Area 1 at 0x8000 to 0x81000
    ld a, #(0x81000 - 0x8000) >> 12
    out0 (CBR), a

    ; Copy 28k from 0x01000 to 0x81000
    ld hl, 0x1000
    ld de, 0x8000
    ld bc, 0x7000
    ldir

    ; Map Bank Area at 0x1000 to 0x08000
    ld a, #(0x08000 - 0x1000) >> 12
    out0 (BBR), a

    ; Map Common Area 1 at 0x8000 to 0x88000
    ld a, #(0x88000 - 0x8000) >> 12
    out0 (CBR), a

    ; Copy 28k from 0x08000 to 0x88000
    ld hl, 0x1000
    ld de, 0x8000
    ld bc, 0x7000
    ldir

    ; Make BA and CA1 offset to 0x80000
    ld a, #0x80000 >> 12
    out0 (BBR), a
    out0 (CBR), a

    ; Common Area 0 to fill 0x0000-0x0FFF
    ; Common Area 1 to fill 0x1000-0xFFFF
    ld a, #0x11
    out0 (CBAR), a


SECTION code_ca0_2

    ; Setup current SP value
    ld sp, interrupt_stack_tail
    
    ; Enable Interrupts
    ei

    ; Call main:
    ; char *args[1] = { NULL };
    ; io_led_output = main(0, &args);
    ld de, argv
    push de
    ld de, #0
    push de
    call _main
    pop de
    pop de
    out0 (__IO_LED_OUTPUT), l

    ; Halt forever
halt_loop:
    halt
    jr halt_loop


EXTERN _int_no_vector
int_no_vector:
    ei
    reti


EXTERN _int_0
int_0:
    call context_save
    call _int_0
    call context_restore
    ei
    reti


EXTERN _int_prt0
int_prt0:
    call context_save
    call _int_prt0
    call context_restore
    ei
    reti


argv:
    DEFW 0


SECTION user_tmp
ORG 0xF000

SECTION kernel
ORG 0x1000
SECTION code_compiler
SECTION data_compiler
SECTION bss_compiler
