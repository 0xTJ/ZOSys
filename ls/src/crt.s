EXTERN _main
EXTERN __exit

ORG 0x1000

SECTION code_crt_init

SECTION code_compiler

    call _main
    pop de
    pop de
    pop de
    push hl
    call __exit

SECTION data_compiler
SECTION bss_compiler
