#include "core/tty.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "core/macros.h"
#include "core/types.h"

namespace {

constexpr int kVgaWidth = 80;
constexpr int kVgaHeight = 25;

// Hardware text mode color constants.
enum VgaColor {
  kVgaColorBlack = 0,
  kVgaColorBlue = 1,
  kVgaColorGreen = 2,
  kVgaColorCyan = 3,
  kVgaColorRed = 4,
  kVgaColorMagenta = 5,
  kVgaColorBrown = 6,
  kVgaColorLightGrey = 7,
  kVgaColorDarkGrey = 8,
  kVgaColorLightBlue = 9,
  kVgaColorLightGreen = 10,
  kVgaColorLightCyan = 11,
  kVgaColorLightRed = 12,
  kVgaColorLightMagenta = 13,
  kVgaColorLightBrown = 14,
  kVgaColorWhite = 15,
};

int g_tty_row = 0;
int g_tty_col = 0;
u8 g_tty_color;
u16* g_tty_buf = reinterpret_cast<u16*>(0xc03ff000);

u8 VgaEntryColor(VgaColor fg, VgaColor bg) {
  return fg | (bg << 4);
}

u16 VgaEntry(u8 c, u8 color) {
  return c | ((u16)color << 8);
}

void SetColor(u8 color) { g_tty_color = color; }

void PutEntryAt(char c, u8 color, int x, int y) {
  assert(x >= 0 && x < kVgaWidth);
  assert(y >= 0 && y < kVgaHeight);

  const int index = y * kVgaWidth + x;
  g_tty_buf[index] = VgaEntry(c, color);
}

void Scroll(int num_lines) {
  assert(num_lines >= 1);
  assert(num_lines <= g_tty_row);

  const int num_total = kVgaWidth * kVgaHeight;
  const int num_scrolled = kVgaWidth * num_lines;
  const int num_moved = num_total - num_scrolled;

  // Scroll everything down.
  memmove(g_tty_buf, g_tty_buf + num_scrolled,
          num_moved * sizeof(g_tty_buf[0]));

  // Clear the new area.
  for (int i = 0; i < num_scrolled; ++i) {
    g_tty_buf[num_moved + i] = VgaEntry(' ', g_tty_color);
  }

  g_tty_row -= num_lines;
}

}  // namespace

void TtyInit(void) {
  SetColor(VgaEntryColor(kVgaColorLightGrey, kVgaColorBlack));

  for (int y = 0; y < kVgaHeight; ++y) {
    for (int x = 0; x < kVgaWidth; ++x) {
      int index = y * kVgaWidth + x;
      g_tty_buf[index] = VgaEntry(' ', g_tty_color);
    }
  }
}

void TtyPutchar(char c) {
  if (c == '\n') {
    g_tty_col = 0;
    if (++g_tty_row == kVgaHeight) {
      Scroll(1);
    }
    return;
  }

  PutEntryAt(c, g_tty_color, g_tty_col, g_tty_row);
  if (++g_tty_col == kVgaWidth) {
    g_tty_col = 0;
    if (++g_tty_row == kVgaHeight) {
      Scroll(1);
    }
  }
}

void TtyWrite(const char* data, size_t size) {
  while (size--) {
    TtyPutchar(*(data++));
  }
}
