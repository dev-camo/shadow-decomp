typedef struct Fn8040307CObject {
    /* 0x00 */ unsigned char padding[0x48];
    /* 0x48 */ void *field_48;
} Fn8040307CObject;

extern void LSC_SetFlowLimit(void *object, int value);

void fn_8040307C(Fn8040307CObject *object, int value) {
    void *field_48 = object->field_48;

    if (field_48 != 0) {
        LSC_SetFlowLimit(field_48, value);
    }
}
