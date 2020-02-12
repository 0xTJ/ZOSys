# ZOSys

ZOSys is a Unix-inspired operating system written for the [SC126](https://smallcomputercentral.wordpress.com/sc126-z180-motherboard-rc2014/) board by [Stephen C Cousins](https://www.tindie.com/stores/tindiescx/).

## Design

ZOSys uses the Z180 MMU heavily. On boot, the system is loaded from ROM to RAM, and execution continues there.

The kernel exists in the first full memory space, pages 0x80-0x8F. Each process receives a block of 16 pages.

The lowest page in memory remains mapped to page 0x00. All vector behaviour is fixed at runtime, and writes to this area are ignored.

The highest page in memory is always mapped to the highest page in the process's page block. This may not be used by user code. It is used for syscall and interrupt stacks, and for kernel temporary data.

The pages between the lowest and the highest are either mapped to kernel space or to process space, depending what code is running.

While running kernel code, the MMU is configured as follows:

```
0x0000+---------+
      | Vectors |
      |(0x00000)|
0x1000+---------+
      |         |
      | Kernel  |
      |(0x81000)|
      |         |
      |         |
0xF000+---------+
      |Temporary|
      |(0xXF000)|
      +---------+
```

When running user code, the MMU is configured as follows:

```
0x0000+---------+
      | Vectors |
      |(0x00000)|
0x1000+---------+
      |         |
      |Userspace|
      |(0xX1000)|
      |         |
      |         |
0xF000+- - - - -+
      |Temporary|
      |(0xXF000)|
      +---------+
```

## Building

Requires [z88dk](https://github.com/z88dk/z88dk).

Run `make` in the top-level directory to build.
