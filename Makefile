CC = zcc

CFLAGS = +scz180 -subtype=none -Iinclude --list --no-crt -nostdlib -SO3 -clib=sdcc_iy --math32_z180 --max-allocs-per-node200000
LDFLAGS = +scz180 -subtype=none -Iinclude --no-crt -nostdlib -SO3 -clib=sdcc_iy --math32_z180 --max-allocs-per-node200000

ASM_SRCS = src/start.s src/interrupt.s src/context.s src/syscall.s
C_SRCS = src/main.c src/process.c src/asci.c src/dma.c src/prt.c src/mutex.c

OBJS = $(ASM_SRCS:.s=.o) $(C_SRCS:.c=.o)

TARGET = zosys

all: $(TARGET).bin

$(TARGET).bin: $(OBJS)
	$(CC) $(LDFLAGS) $(INCLUDES) -o $@ $(OBJS) $(LFLAGS) $(LIBS)
	dd if=$(TARGET)_rom_resident.bin of=$@ bs=1 seek=0
	dd if=$(TARGET)_kernel.bin of=$@ bs=1 seek=4096
	dd if=/dev/zero of=$@ bs=1 count=1 seek=32767

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.s.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	$(RM) *.bin *.o *.map *.com *.def $(OBJS)
