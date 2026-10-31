#ifndef GAME_TORNADOCHARINFO_H
#define GAME_TORNADOCHARINFO_H

#include "Game/GadgetCharInfo.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class Tornado;

/* TornadoCharInfo (class name from RTTI, with CharInfo and GadgetCharInfo as its bases): the
 * 0x70-byte character info that the Tornado gadget registers with the character info list under
 * the constructor argument 0x2A. Only the pointer to the gadget at 0x6C is mapped. */
class TornadoCharInfo : public GadgetCharInfo {
  public:
    /* Hands victim to the tornado when tag equals the tornado's tag byte at 0x74, and reports
     * whether it did (behaviour inferred; the original name is unknown). */
    bool AttachTarget(int tag, boost::shared_ptr<CharInfo> victim);

    /* 0x6C */ Tornado *m_tornado;
};
CHECK_SIZEOF(TornadoCharInfo, 0x70);

#endif
