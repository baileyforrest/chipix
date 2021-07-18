/* The bootloader will look at this image and start execution at the symbol
 * designated as the entry point.
 */
ENTRY(_start)

SECTIONS {
  /* The kernel will live at 3GB + 1MB in the virtual address space, which will
   * be mapped to 1MB in the physical address space.
   *
   * Note that we page-align the sections.
   */
  . = 1M;

  __kernel_begin = .;
  .multiboot.data : { *(.multiboot.data) }
  .multiboot.text : { *(.multiboot.text) }

  . += 0xc0000000;
  __text_begin = .;
  .text ALIGN (4K) : AT (ADDR (.text) - 0xc0000000) {
    *(.text)
  }
  __text_end = .;

  __rodata_begin = .;
  .rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xc0000000) {
    *(.rodata)
  }
  __rodata_end = .;

  .data ALIGN (4K) : AT (ADDR (.data) - 0xc0000000) {
    *(.data)
  }

  /* TODO(bcf): Do we need to manually zero BSS? */
  .bss ALIGN (4K) : AT (ADDR (.bss) - 0xc0000000) {
    *(COMMON)
    *(.bss)
    *(.bootstrap_stack)
  }

  /* Add a symbol that indicates the end address of the kernel. */
  __kernel_end = .;
}
