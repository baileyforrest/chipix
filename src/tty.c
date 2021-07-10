#include <stdbool.h>

#include "tty.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t tty_row;
size_t tty_column;
uint8_t tty_color;
uint16_t* tty_buffer;

void tty_init(void) {
  tty_row = 0;
  tty_column = 0;
  tty_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  tty_buffer = (uint16_t*) 0xB8000;
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      tty_buffer[index] = vga_entry(' ', tty_color);
    }
  }
}

void tty_setcolor(uint8_t color) {
  tty_color = color;
}

void tty_putentryat(char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  tty_buffer[index] = vga_entry(c, color);
}

void tty_putchar(char c) {
  tty_putentryat(c, tty_color, tty_column, tty_row);
  if (++tty_column == VGA_WIDTH) {
    tty_column = 0;
    if (++tty_row == VGA_HEIGHT)
      tty_row = 0;
  }
}

void tty_write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++)
    tty_putchar(data[i]);
}

size_t strlen(const char* str) {
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

void tty_writestring(const char* data) {
  tty_write(data, strlen(data));
}
