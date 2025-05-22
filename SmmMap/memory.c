#include "memory.h"

void *ZMemSet(void *ptr, int value, UINT64 num) {
    unsigned char *p = (unsigned char *)ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

