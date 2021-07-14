#ifndef STDIO_H_
#define STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define EOF (-1)

// TODO(bcf): Implement for real.
#define fprintf(stream, ...) printf(__VA_ARGS__)

int printf(const char *format, ...);
int putchar(int c);
int puts(const char *s);

#ifdef __cplusplus
}
#endif

#endif  // STDIO_H_
