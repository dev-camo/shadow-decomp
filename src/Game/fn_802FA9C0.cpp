/* Unk802FA9C0 (placeholder name; class described in Game/fn_802FA9C0.h) and the stage message
 * table it requests messages from. The original file name is unknown. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first). */
#include "Game/fn_802FA9C0.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/fn_8040_random.h"

/* An entry of the message table (placeholder name): count message ids, of which one is
 * requested. */
struct UnkMessageChoice_802FAAB8 {
    /* 0x0 */ int count;
    /* 0x4 */ const int *ids;
};
CHECK_SIZEOF(UnkMessageChoice_802FAAB8, 0x8);

static const int s_ids0[2] = {0x7F0, 0x79E};
static const int s_ids1[2] = {0x74F, 0x79F};
static const int s_ids2[2] = {0x76D, 0x7A8};
static const int s_ids3[2] = {0x74E, 0x7A9};
static const int s_ids4[2] = {0x76D, 0x7B2};
static const int s_ids5[2] = {0x74E, 0x7B3};
static const int s_ids6[2] = {0x76C, 0x7BC};
static const int s_ids7[2] = {0x74F, 0x7BD};
static const int s_ids8[2] = {0x76D, 0x7C6};
static const int s_ids9[2] = {0x750, 0x7C7};
static const int s_ids10[2] = {0x76D, 0x7D0};
static const int s_ids11[2] = {0x74E, 0x7D1};
static const int s_ids12[2] = {0x76C, 0x7DA};
static const int s_ids13[2] = {0x74F, 0x7DB};
static const int s_ids14[2] = {0x76C, 0x7E4};
static const int s_ids15[2] = {0x74F, 0x7E5};
static const int s_ids16[2] = {0x76D, 0x7EE};
static const int s_ids17[2] = {0x74E, 0x7EF};
static const int s_idsC[1] = {0x762};
static const int s_idsB[1] = {0x758};
static const int s_idsA[1] = {0x744};

/* The messages of the five slots, one table for each stage that has them (in the order of the
 * stage switch below). */
static const UnkMessageChoice_802FAAB8 s_messages0[5] = {
    {2, s_ids0}, {2, s_ids1}, {1, s_idsA}, {1, s_idsB}, {1, s_idsC}};
static const UnkMessageChoice_802FAAB8 s_messages1[5] = {
    {2, s_ids2}, {2, s_ids3}, {1, s_idsA}, {1, s_idsB}, {1, s_idsC}};
static const UnkMessageChoice_802FAAB8 s_messages2[5] = {
    {2, s_ids4}, {2, s_ids5}, {1, s_idsA}, {1, s_idsB}, {1, s_idsC}};
static const UnkMessageChoice_802FAAB8 s_messages3[5] = {
    {2, s_ids6}, {2, s_ids7}, {1, s_idsA}, {1, s_idsB}, {1, s_idsC}};
static const UnkMessageChoice_802FAAB8 s_messages4[5] = {
    {2, s_ids8}, {2, s_ids9}, {1, s_idsA}, {1, s_idsB}, {1, s_idsC}};
static const UnkMessageChoice_802FAAB8 s_messages5[5] = {
    {2, s_ids10}, {2, s_ids11}, {1, s_idsA}, {1, s_idsB}, {1, s_idsC}};
static const UnkMessageChoice_802FAAB8 s_messages6[5] = {
    {2, s_ids12}, {2, s_ids13}, {1, s_idsA}, {1, s_idsB}, {1, s_idsC}};
static const UnkMessageChoice_802FAAB8 s_messages7[5] = {
    {2, s_ids14}, {2, s_ids15}, {1, s_idsA}, {1, s_idsB}, {1, s_idsC}};
static const UnkMessageChoice_802FAAB8 s_messages8[5] = {
    {2, s_ids16}, {2, s_ids17}, {1, s_idsA}, {1, s_idsB}, {1, s_idsC}};

/* The message choice of slot index for the current stage (the word at 0x4 of lbl_8057D6A0's
 * record at 0xA8), or null for a stage without messages. */
static const UnkMessageChoice_802FAAB8 *fn_802FAAB8(int index) {
    switch (lbl_8057D6A0.field_0xA8.field_0x4) {
    case 5:
        return &s_messages0[index];
    case 7:
        return &s_messages1[index];
    case 8:
        return &s_messages2[index];
    case 12:
        return &s_messages3[index];
    case 13:
        return &s_messages4[index];
    case 16:
        return &s_messages5[index];
    case 18:
        return &s_messages6[index];
    case 21:
        return &s_messages7[index];
    case 22:
        return &s_messages8[index];
    }
    return 0;
}

/* Passes an id of slot index's choice and 1 to fn_801FAC44, ignoring its result; nothing happens
 * for a stage without messages. With more than one id the index is fn_8040EA90(count), which is
 * count itself when the random float it scales rounds to 1.0f (the frsp at 0x8040EC74, USA). The
 * id is read without a bounds check, as in the original, so that index reads the word after the
 * array: for s_ids0 the 0x74F at the start of s_ids1. */
static void fn_802FAA58(int index) {
    const UnkMessageChoice_802FAAB8 *choice = fn_802FAAB8(index);
    if (choice != 0) {
        int i = 0;
        if (choice->count > 1) {
            i = fn_8040EA90(choice->count);
        }
        fn_801FAC44(fn_80109D64(), choice->ids[i], 1);
    }
}

Unk802FA9C0::Unk802FA9C0() : m_requested(0) {}

Unk802FA9C0::~Unk802FA9C0() {}

void Unk802FA9C0::fn_802FA9C0(int index) {
    if (!(m_requested & (1 << index))) {
        fn_802FAA58(index);
        m_requested |= 1 << index;
    }
}
