EXTERN _init

ORG 0x1000

SECTION code_crt_init

SECTION code_compiler

jmp _init

SECTION data_compiler
SECTION bss_compiler
