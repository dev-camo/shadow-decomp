typedef struct Fn8040AC40Node {
    /* 0x00 */ struct Fn8040AC40Node *next;
    /* 0x04 */ unsigned int value;
#include "layout_check.h"
} Fn8040AC40Node;
CHECK_SIZEOF(Fn8040AC40Node, 0x8);

typedef struct Fn8040AC40List {
    /* 0x00 */ unsigned int unknown;
    /* 0x04 */ Fn8040AC40Node *sentinel;
} Fn8040AC40List;
CHECK_SIZEOF(Fn8040AC40List, 0x8);

void fn_8040AC3C(void) {}

unsigned int fn_8040AC40(Fn8040AC40List *list) {
    unsigned int sum = 0;
    Fn8040AC40Node *node = list->sentinel->next;

    while (node != 0) {
        sum += node->value;
        node = node->next;
    }
    return sum;
}
