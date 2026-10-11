/* Inferred raw counter view; the original C type is unknown.
 * An explicit zero definition preserves MWCC's BSS symbol order.
 */
unsigned int sjrbf_init_cnt = 0;

/* Observed clearing covers 0x4000 bytes. Preserve the four remaining
 * allocation bytes without asserting original pool or slot types.
 */
unsigned char sjrbf_obj[0x4000 + 4];
