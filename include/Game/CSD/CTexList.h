#ifndef GAME_CSD_CTEXLIST_H
#define GAME_CSD_CTEXLIST_H

#include "Game/CSD/csdBase.h"
#include "Game/CSD/csdRCObject.h"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Chao {
namespace CSD {

/* A loader of a scene's texture list (class name from the game's RTTI, which names
 * RCPtr<Chao::CSD::CTexList>). Its RenderWare implementation, in csdTexListRW.cpp, has the vtable
 * 0x8055FC48 (class name CTexListRW inferred from that file name). Every virtual slot is pure, the
 * destructor included (it still has a body). Slot names are inferred from CTexListRW's
 * implementations; the last three are unknown and keep the implementations' addresses. */
class CTexList : public CBase {
  public:
    virtual ~CTexList() = 0;
    /* The key the texture lists are registered under (CTexListRW returns the four characters
     * ".RWT"). */
    virtual u32 GetType() = 0;
    /* A new texture list of the same type. */
    virtual RCPtr<CTexList> Create() = 0;
    /* Loads the texture list from file data in memory. */
    virtual int Load(const void *data, u32 size) = 0;
    virtual void fn_803637CC() = 0;
    virtual void fn_803636C8() = 0;
    virtual void fn_80363724() = 0;

    /* Load with the current directory set to directory, when given (name inferred). */
    int LoadInDirectory(const void *data, u32 size, const char *directory);
    /* Reads the file and loads it with the current directory set to the file's directory (name
     * inferred). */
    int LoadFile(const char *path);

    /* 0x04 */ RCPtr<unsigned char> field_0x4;
};
CHECK_SIZEOF(Chao::CSD::CTexList, 0x8);

/* The registered texture-list types, by key (names inferred). */
void InitTexLists();
void TermTexLists();
/* Replaces any texture list registered under the same key. */
void RegisterTexList(CTexList *texList);
/* A new texture list of the registered type, or a null pointer. */
RCPtr<CTexList> CreateTexList(u32 type);

} // namespace CSD
} // namespace Chao

#endif
