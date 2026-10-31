#ifndef GAME_CSD_CTEXLISTRW_H
#define GAME_CSD_CTEXLISTRW_H

#include <dolphin/types.h>
#include "Game/CSD/CTexList.h"
#include "Game/CSD/csdBase.h"
#include "rw/rwcore.h"
#include "layout_check.h"

namespace Chao {
namespace CSD {

/* The rasters of a texture dictionary's textures in the order the dictionary lists them (class name
 * inferred). The array is allocated through the library's allocator. The constructor and the
 * destructor are defined in csdTexListRW.cpp. */
class TexArray {
  public:
    TexArray(RwTexDictionary *dict);
    ~TexArray();

    /* The raster of the texture with the number, or null if there is none. */
    RwRaster *Get(int index) const {
        if (index < m_count && m_count != 0) {
            return m_rasters[index];
        }
        return NULL;
    }

    /* 0x0 */ RwRaster **m_rasters;
    /* 0x4 */ int m_count;
};
CHECK_SIZEOF(Chao::CSD::TexArray, 0x8);

/* The texture list of the RenderWare renderer (class name inferred from csdTexListRW.cpp). It
 * loads the texture dictionary of a ".RWT" stream; the texture numbers of the project data index
 * the dictionary's textures. */
class CTexListRW : public CTexList {
  public:
    CTexListRW() : m_dict(NULL), m_textures(NULL) {}
    virtual ~CTexListRW();
    virtual u32 GetType();
    virtual RCPtr<CTexList> Create();
    virtual int Load(const void *data, u32 size);
    virtual void fn_803637CC(int texture, const s32 *values, int valueCount);
    virtual void fn_803636C8(int texture, int filter, const s32 *values, int valueCount);
    virtual void fn_80363724(int addressU, int addressV, const s32 *values, int valueCount);

    /* 0x08 */ RwTexDictionary *m_dict;
    /* 0x0C */ TexArray *m_textures;
};
CHECK_SIZEOF(Chao::CSD::CTexListRW, 0x10);

/* A second texture list type, registered under the key ".RW2" (class name inferred). Its Load
 * takes a texture dictionary the caller already has (the data argument is the dictionary itself,
 * not a stream) and its destructor leaves the dictionary alone. */
class CTexListRW2 : public CTexListRW {
  public:
    CTexListRW2() {}
    virtual ~CTexListRW2();
    virtual u32 GetType();
    virtual RCPtr<CTexList> Create();
    virtual int Load(const void *data, u32 size);
};
CHECK_SIZEOF(Chao::CSD::CTexListRW2, 0x10);

} // namespace CSD
} // namespace Chao

#endif
