#ifndef GAME_CSD_CSDMNODE_H
#define GAME_CSD_CSDMNODE_H

#include "Game/CSD/csdmScene.h"

/* The objects only csdmNode.cpp uses (CNode is in csdm.h, which CMotionPattern also needs;
 * names remain working names where evidence is absent; holder RTTI does not prove direct class RTTI). */
namespace Chao {
namespace CSD {

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
