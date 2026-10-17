#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <stddef.h>
#include <wchar.h>

#define RAND_MAX 32767

#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *calloc(size_t num, size_t size);
void srand(unsigned int seed);
int rand(void);
void abort(void);
void exit(int status);
size_t wcstombs(char *dest, const wchar_t *src, size_t max);
unsigned long strtoul(const char *str, char **end, int base);
void *bsearch(const void *key, const void *base, size_t num, size_t size,
              int (*compare)(const void *, const void *));

#ifdef __cplusplus
}
#endif

#ifdef __MWERKS__
#define abs(x) __abs(x)
#else
static inline int abs(int x) {
    int mask = x >> 31;
    return (x + mask) ^ mask;
}
#endif

#endif
