#ifndef GAME_CSD_CSDMNODE_H
#define GAME_CSD_CSDMNODE_H

#include "Game/CSD/csdmScene.h"

/* The objects that only csdmNode.cpp uses (CNode is in csdm.h, which CMotionPattern also needs;
 * names inferred, as this library's GameCube objects carry no run-time type information). */
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
