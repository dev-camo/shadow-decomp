/* Takes the pointer out of *slot, leaving NULL behind (behaviour inferred; the original name is
 * unknown). */
void *fn_8040D9D8(void **slot) {
    void *value = *slot;
    *slot = 0;
    return value;
}
