PREFIX ?= i686-elf-

CC := $(PREFIX)gcc
CFLAGS ?= -O0 -g

CFLAGS := $(CFLAGS) -std=c11 -ffreestanding -Wall -Wextra -Werror
LIBS := -nostdlib -lgcc

SRCS = $(shell find src -name '*.c')
ASMS = $(shell find boot -name '*.s')
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

out/kernel.bin: $(OBJS) linker.ld
	$(CC) -T linker.ld -o $@ $(CFLAGS) $(LINK_LIST)

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
