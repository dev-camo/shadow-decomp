typedef struct Fn8040CF2CVector {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
} Fn8040CF2CVector;

#pragma fp_contract off
float fn_8040CF2C(Fn8040CF2CVector *vector) {
    return vector->x * vector->x + vector->y * vector->y + vector->z * vector->z;
}
