/* Effect command destructors kept separate until the intervening units can be merged. */
#include "Game/EffectCommandExec.h"

namespace Effect {
namespace System {

EffCmdLoopStartExec::~EffCmdLoopStartExec() {}
EffCmdLoopEndExec::~EffCmdLoopEndExec() {}
EffCmdCallbackExec::~EffCmdCallbackExec() {}

} // namespace System
} // namespace Effect
