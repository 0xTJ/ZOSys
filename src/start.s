INCLUDE "config_scz180_public.inc"
PUBLIC _halt
EXTERN vector_table, trap
EXTERN _main
EXTERN asm_heap_init

HEAP_BLOCK_SIZE = 0x1000

SECTION rom_resident
ORG 0x0000

SECTION interrupt_table

SECTION code_crt_init

PUBLIC reset
reset:
    ; Relocate internal registers
    ld a, __IO_BASE_ADDRESS
    out0 (ICR), a

    ; Jump to trap if in trap handler
    in0 a, (ITC)
    tst 0x80
    jp nz, trap

    ; Initialize Stack Pointer
    ; Until the proper stack is setup, the top of memory is used as stack
    ; There must always be a valid stack to allow for NMI
    ld sp, 0x10000

    im 1

    ; Load interrupt vector table location
IF vector_table & 0x1F
    ERROR "Vector table not aligned at 0x20 boundary"
ENDIF
    ld a, vector_table & 0xE0
    out0 (IL), a
    ld a, [vector_table >> 8] & 0xFF
    ld i, a

    ; Common Area 0 to fill 0x0000-0x0FFF
    ; Bank Area to fill 0x1000-0x7FFF
    ; Common Area 1 to fill 0x8000-0xFFFF
    ld a, 0x81
    out0 (CBAR), a

    ; Map Bank Area at 0x1000 to 0x01000
    ld a, #(0x01000 - 0x1000) >> 12
    out0 (BBR), a

    ; Map Common Area 1 at 0x8000 to 0x81000
    ld a, [0x81000 - 0x8000] >> 12
    out0 (CBR), a

    ; Copy 28k from 0x01000 to 0x81000
    ld hl, 0x1000
    ld de, 0x8000
    ld bc, 0x7000
    ldir

    ; Map Bank Area at 0x1000 to 0x08000
    ld a, [0x08000 - 0x1000] >> 12
    out0 (BBR), a

    ; Map Common Area 1 at 0x8000 to 0x88000
    ld a, [0x88000 - 0x8000] >> 12
    out0 (CBR), a

    ; Copy 28k from 0x08000 to 0x88000
    ld hl, 0x1000
    ld de, 0x8000
    ld bc, 0x7000
    ldir

    ; Make BA and CA1 offset to 0x80000
    ld a, 0x80000 >> 12
    out0 (BBR), a
    out0 (CBR), a

    ; Common Area 0 to fill 0x0000-0x0FFF
    ; Bank Area to fill 0x1000-0xEFFF
    ; Common Area 1 to fill 0xF000-0xFFFF
    ld a, 0xF1
    out0 (CBAR), a

    ; Initialize heap
    ld bc, HEAP_BLOCK_SIZE
    ld hl, (__malloc_heap)
    call asm_heap_init


SECTION code_rom_resident

    ; Setup current SP value
    ld sp, kernel_stack_tail
    
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
_halt:
    di
halt_loop:
    halt
    jr halt_loop


argv:
    DEFW 0


SECTION user_tmp
ORG 0xF000

PUBLIC kernel_stack, kernel_stack_tail
kernel_stack:
    DEFS 0x400
kernel_stack_tail:

SECTION kernel
ORG 0x1000

PUBLIC __malloc_heap
__malloc_block:
    DEFS HEAP_BLOCK_SIZE
__malloc_heap:
    DEFW __malloc_block

stack_tail = 0xF000

SECTION code_compiler
SECTION data_compiler
SECTION bss_compiler
