#ifndef GAME_CSD_CSDMNODE_H
#define GAME_CSD_CSDMNODE_H

#include "Game/CSD/csdmScene.h"

/* The objects only csdmNode.cpp uses (CNode is in csdm.h, which CMotionPattern also needs;
 * names remain working names where evidence is absent; holder RTTI does not prove direct class RTTI). */
namespace Chao {
namespace CSD {

/* The callback interface held by the node's counted-reference list (name and signature inferred).
 * Invoke returns whether the callback remains in the list. Only this virtual interface is
 * recovered; this declaration is not used to allocate or copy callback objects. */
class CNodeCallback {
  public:
    virtual bool Invoke(CNode *node) = 0;
    virtual ~CNodeCallback();
};
CHECK_SIZEOF(Chao::CSD::CNodeCallback, 0x4);

/* Partial persistent-listener interface (class and member names inferred). Native dispatch uses
 * slot 0x0C and removal uses the node-pointer list at 0x04. Slot 0x08 is unestablished: fn_0x8
 * reserves that slot, with an unknown signature, and is never called through this interface.
 * The full listener extent is unknown. This declaration is not used to construct or delete it. */
class CNodeListener {
  public:
    virtual void fn_0x8() = 0;
    virtual void fn_0xC(CNode *node) = 0;

    void fn_8036B9FC(CNode *node);

    /* 0x04 */ std::list<CNode *> m_nodes;
};
CHECK_SIZEOF(Chao::CSD::CNodeListener, 0x10);

/* The two objects a node creates in the block it allocates for them: the pattern of its entry
 * and a copy of the fonts it was given (see CNode::SetFonts). */
struct NodeObjects {
    /* 0x00 */ CMotionPattern pattern;
    /* 0x18 */ CFontList fonts;
};
CHECK_SIZEOF(Chao::CSD::NodeObjects, 0x24);

} // namespace CSD
} // namespace Chao

#endif
