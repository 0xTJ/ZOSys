PUBLIC _initrd_init_start, _initrd_init_end
PUBLIC _initrd_sh_start, _initrd_sh_end
PUBLIC _initrd_ls_start, _initrd_ls_end
PUBLIC _initrd_dd_start, _initrd_dd_end

SECTION code_compiler

_initrd_init_start:
    BINARY "init/init.bin"
_initrd_init_end:

_initrd_sh_start:
    BINARY "sh/sh.bin"
_initrd_sh_end:

_initrd_ls_start:
    BINARY "ls/ls.bin"
_initrd_ls_end:

_initrd_dd_start:
    BINARY "dd/dd.bin"
_initrd_dd_end:
