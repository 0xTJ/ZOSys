CC = zcc

ARCH = +scz180 -subtype=none

SRCDIR = src
DEPDIR = dep
OBJDIR = obj

SRCS_C = $(wildcard $(SRCDIR)/*.c)
SRCS_ASM = $(wildcard $(SRCDIR)/*.s)
DEPS := $(SRCS_C:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
OBJS = $(SRCS_ASM:$(SRCDIR)/%.s=$(OBJDIR)/%.o) $(SRCS_C:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

DEPFLAGS = -Cp"-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d"
CPPFLAGS += $(DEPFLAGS) -Iinclude
CFLAGS += --list -SO3 -clib=sdcc_iy --math32_z180 #--max-allocs-per-node200000
LDFLAGS += --no-crt -nostdlib -clib=sdcc_iy --math32_z180
LDLIBS += -lm

TARGET = user

.PHONY: all clean

all: $(TARGET).bin

$(TARGET).bin: $(OBJS)
	$(CC) $(ARCH) $(LDFLAGS) $^ $(LDLIBS) -o $@

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
