PREFIX ?= i686-elf-
ARCH ?= i386
CFLAGS ?= -O0 -g

CC := $(PREFIX)gcc
CXX := $(PREFIX)g++
AS := $(PREFIX)as
AR := $(PREFIX)ar

ARCH_DIR := arch/$(ARCH)

CFLAGS := $(CFLAGS) -ffreestanding \
	-Wall -Wextra -Werror \
	-Wno-sign-compare -Wno-missing-field-initializers

CPPFLAGS:=$(CPPFLAGS) -DLIBC_IS_LIBK \
	-I$(ARCH_DIR)/include \
	-Ilibc/include \
	-Ilibcxx/include \
	-I.

LIBS := -nostdlib -lgcc

SRC_DIRS = $(ARCH_DIR) core libc libcxx
OBJS = \
	$(patsubst %.S,%.o,$(shell find $(SRC_DIRS) -name '*.S')) \
	$(patsubst %.c,%.o,$(shell find $(SRC_DIRS) -name '*.c')) \
	$(patsubst %.cc,%.o,$(shell find $(SRC_DIRS) -name '*.cc')) \

CRT_DIR = arch/crt/$(ARCH)
CRTI_OBJ = $(patsubst %.S,%.o,$(shell find $(CRT_DIR) -name 'crti.S'))
CRTN_OBJ = $(patsubst %.S,%.o,$(shell find $(CRT_DIR) -name 'crtn.S'))

.SUFFIXES: .o .c .cc .S
.c.o:
	$(CC) -MD -c $< -o $@ -std=c11 $(CFLAGS) $(CPPFLAGS)

.cc.o:
	$(CXX) -MD -c $< -o $@ -std=c++17 $(CFLAGS) $(CPPFLAGS) \
		-fno-exceptions -fno-rtti

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

all: out/kernel.iso

LINK_LIST = \
	$(CRTI_OBJ) \
	$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o) \
	$(OBJS) \
	$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o) \
	$(CRTN_OBJ) \
	$(LIBS)

out/kernel.bin: $(OBJS) $(CRTI_OBJ) $(CRTN_OBJ) $(ARCH_DIR)/linker.ld
	$(CXX) -T $(ARCH_DIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)

out/kernel.iso: out/kernel.bin boot/grub.cfg
	mkdir -p out/isodir/boot/grub
	cp out/kernel.bin out/isodir/boot/kernel.bin
	cp boot/grub.cfg out/isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ out/isodir

.PHONY: clean
clean:
	rm -rf out/*
	rm -f $(OBJS)
	rm -f $(OBJS:.o=.d)

.PHONY: bochs
bochs: out/kernel.iso
	bochs

.PHONY: qemu
qemu: out/kernel.iso
	qemu-system-i386 -cdrom out/kernel.iso

-include $(OBJS:.o=.d)
