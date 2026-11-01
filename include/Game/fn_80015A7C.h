#ifndef GAME_FN_80015A7C_H
#define GAME_FN_80015A7C_H

#include "Game/fn_8040_vector_types.h"
#include "rw/rpworld.h"
#include "layout_check.h"

struct _stMtnPlayWork;
struct Unk80015A7CNode;

/* Original name unknown; the clump and motion control are borrowed. */
class Unk80015A7C {
  public:
    Unk80015A7C(RpClump *clump, _stMtnPlayWork *control);
    ~Unk80015A7C();
    void fn_80015954(const Fn8040Matrix34 *transform);
    void fn_80015980(const Fn8040Vector3 *offset);
    void fn_800159AC();

    /* 0x00 */ RpClump *field_0x0;
    /* 0x04 */ _stMtnPlayWork *field_0x4;
    /* 0x08 */ Unk80015A7CNode *field_0x8;
    /* 0x0C */ Unk80015A7CNode *field_0xC;
    /* 0x10 */ RwMatrix *field_0x10;
    /* 0x14 */ const Fn8040Vector3 *field_0x14;
    /* 0x18 */ const Fn8040Matrix34 *field_0x18;
};
CHECK_SIZEOF(Unk80015A7C, 0x1C);

#endif
