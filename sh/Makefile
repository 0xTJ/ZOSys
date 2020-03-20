CC = zcc

ARCH = +scz180 -subtype=none

SRCDIR = src
DEPDIR = dep
OBJDIR = obj

SRCS_C = $(wildcard $(SRCDIR)/*.c)
SRCS_ASM = $(SRCDIR)/crt.s $(filter-out $(SRCDIR)/crt.s,$(wildcard $(SRCDIR)/*.s))
DEPS := $(SRCS_C:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
OBJS = $(SRCS_ASM:$(SRCDIR)/%.s=$(OBJDIR)/%.o) $(SRCS_C:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

DEPFLAGS = -Cp"-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d"
CPPFLAGS += $(DEPFLAGS) -Iinclude
CFLAGS += --list -SO3 -clib=sdcc_iy -isystem../clib/include #--max-allocs-per-node200000
LDFLAGS += --no-crt -nostdlib -clib=sdcc_iy -L../clib -lclib

TARGET = sh

.PHONY: all clean

all: $(TARGET).bin

$(TARGET).bin: $(OBJS)
	$(CC) $(ARCH) $(LDFLAGS) $^ $(LDLIBS) -o $@
	$(RM) ../$(OBJDIR)/initrd.o

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
