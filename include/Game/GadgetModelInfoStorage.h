#ifndef GAME_GADGETMODELINFO_STORAGE_H
#define GAME_GADGETMODELINFO_STORAGE_H

#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

struct RpClump;
struct RwFrame;
struct RwMatrix;
struct _stMtnPlayWork;
struct UnkModelEntry_8015F7BC;
class Unk80015A7C;
struct UnkModelAtomicState;
struct UnkModelOwnedAllocation;

/* A motion entry of the model resource. Names and the contents at +8 are unrecovered. */
struct UnkModelMotionResource {
    /* 0x00 */ const char *field_0x0;
    /* 0x04 */ const char *field_0x4;
    /* 0x08 */ u8 unknown_0x8[4];
    /* 0x0C */ const char *field_0xC;
    /* 0x10 */ const char *field_0x10;
    /* 0x14 */ const char *field_0x14;
    /* 0x18 */ int field_0x18;
};
CHECK_SIZEOF(UnkModelMotionResource, 0x1C);

/* The list's motion payload; its native destructor releases the shared control only. */
struct UnkModelMotionData {
    UnkModelMotionData();
    UnkModelMotionData(const UnkModelMotionData &other);
    ~UnkModelMotionData();

    /* 0x00 */ const char *field_0x0;
    /* 0x04 */ boost::shared_ptr<_stMtnPlayWork> field_0x4;
    /* 0x0C */ Unk80015A7C *field_0xC;
    /* 0x10 */ unsigned int field_0x10;
    /* 0x14 */ u32 field_0x14;
    /* 0x18 */ const UnkModelMotionResource *field_0x18;
    /* 0x1C */ u32 field_0x1C;
};
CHECK_SIZEOF(UnkModelMotionData, 0x20);

/* The full part payload is separate from the small UnkModelPart_8015FA78 callback record.
 * Its native destructor is empty; fn_80162944 performs the explicit allocation cleanup. */
struct UnkModelPartData {
    UnkModelPartData();
    UnkModelPartData(const UnkModelPartData &other);
    ~UnkModelPartData();
    void fn_80162944();

    /* 0x00 */ RpClump *field_0x0;
    /* 0x04 */ RwFrame *field_0x4;
    /* 0x08 */ const char *field_0x8;
    /* 0x0C */ u16 field_0xC;
    /* 0x0E */ u16 field_0xE;
    /* 0x10 */ const UnkModelEntry_8015F7BC *field_0x10;
    /* 0x14 */ RwMatrix *field_0x14;
    /* 0x18 */ u32 field_0x18;
    /* 0x1C */ UnkModelAtomicState *field_0x1C;
    /* 0x20 */ UnkModelOwnedAllocation *field_0x20;
};
CHECK_SIZEOF(UnkModelPartData, 0x24);

/* The embedded sentinel and the links of each allocated node have this representation.
 * The value-bearing node representation is not recovered; native accessors supply its payload. */
struct UnkModelListLink {
    /* 0x00 */ UnkModelListLink *previous;
    /* 0x04 */ UnkModelListLink *next;
};
CHECK_SIZEOF(UnkModelListLink, 0x8);

/* Iterator names and operation spellings are inferred. The node-value constructor represents
 * the pointer initialization performed by the native begin/end result creators. */
struct UnkModelMotionIterator {
    explicit UnkModelMotionIterator(UnkModelListLink *node) : node(node) {}
    UnkModelMotionData &operator*() const;
    bool operator!=(const UnkModelMotionIterator &other) const {
        return node != other.node;
    }
    UnkModelMotionIterator &operator++() {
        node = node->next;
        return *this;
    }
    UnkModelMotionIterator operator++(int) {
        UnkModelMotionIterator previous = *this;
        node = node->next;
        return previous;
    }

    /* 0x00 */ UnkModelListLink *node;
};
CHECK_SIZEOF(UnkModelMotionIterator, 0x4);

struct UnkModelPartIterator {
    explicit UnkModelPartIterator(UnkModelListLink *node) : node(node) {}
    UnkModelPartData &operator*() const;
    bool operator!=(const UnkModelPartIterator &other) const {
        return node != other.node;
    }
    UnkModelPartIterator &operator++() {
        node = node->next;
        return *this;
    }
    UnkModelPartIterator operator++(int) {
        UnkModelPartIterator previous = *this;
        node = node->next;
        return previous;
    }

    /* 0x00 */ UnkModelListLink *node;
};
CHECK_SIZEOF(UnkModelPartIterator, 0x4);

/* Native container interfaces; their original library and type names are unknown.
 * The first word counts allocated entries, followed by the embedded sentinel. */
class UnkModelMotionList {
  public:
    UnkModelMotionList();
    ~UnkModelMotionList();
    UnkModelMotionIterator begin();
    UnkModelMotionIterator end();
    UnkModelListLink *fn_80160000();

    /* 0x00 */ u32 count;
    /* 0x04 */ UnkModelListLink sentinel;

  private:
    /* Container copying has not been recovered. */
    UnkModelMotionList(const UnkModelMotionList &);
    UnkModelMotionList &operator=(const UnkModelMotionList &);
};
CHECK_SIZEOF(UnkModelMotionList, 0xC);

class UnkModelPartList {
  public:
    UnkModelPartList();
    ~UnkModelPartList();
    UnkModelPartIterator begin();
    UnkModelPartIterator end();
    void clear();
    UnkModelListLink *fn_8015FB44();

    /* 0x00 */ u32 count;
    /* 0x04 */ UnkModelListLink sentinel;

  private:
    /* Container copying has not been recovered. */
    UnkModelPartList(const UnkModelPartList &);
    UnkModelPartList &operator=(const UnkModelPartList &);
};
CHECK_SIZEOF(UnkModelPartList, 0xC);

#endif
