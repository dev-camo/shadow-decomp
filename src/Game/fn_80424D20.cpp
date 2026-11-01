/* Effect command destructors kept separate until the intervening units can be merged. */
#include "Game/EffectCommandExec.h"

namespace Effect {
namespace System {

EffCmdSingleExec::~EffCmdSingleExec() {}
EffCmdVector2Exec::~EffCmdVector2Exec() {}
EffCmdPtnAnimExec::~EffCmdPtnAnimExec() {}
EffCmdGlareSWExec::~EffCmdGlareSWExec() {}
EffCmd3DSclExec::~EffCmd3DSclExec() {}
EffCmd2DSclExec::~EffCmd2DSclExec() {}
EffCmdMatColorExec::~EffCmdMatColorExec() {}
EffCmdMatAlphaExec::~EffCmdMatAlphaExec() {}
EffCmdWaitExec::~EffCmdWaitExec() {}
EffCmdLoopStartExec::~EffCmdLoopStartExec() {}
EffCmdLoopEndExec::~EffCmdLoopEndExec() {}
EffCmdCallbackExec::~EffCmdCallbackExec() {}

} // namespace System
} // namespace Effect
