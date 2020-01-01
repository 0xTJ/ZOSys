INCLUDE "config_scz180_public.inc"
EXTERN _main

STACK_TAIL = 0xF000
KSTACK_TAIL = 0x10000


SECTION code_ca0
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
    jmp int0

    DEFS (0x40 - ASMPC), 0xFF
vector_table:
    DEFW no_vector              ; INT1
    DEFW no_vector              ; INT2
    DEFW int_prt0               ; PRT0
    DEFW no_vector              ; PRT1
    DEFW no_vector              ; DMA0
    DEFW no_vector              ; DMA1
    DEFW no_vector              ; CSIO
    DEFW no_vector              ; ASCI0
    DEFW no_vector              ; ASCI1

    DEFS (0x66 - ASMPC), 0xFF   ; NMI
    retn


no_vector:
    ei
    reti


SECTION code_crt_init

start:
    di
    im 1

    ; Relocate internal registers
    ld a, #__IO_BASE_ADDRESS
    out0 (ICR), a

    ; Initialize Stack Pointer
    ld hl, #KSTACK_TAIL
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

    ; Map Common Area 1 at 0x8000 to 0x80000
    ld a, #(0x80000 - 0x8000) >> 12
    out0 (CBR), a

    ; Copy 28k from 0x01000 to 0x81000
    ld hl, 0x1000
    ld de, 0x9000
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

    ; Common Area 0 to fill 0x0000-0x0FFF
    ; Bank Area to fill 0x1000-0xEFFF
    ; Common Area 1 to fill 0xF000-0xFFFF
    ld a, #0xF1
    out0 (CBAR), a

    ; Make BA and CA1 offset to 0x80000
    ld a, #0x80000 >> 12
    out0 (BBR), a
    out0 (CBR), a


SECTION code_ca0_2

    ; Setup current and saved SP values
    ld hl, KSTACK_TAIL
    ld (_context_temp_sp), hl
    ld hl, STACK_TAIL
    ld sp, hl

    ; Setup saved BBR value
    ld a, 0x80000 >> 12
    ld (_context_temp_bbr), a
    
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


EXTERN _int0

int0:
    call context_save
    call _int0
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


syscall:
    ; Load A * 2 to HL
    xor h
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

    ; Tail-call function in HL
    jp (hl)

syscall_bad:
    ld hl, 0xFFFF
    ret


PUBLIC _context_init
PUBLIC context_init

; void context_init(void (*pc)());
_context_init:
context_init:
    ; Copy pc argument to DE
    pop hl
    pop de
    push de
    push hl
    ; Swap _context_temp_sp and SP using HL
    ld hl, (_context_temp_sp)
    ld (_context_temp_sp), sp
    ld sp, hl
    ; Swap _context_temp_bbr and BBR
    ld a, (_context_temp_bbr)
    in0 b, (BBR)
    out0 (BBR), a
    ld a, b
    ld (_context_temp_bbr), a
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
    ; Swap _context_temp_bbr and BBR
    ld a, (_context_temp_bbr)
    in0 b, (BBR)
    out0 (BBR), a
    ld a, b
    ld (_context_temp_bbr), a
    ; Swap _context_temp_sp and SP using HL
    ld hl, (_context_temp_sp)
    ld (_context_temp_sp), sp
    ld sp, hl
    ; Return
    ret


PUBLIC context_save

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
    ; Swap _context_temp_bbr and BBR
    ld a, (_context_temp_bbr)
    in0 b, (BBR)
    out0 (BBR), a
    ld a, b
    ld (_context_temp_bbr), a
    ; Swap _context_temp_sp and SP using HL
    ld hl, (_context_temp_sp)
    ld (_context_temp_sp), sp
    ld sp, hl
    ; Push return address from IX
    push ix
    ; Return
    ret


PUBLIC context_restore

context_restore:
    ; Pop return address to IX
    pop ix
    ; Swap _context_temp_sp and SP using HL
    ld hl, (_context_temp_sp)
    ld (_context_temp_sp), sp
    ld sp, hl
    ; Swap _context_temp_bbr and BBR
    ld a, (_context_temp_bbr)
    in0 b, (BBR)
    out0 (BBR), a
    ld a, b
    ld (_context_temp_bbr), a
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


argv:
    DEFW 0


EXTERN _sys_0

syscall_table:
    DEFW _sys_0
    DEFS (256 - (ASMPC - syscall_table)) * 2, 0x00

SECTION code_ca1
ORG 0xE000

PUBLIC _context_temp_sp, _context_temp_bbr

_context_temp_sp:
    DEFW 0
_context_temp_bbr:
    DEFB 0


SECTION code_ba
ORG 0x1000

SECTION code_compiler
SECTION data_compiler
SECTION bss_compiler
