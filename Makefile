CC = zcc

ARCH = +scz180 -subtype=none

SRCDIR = src
DEPDIR = dep
OBJDIR = obj
CLIBDIR = clib
INITDIR = init
SHDIR = sh
LSDIR = ls
DDDIR = dd

SRCS_C = $(wildcard $(SRCDIR)/*.c)
SRCS_ASM = $(SRCDIR)/start.s $(filter-out $(SRCDIR)/start.s,$(wildcard $(SRCDIR)/*.s))
DEPS := $(SRCS_C:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
OBJS = $(SRCS_ASM:$(SRCDIR)/%.s=$(OBJDIR)/%.o) $(SRCS_C:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# DEPFLAGS = -Cp"-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d"
CPPFLAGS += $(DEPFLAGS) -Iinclude
CFLAGS += --list -SO3 -clib=sdcc_iy -nostdlib --no-crt -isystemclib/include -s -g -m #--max-allocs-per-node200000
LDFLAGS += --no-crt -nostdlib -clib=sdcc_iy

TARGET = zosys

.PHONY: all clean clib init sh ls dd

all: clib init sh ls dd $(TARGET).bin

$(TARGET).bin: $(SRCS_ASM) $(SRCS_C)
	$(CC) $(ARCH) -s -g -m $(CPPFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@
	dd if=/dev/zero of=$@ bs=1 count=65536
	dd if=$(TARGET)_rom_resident.bin of=$@ bs=1 seek=0
	dd if=$(TARGET)_kernel.bin of=$@ bs=1 seek=4096

# $(TARGET).bin: $(OBJS)
# 	$(CC) $(ARCH) -s -g -m $(LDFLAGS) $^ $(LDLIBS) -o $@ -bn $@
# 	dd if=/dev/zero of=$@ bs=1 count=65536
# 	dd if=$(TARGET)_rom_resident.bin of=$@ bs=1 seek=0
# 	dd if=$(TARGET)_kernel.bin of=$@ bs=1 seek=4096

clib:
	$(MAKE) -C $(CLIBDIR)

init: clib
	$(MAKE) -C $(INITDIR)

sh: clib
	$(MAKE) -C $(SHDIR)

ls: clib
	$(MAKE) -C $(LSDIR)

dd: clib
	$(MAKE) -C $(DDDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@) $(dir $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d))
	$(CC) $(ARCH) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	mkdir -p $(dir $@)
	$(CC) $(ARCH) $(CFLAGS) -c $< -o $@

$(DEPS):

include $(wildcard $(DEPS))

clean:
	$(RM) *.lib *.bin *.def $(DEPDIR) $(OBJDIR) -r
	$(MAKE) -C $(CLIBDIR) clean
	$(MAKE) -C $(INITDIR) clean
	$(MAKE) -C $(SHDIR) clean
	$(MAKE) -C $(LSDIR) clean
	$(MAKE) -C $(DDDIR) clean
