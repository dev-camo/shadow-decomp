#include "Game/fn_8040D640.h"

#include <cstring>

/* Initializes the observed entry fields, leaving the unrecovered 0x0C interval alone. */
Fn8040D708Entry::Fn8040D708Entry() {
    field_0x0 = 0;
    field_0x4 = 0;
    field_0x8 = 0;
    field_0x14 = 30;
    field_0x18 = 4;
    field_0x1C = 30;
    field_0x20 = 1.0f;
    field_0x24 = 1.0f;
    memset(field_0x28, 0, sizeof(field_0x28));
    memset(field_0x30, 0, sizeof(field_0x30));
}

void Fn8040D708Entry::fn_8040D850(float value) {
    field_0x24 = value;
}

/* Constructs the entry array followed by the pointer-slot array. */
Fn8040D640Owner::Fn8040D640Owner() {}
