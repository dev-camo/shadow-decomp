#ifndef GAME_FN_801A35AC_H
#define GAME_FN_801A35AC_H

#include "Game/fn_8040_vector_types.h"
#include "Game/fn_80027D5C.h"
#include "boost/scoped_array.hpp"
#include "rw/rwcore.h"
#include "layout_check.h"

struct RpClump;
class Unk80015A7C;
struct _stMtnPlayWork;

/* Original type name unknown; the array constructor uses a 0x44 stride. */
class Unk801A35ACItem {
  public:
    Unk801A35ACItem();
    ~Unk801A35ACItem();
    void fn_801A3F0C(_stMtnPlayWork *control);
    void fn_801A3F90(float red, float green, float blue, float alpha);
    void fn_801A3FB0(float value);
    void fn_801A4144();
    void fn_801A4298(int resourceKey, int index);
    void fn_801A3EAC(const Unk801A35ACParameters &parameters);
    void fn_801A41D8(const Fn8040Matrix34 &matrix);

    /* 0x00 */ RpClump *m_clump;
    /* 0x04 */ RwFrame *m_frame;
    /* 0x08 */ Unk80015A7C *field_0x8;
    /* 0x0C */ float field_0xC;
    /* 0x10 */ int field_0x10;
    /* 0x14 */ int field_0x14;
    /* 0x18 */ Unk801A35ACParameters m_parameters;
};
CHECK_SIZEOF(Unk801A35ACItem, 0x44);

struct Unk801A35ACEntry {
    /* 0x00 */ short m_index;
    /* 0x02 */ short m_parentFrameId;
};
CHECK_SIZEOF(Unk801A35ACEntry, 0x4);

struct Unk801A35ACDescriptor {
    /* 0x00 */ const Unk801A35ACEntry *m_entries;
    /* 0x04 */ int m_count;
};
CHECK_SIZEOF(Unk801A35ACDescriptor, 0x8);

/* Original type name unknown; member names are inferred. */
class Unk801A35AC {
  public:
    Unk801A35AC();
    ~Unk801A35AC();
    void fn_801A3814(RwFrame *parent);
    void fn_801A386C(const Fn8040Matrix34 *transform);
    void fn_801A38E8();
    void fn_801A395C(float red, float green, float blue, float alpha);
    void fn_801A39E0(float value);
    void fn_801A3A44();
    void fn_801A3AA0();
    void fn_801A3AFC();
    void fn_801A3B58();
    void fn_801A3CA4(int resourceKey, short index, _stMtnPlayWork *control);
    void fn_801A3CE4(int resourceKey, const Unk801A35ACDescriptor &descriptor,
                     _stMtnPlayWork *control);
    void fn_801A35AC(const Unk801A35ACParameters &parameters, int index);
    Unk801A35ACParameters *fn_801A3634(int index);
    Unk801A35ACItem *fn_801A366C(int index);
    void fn_801A367C(const Fn8040Vector3 &position, const Fn8040Vector3 &rotation);
    RwFrame *fn_801A3704(int id);
    RwFrame *fn_801A3750();
    RpClump *fn_801A378C();
    void fn_801A37C8();

    /* 0x00 */ boost::scoped_array<Unk801A35ACItem> m_items;
    /* 0x04 */ int m_count;
};
CHECK_SIZEOF(Unk801A35AC, 0x8);

#endif
