CC = zcc

ARCH = +scz180 -subtype=none

SRCDIR = src
DEPDIR = dep
OBJDIR = obj
INITDIR = init

SRCS_C = $(wildcard $(SRCDIR)/*.c)
SRCS_ASM = $(SRCDIR)/start.s $(filter-out $(SRCDIR)/start.s,$(wildcard $(SRCDIR)/*.s))
DEPS := $(SRCS_C:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
OBJS = $(SRCS_ASM:$(SRCDIR)/%.s=$(OBJDIR)/%.o) $(SRCS_C:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

DEPFLAGS = -Cp"-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d"
CPPFLAGS += $(DEPFLAGS) -Iinclude
CFLAGS += --list -SO3 -clib=sdcc_iy --math32_z180 #--max-allocs-per-node200000
LDFLAGS += --no-crt -nostdlib -clib=sdcc_iy --math32_z180
LDLIBS += -lm

TARGET = zosys

.PHONY: all clean init

all: init $(TARGET).bin

$(TARGET).bin: $(OBJS)
	$(CC) $(ARCH) $(LDFLAGS) $^ $(LDLIBS) -o $@
	dd if=$(TARGET)_rom_resident.bin of=$@ bs=1 seek=0
	dd if=$(TARGET)_kernel.bin of=$@ bs=1 seek=4096
	dd if=/dev/zero of=$@ bs=1 count=1 seek=32767

$(OBJDIR)/init.o: $(SRCDIR)/init.s $(INITDIR)/init.bin
	$(CC) $(ARCH) $(CFLAGS) -c $< -o $@

init:
	$(MAKE) -C $(INITDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d | $(DEPDIR) $(OBJDIR)
	$(CC) $(ARCH) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	$(CC) $(ARCH) $(CFLAGS) -c $< -o $@

$(OBJDIR): ; @mkdir -p $@
$(DEPDIR): ; @mkdir -p $@

$(DEPS):

include $(wildcard $(DEPS))

clean:
	$(RM) *.bin *.def $(DEPDIR)/* $(OBJDIR)/*
	$(MAKE) -C $(INITDIR) clean
