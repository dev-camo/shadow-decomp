#include <cstdlib>

void *bsearch(const void *key, const void *base, size_t num, size_t size,
              int (*compare)(const void *, const void *)) {
    size_t l, u, m;
    const char *p;
    int c;

    if (key == NULL || base == NULL || num == 0 || size == 0 || compare == NULL) {
        return NULL;
    }

    /* the first element is checked on its own, then the search covers [1, num - 1] */
    p = (const char *)base;
    c = compare(key, p);

    if (c == 0) {
        return (void *)p;
    }

    if (c < 0) {
        return NULL;
    }

    l = 1;
    u = num - 1;

    while (l <= u) {
        m = (l + u) >> 1;
        p = (const char *)base + size * m;
        c = compare(key, p);

        if (c == 0) {
            return (void *)p;
        }

        if (c < 0) {
            u = m - 1;
        } else {
            l = m + 1;
        }
    }

    return NULL;
}
