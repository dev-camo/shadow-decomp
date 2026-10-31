#ifndef GAME_CSD_RENDERSTATE_H
#define GAME_CSD_RENDERSTATE_H

#include <dolphin/types.h>
#include "Game/CSD/CTexList.h"
#include "Game/CSD/csdRCObject.h"
#include "layout_check.h"

/* The library's cache of the renderer's state while it draws: a setting is stored here and
 * reaches the platform and the texture list only when the next quad is drawn, and only if
 * something changed since the last one. Class, member and function names are inferred from
 * how csdLoader.cpp uses it; members whose meaning is not clear keep their offset. */
namespace Chao {
namespace CSD {

class RenderState {
  public:
    RenderState();
    ~RenderState();

    /* The outermost Enter starts the platform's drawing (CPlatform::Begin); the Leave that
     * balances it ends it. */
    void Enter();
    void Leave();

    /* Each setter marks the state changed only when the value differs. */
    void SetTexture(const RCPtr<CTexList> &texList, int texture);
    void SetAddressing(int addressU, int addressV);
    void SetFilter(int filter);
    void SetBlendMode(int mode);
    void SetField0x24(int value);
    void SetTextured(int textured);
    void SetField0x30(const s32 *values);

    /* Sends what changed since the last call to the platform and the texture list. */
    void Apply();

  private:
    void Begin();
    void End();

    /* 0x00 */ int m_enterCount;
    /* 0x04 */ int m_applied;
    /* 0x08 */ int m_changed;
    /* 0x0C */ RCPtr<CTexList> m_texList;
    /* 0x10 */ int m_texture;
    /* 0x14 */ int m_addressU;
    /* 0x18 */ int m_addressV;
    /* 0x1C */ int m_filter;
    /* 0x20 */ int m_blendMode;
    /* 0x24 */ int m_field_0x24;
    /* 0x28 */ int m_textured;
    /* 0x2C */ int m_appliedTextured;
    /* 0x30 */ s32 m_field_0x30[3];
    /* 0x3C */ s32 m_appliedField0x30[3];
};
CHECK_SIZEOF(Chao::CSD::RenderState, 0x48);

/* The one render state of the library. */
RenderState &GetRenderState();

} // namespace CSD
} // namespace Chao

#endif
