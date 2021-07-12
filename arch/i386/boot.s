// Declare constants for the multiboot header.

// Align loaded modules on page boundaries.
.set ALIGN, 1 << 0

// Provide memory map.
.set MEMINFO, 1 << 1

// This is the Multiboot 'flag' field.
.set FLAGS, ALIGN | MEMINFO

// 'magic number' lets bootloader find the header.
.set MAGIC, 0x1badb002

// Checksum of above, to prove we are multiboot.
.set CHECKSUM, -(MAGIC + FLAGS)

// Declare a multiboot header that marks the program as a kernel.
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

// Allocate stack. Must be 16B aligned.
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 16 * 1024
stack_top:

// Preallocate pages used for paging. Don't hard-code addresses and assume they
// are available, as the bootloader might have loaded its multiboot structures
// or modules there. This lets the bootloader know it must avoid the addresses.
.section .bss, "aw", @nobits
.align 4096
boot_page_directory:
.skip 4096
boot_page_table1:
.skip 4096
// Further page tables may be required if the kernel grows beyond 3 MiB.

// The kernel entry point.
.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:
	// Physical address of boot_page_table1.
	movl $(boot_page_table1 - 0xc0000000), %edi

	// First address to map is address 0.
	movl $0, %esi

1:
	cmpl $(_kernel_end - 0xc0000000), %esi
	jge 3f

	// Map physical address as "present, writable". Note that this maps text
	// and .rodata as writable. Mind security and map them as non-writable.
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

	// Size of page is 4096 bytes.
	addl $4096, %esi
	// Size of entries in boot_page_table1 is 4 bytes.
	addl $4, %edi
	// Loop to the next entry if we haven't finished.
	loop 1b

3:
	// Map VGA video memory to 0xc03ff000 as "present, writable".
	movl $(0x000b8000 | 0x003), boot_page_table1 - 0xc0000000 + 1023 * 4

	// The page table is used at both page directory entry 0 (virtually from
	// 0x0 to 0x3fffff) (thus identity mapping the kernel) and page
	// directory entry 768 (virtually from 0xc0000000 to 0xc03fffff) (thus
	// mapping it in the higher half). The kernel is identity mapped because
	// enabling paging does not change the next instruction, which continues
	// to be physical. The CPU would instead page fault if there was no
	// identity mapping.

	// Map the page table to both virtual addresses 0x00000000 and 0xc0000000.
	movl $(boot_page_table1 - 0xc0000000 + 0x3), boot_page_directory - 0xc0000000 + 0
	movl $(boot_page_table1 - 0xc0000000 + 0x3), boot_page_directory - 0xc0000000 + 768 * 4

	// Set cr3 to the address of the boot_page_directory.
	movl $(boot_page_directory - 0xc0000000), %ecx
	movl %ecx, %cr3

	// Enable paging and the write-protect bit.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	// Jump to higher half with an absolute jump.
	lea 4f, %ecx
	jmp *%ecx

.section .text
4:
	// Set up the stack.
	mov $stack_top, %esp

	// Enter the high-level kernel.
	push %eax  // magic
	push %ebx  // multiboot_info_t*
	call kernel_main

	// Infinite loop if the system has nothing more to do.
	cli
1:	hlt
	jmp 1b
