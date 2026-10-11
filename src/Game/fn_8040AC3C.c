typedef struct Fn8040AC40Node {
    /* 0x00 */ struct Fn8040AC40Node *next;
    /* 0x04 */ unsigned int value;
} Fn8040AC40Node;

typedef struct Fn8040AC40List {
    /* 0x00 */ unsigned int unknown;
    /* 0x04 */ Fn8040AC40Node *sentinel;
} Fn8040AC40List;

void fn_8040AC3C(void) {}

unsigned int fn_8040AC40(Fn8040AC40List *list) {
    unsigned int sum = 0;
    Fn8040AC40Node *node = list->sentinel->next;
    goto check;
loop:
    sum += node->value;
    node = node->next;
check:
    if (node != 0) {
        goto loop;
    }
    return sum;
}
