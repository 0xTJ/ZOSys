PUBLIC _initrd_init_start, _initrd_init_end
PUBLIC _initrd_sh_start, _initrd_sh_end

SECTION code_compiler

_initrd_init_start:
    BINARY "init/init.bin"
_initrd_init_end:

_initrd_sh_start:
    BINARY "init/init.bin"
_initrd_sh_end:
