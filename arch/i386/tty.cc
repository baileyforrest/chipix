#include "core/tty.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "core/macros.h"
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

static int g_tty_row;
static int g_tty_col;
static u8 g_tty_color;
static u16* g_tty_buf;

static u8 vga_entry_color(enum vga_color fg, enum vga_color bg) {
  return fg | (bg << 4);
}

static u16 vga_entry(unsigned char uc, u8 color) {
  return uc | ((u16)color << 8);
}

static void tty_setcolor(u8 color) { g_tty_color = color; }

static void tty_putentryat(char c, u8 color, int x, int y) {
  assert(x >= 0 && x < VGA_WIDTH);
  assert(y >= 0 && y < VGA_HEIGHT);

  const int index = y * VGA_WIDTH + x;
  g_tty_buf[index] = vga_entry(c, color);
}

static void tty_scroll(int num_lines) {
  assert(num_lines >= 1);
  assert(num_lines <= g_tty_row);

  const int num_total = VGA_WIDTH * VGA_HEIGHT;
  const int num_scrolled = VGA_WIDTH * num_lines;
  const int num_moved = num_total - num_scrolled;

  // Scroll everything down.
  memmove(g_tty_buf, g_tty_buf + num_scrolled,
          num_moved * sizeof(g_tty_buf[0]));

  // Clear the new area.
  for (int i = 0; i < num_scrolled; ++i) {
    g_tty_buf[num_moved + i] = vga_entry(' ', g_tty_color);
  }

  g_tty_row -= num_lines;
}

void tty_init(void) {
  g_tty_row = 0;
  g_tty_col = 0;
  g_tty_buf = (u16*)0xc03ff000;
  tty_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

  for (int y = 0; y < VGA_HEIGHT; ++y) {
    for (int x = 0; x < VGA_WIDTH; ++x) {
      int index = y * VGA_WIDTH + x;
      g_tty_buf[index] = vga_entry(' ', g_tty_color);
    }
  }
}

void tty_putchar(char c) {
  if (c == '\n') {
    g_tty_col = 0;
    if (++g_tty_row == VGA_HEIGHT) {
      tty_scroll(1);
    }
    return;
  }

  tty_putentryat(c, g_tty_color, g_tty_col, g_tty_row);
  if (++g_tty_col == VGA_WIDTH) {
    g_tty_col = 0;
    if (++g_tty_row == VGA_HEIGHT) {
      tty_scroll(1);
    }
  }
}

void tty_write(const char* data, size_t size) {
  while (size--) {
    tty_putchar(*(data++));
  }
}
