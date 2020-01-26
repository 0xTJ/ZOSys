CC = zcc

ARCH = +scz180 -subtype=none

SRCDIR = src
DEPDIR = dep
OBJDIR = obj
USERDIR = user

SRCS_C = $(wildcard $(SRCDIR)/*.c)
SRCS_ASM = $(SRCDIR)/start.s $(filter-out $(SRCDIR)/start.s,$(wildcard $(SRCDIR)/*.s))
DEPS := $(SRCS_C:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
OBJS = $(SRCS_ASM:$(SRCDIR)/%.s=$(OBJDIR)/%.o) $(SRCS_C:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

USER_SRCS_C = $(wildcard $(USERDIR)/$(SRCDIR)/*.c)
USER_SRCS_ASM = $(wildcard $(USERDIR)/$(SRCDIR)/*.s)
USER_DEPS := $(USER_SRCS_C:$(USERDIR)/$(SRCDIR)/%.c=$(USERDIR)/$(DEPDIR)/%.d)
USER_OBJS = $(USER_SRCS_ASM:$(USERDIR)/$(SRCDIR)/%.s=$(USERDIR)/$(OBJDIR)/%.o) $(USER_SRCS_C:$(USERDIR)/$(SRCDIR)/%.c=$(USERDIR)/$(OBJDIR)/%.o)

DEPFLAGS = -Cp"-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d"
CPPFLAGS += $(DEPFLAGS) -Iinclude
CFLAGS += --list -SO3 -clib=sdcc_iy --math32_z180 #--max-allocs-per-node200000
LDFLAGS += --no-crt -nostdlib -clib=sdcc_iy --math32_z180
LDLIBS += -lm

TARGET = zosys

.PHONY: all clean user

all: $(TARGET).bin

$(TARGET).bin: $(OBJS) $(USER_OBJS) | user
	$(CC) $(ARCH) $(LDFLAGS) $^ $(LDLIBS) -o $@
	dd if=$(TARGET)_rom_resident.bin of=$@ bs=1 seek=0
	dd if=$(TARGET)_kernel.bin of=$@ bs=1 seek=4096
	dd if=/dev/zero of=$@ bs=1 count=1 seek=32767

user $(USER_OBJS):
	$(MAKE) -C $(USERDIR)

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
	$(MAKE) -C $(USERDIR) clean
