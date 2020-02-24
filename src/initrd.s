PUBLIC _initrd_init_start, _initrd_init_end

SECTION code_compiler

_initrd_init_start:
    BINARY "init/init.bin"
_initrd_init_end:
