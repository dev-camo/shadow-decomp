typedef struct Fn8040CCD4Args {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
} Fn8040CCD4Args;

#pragma fp_contract off
float fn_8040CCD4(Fn8040CCD4Args *vector) {
    return vector->x * vector->x + vector->y * vector->y;
}
