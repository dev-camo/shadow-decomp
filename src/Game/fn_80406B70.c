typedef struct Fn80406B70Global {
    /* 0x00 */ unsigned char padding[0x38];
    /* 0x38 */ int field_38;
    /* 0x3C */ int field_3C;
} Fn80406B70Global;

extern Fn80406B70Global lbl_805DD9CC;

int fn_80406B70(void) {
    return lbl_805DD9CC.field_3C;
}

int fn_80406B80(void) {
    return lbl_805DD9CC.field_38;
}
