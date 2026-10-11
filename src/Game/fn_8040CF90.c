typedef struct Fn8040CF90Vector {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
} Fn8040CF90Vector;

float fn_8040CF90(const Fn8040CF90Vector *left, const Fn8040CF90Vector *right) {
    return left->x * right->x + left->y * right->y + left->z * right->z;
}

void fn_8040CFB8(Fn8040CF90Vector *result, const Fn8040CF90Vector *left,
                 const Fn8040CF90Vector *right) {
    result->x = left->y * right->z - left->z * right->y;
    result->y = left->z * right->x - left->x * right->z;
    result->z = left->x * right->y - left->y * right->x;
}

void fn_8040CFF8(Fn8040CF90Vector *result, const Fn8040CF90Vector *value) {
    result->x += value->x;
    result->y += value->y;
    result->z += value->z;
}
