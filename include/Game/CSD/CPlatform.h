#ifndef GAME_CSD_CPLATFORM_H
#define GAME_CSD_CPLATFORM_H

#include <dolphin/types.h>
#include "Game/CSD/csdBase.h"
#include "layout_check.h"

namespace Chao {
namespace CSD {

struct Vertex;
struct PlainVertex;

/* The library's interface to the target system: message output and 2D drawing. The class name is
 * inferred from its RenderWare implementation's name, CPlatformRW (from that class's assert text),
 * without the RW suffix. Three vtables are involved: this class's (0x8055FC10, every slot pure
 * except the destructor), CPlatformRW's (0x8055FBD8) and a class derived from it (0x8055FC70)
 * that supplies the slot CPlatformRW leaves pure. Unnamed slots keep the address of the
 * implementation in those vtables; the parameter lists of the slots called by csdLoader.cpp are
 * inferred from those calls. Deleting destructors free through CBase (the allocator's object
 * slot). */
class CPlatform : public CBase {
  public:
    CPlatform() : m_beginCount(0) {}
    virtual ~CPlatform() {}
    /* Receives the text of PrintError and PrintFatalError (name inferred); CPlatformRW's is
     * empty. */
    virtual void Print(const char *message) = 0;
    /* The implementation returns 1. */
    virtual int fn_80363BE8() = 0;
    /* Slots called by RenderState; the parameters are inferred from its calls (the array is
     * RenderState's three words at 0x30, passed with their count). The implementations
     * read only the first argument: fn_803632E4 sets two render states from a mode of 0 or 1, and
     * fn_803632A0 and fn_80363198 each set one render state to whether the mode is 1. */
    virtual void fn_803632E4(int blendMode, const s32 *values, int valueCount) = 0;
    virtual void fn_803632A0(int mode, const s32 *values, int valueCount) = 0;
    /* Name from the RenderWare implementation's assert text. DrawQuadElement and DrawTextElement
     * pass the four vertices of a quad; the implementations read only the vertices and their
     * count. */
    virtual void DrawPrimitive2D(const Vertex *vertices, int vertexCount, const s32 *values,
                                 int valueCount) = 0;
    /* Empty in every implementation. */
    virtual void fn_80362E74(const s32 *values, int valueCount) = 0;
    virtual void fn_80363198(int mode, const s32 *values, int valueCount) = 0;
    /* The same call for vertices without texture coordinates. */
    virtual void fn_803630C8(const PlainVertex *vertices, int vertexCount, const s32 *values,
                             int valueCount) = 0;
    /* Empty in every implementation. */
    virtual void fn_80362E78(const s32 *values, int valueCount) = 0;
    /* Called by the outermost Begin and End; the RenderWare implementation saves and restores
     * render states here (names inferred). */
    virtual void OnBegin() = 0;
    virtual void OnEnd() = 0;

    /* Nestable: only the outermost pair calls OnBegin and OnEnd (names inferred). */
    void Begin();
    void End();

    /* 0x04 */ int m_beginCount;
};
CHECK_SIZEOF(Chao::CSD::CPlatform, 0x8);

} // namespace CSD
} // namespace Chao

#endif
