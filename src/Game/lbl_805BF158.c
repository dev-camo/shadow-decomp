/* Inferred raw counter view; the original C type is unknown.
 * An explicit zero definition preserves MWCC's BSS symbol order.
 */
unsigned int sjuni_init_cnt = 0;

/* Observed clearing covers 0xC00 bytes. Preserve the four remaining
 * allocation bytes without asserting original pool or slot types.
 */
unsigned char sjuni_obj[0xC00 + 4];
