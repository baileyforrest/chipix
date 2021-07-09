all: out/kernel.iso

out/boot.o: boot/boot.s
	i686-elf-as boot/boot.s -o out/boot.o

out/kernel.o: src/kernel.c
	i686-elf-gcc -c src/kernel.c -o out/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

out/kernel.bin: out/boot.o out/kernel.o linker.ld
	i686-elf-gcc -T linker.ld -o out/kernel.bin -ffreestanding -O2 -nostdlib out/boot.o out/kernel.o -lgcc

out/kernel.iso: out/kernel.bin boot/grub.cfg
	mkdir -p out/isodir/boot/grub
	cp out/kernel.bin out/isodir/boot/kernel.bin
	cp boot/grub.cfg out/isodir/boot/grub/grub.cfg
	grub-mkrescue -o out/kernel.iso out/isodir

clean:
	rm -rf out/*

run: out/kernel.iso
	bochs
