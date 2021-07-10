#ifndef STDIO_H_
#define STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define EOF (-1)

int printf(const char *format, ...);
int putchar(int c);
int puts(const char *s);

#ifdef __cplusplus
}
#endif

#endif  // STDIO_H_
