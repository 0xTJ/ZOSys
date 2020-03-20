CC = zcc

ARCH = +scz180 -subtype=none

SRCDIR = src
DEPDIR = dep
OBJDIR = obj
CLIBDIR = clib
INITDIR = init
SHDIR = sh

SRCS_C = $(wildcard $(SRCDIR)/*.c)
SRCS_ASM = $(SRCDIR)/start.s $(filter-out $(SRCDIR)/start.s,$(wildcard $(SRCDIR)/*.s))
DEPS := $(SRCS_C:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
OBJS = $(SRCS_ASM:$(SRCDIR)/%.s=$(OBJDIR)/%.o) $(SRCS_C:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

DEPFLAGS = -Cp"-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d"
CPPFLAGS += $(DEPFLAGS) -Iinclude
CFLAGS += --list -SO3 -clib=sdcc_iy #--max-allocs-per-node200000
LDFLAGS += --no-crt -nostdlib -clib=sdcc_iy

TARGET = zosys

.PHONY: all clean clib init sh

all: clib init sh $(TARGET).bin

$(TARGET).bin: $(OBJS)
	$(CC) $(ARCH) $(LDFLAGS) $^ $(LDLIBS) -o $@
	dd if=/dev/zero of=$@ bs=1 count=32768
	dd if=$(TARGET)_rom_resident.bin of=$@ bs=1 seek=0
	dd if=$(TARGET)_kernel.bin of=$@ bs=1 seek=4096

clib:
	$(MAKE) -C $(CLIBDIR)

init: clib
	$(MAKE) -C $(INITDIR)

sh: clib
	$(MAKE) -C $(SHDIR)

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
