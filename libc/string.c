#include <string.h>

int memcmp(const void* s1, const void* s2, size_t n) {
  const unsigned char* s1_char = s1;
  const unsigned char* s2_char = s2;

  while (n--) {
    unsigned char c1 = *(s1_char++);
    unsigned char c2 = *(s2_char++);

    if (c1 < c2) {
      return -1;
    } else if (c1 > c2) {
      return 1;
    }
  }

  return 0;
}

void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
  char* dest_char = dest;
  const char* src_char = src;

  while (n--) {
    *(dest_char++) = *(src_char++);
  }

  return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
  char* dest_char = dest;
  const char* src_char = src;

  if (dest_char < src_char) {
    while (n--) {
      *(dest_char++) = *(src_char++);
    }
  } else {
    dest_char += n;
    src_char += n;

    while (n--) {
      *(--dest_char) = *(--src_char);
    }
  }

  return dest;
}

void* memset(void* s, int c, size_t n) {
  unsigned char* buf = s;

  while (n--) {
    *(buf++) = c;
  }

  return s;
}

size_t strlen(const char* s) {
  size_t len = 0;
  while (*(s++) != '\0') {
    ++len;
  }

  return len;
}
