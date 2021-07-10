#include "core/tty.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/types.h"

static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;

// Hardware text mode color constants.
enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};

static int tty_row;
static int tty_col;
static u8 tty_color;
static u16* tty_buffer;

static u8 vga_entry_color(enum vga_color fg, enum vga_color bg) {
  return fg | (bg << 4);
}

static u16 vga_entry(unsigned char uc, u8 color) {
  return uc | ((u16)color << 8);
}

static void tty_setcolor(u8 color) { tty_color = color; }

static void tty_putentryat(char c, u8 color, int x, int y) {
  const int index = y * VGA_WIDTH + x;
  tty_buffer[index] = vga_entry(c, color);
}

void tty_init(void) {
  tty_row = 0;
  tty_col = 0;
  tty_buffer = (u16*)0xb8000;
  tty_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

  for (int y = 0; y < VGA_HEIGHT; ++y) {
    for (int x = 0; x < VGA_WIDTH; ++x) {
      int index = y * VGA_WIDTH + x;
      tty_buffer[index] = vga_entry(' ', tty_color);
    }
  }
}

void tty_putchar(char c) {
  if (c == '\n') {
    tty_col = 0;
    ++tty_row;
    return;
  }

  tty_putentryat(c, tty_color, tty_col, tty_row);
  if (++tty_col == VGA_WIDTH) {
    tty_col = 0;
    if (++tty_row == VGA_HEIGHT) {
      tty_row = 0;
    }
  }
}

void tty_write(const char* data, size_t size) {
  while (size--) {
    tty_putchar(*(data++));
  }
}