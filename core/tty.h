#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void TtyInit(void);
void TtyPutchar(char c);
void TtyWrite(const char* data, size_t size);

#ifdef __cplusplus
}
#endif
