CC = zcc

ARCH = +scz180 -subtype=none

SRCDIR = src
DEPDIR = dep
OBJDIR = obj
INITDIR = init
SHDIR = sh

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

.PHONY: all clean init sh

all: init sh $(TARGET).bin

$(TARGET).bin: $(OBJS)
	$(CC) $(ARCH) $(LDFLAGS) $^ $(LDLIBS) -o $@
	dd if=/dev/zero of=$@ bs=1 count=32768
	dd if=$(TARGET)_rom_resident.bin of=$@ bs=1 seek=0
	dd if=$(TARGET)_kernel.bin of=$@ bs=1 seek=4096

init:
	$(MAKE) -C $(INITDIR)

sh:
	$(MAKE) -C $(SHDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d | $(DEPDIR) $(OBJDIR)
	$(CC) $(ARCH) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.s | $(OBJDIR)
	$(CC) $(ARCH) $(CFLAGS) -c $< -o $@

$(OBJDIR): ; @mkdir -p $@
$(DEPDIR): ; @mkdir -p $@

$(DEPS):

include $(wildcard $(DEPS))

clean:
	$(RM) *.bin *.def $(DEPDIR)/* $(OBJDIR)/*
	$(MAKE) -C $(INITDIR) clean
