PREFIX ?= i686-elf-
ARCH ?= i386
CFLAGS ?= -O0 -g

CC := $(PREFIX)gcc
AS := $(PREFIX)as
AR := $(PREFIX)ar

CFLAGS := $(CFLAGS) -std=c11 -ffreestanding -Wall -Wextra -Werror
CPPFLAGS:=$(CPPFLAGS) -I. -Ilibc/include -DLIBC_IS_LIBK
LIBS := -nostdlib -lgcc

ARCH_DIR := arch/$(ARCH)

SRC_DIRS = $(ARCH_DIR) core
SRCS = $(shell find $(DIRS) -name '*.c')
ASMS = $(shell find $(DIRS) -name '*.s')
OBJS = $(subst .c,.o,$(SRCS)) $(subst .s,.o,$(ASMS))

.SUFFIXES: .o .c .s
.c.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)
 
.s.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

all: out/kernel.iso

LINK_LIST = \
	$(OBJS) \
	$(LIBS)

out/kernel.bin: $(OBJS) $(ARCH_DIR)/linker.ld
	$(CC) -T $(ARCH_DIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)

out/kernel.iso: out/kernel.bin boot/grub.cfg
	mkdir -p out/isodir/boot/grub
	cp out/kernel.bin out/isodir/boot/kernel.bin
	cp boot/grub.cfg out/isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ out/isodir

clean:
	rm -rf out/*
	rm -f $(OBJS)
	rm -f $(OBJS:.o=.d)

run: out/kernel.iso
	bochs

-include $(OBJS:.o=.d)
