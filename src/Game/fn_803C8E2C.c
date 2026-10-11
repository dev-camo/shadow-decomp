/* Inferred storage layout; the trailing four bytes have no established type. */
typedef struct {
    /* 0x00 */ volatile unsigned int count;
    /* 0x04 */ unsigned char reserved[4];
} NestingState;

NestingState lbl_805BACC0;
extern volatile unsigned int sjcrs_lvl;

/* SJCRS_Finish/SJCRS_Init (names and bodies as in the Xenoblade CRI sj_crs.c).
 * Signed comparison views select target cmpwi without asserting original signedness.
 */
void SJCRS_Finish(void) {
    lbl_805BACC0.count--;
    if ((int)lbl_805BACC0.count == 0) {
        sjcrs_lvl = 0;
    }
}

void SJCRS_Init(void) {
    lbl_805BACC0.count++;
    if ((int)lbl_805BACC0.count == 1) {
        sjcrs_lvl = 0;
    }
}
