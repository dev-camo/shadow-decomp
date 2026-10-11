typedef struct Fn8040CD20Vector {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
} Fn8040CD20Vector;

float fn_8040CD20(Fn8040CD20Vector *left, Fn8040CD20Vector *right) {
    return left->x * right->x + left->y * right->y;
}

void fn_8040CD3C(Fn8040CD20Vector *left, const Fn8040CD20Vector *right) {
    left->x = left->x + right->x;
    left->y = left->y + right->y;
}
