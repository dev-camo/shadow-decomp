#ifndef GAME_FN_8042A444_H
#define GAME_FN_8042A444_H

namespace Effect {
namespace System {
class EffectSystem;
}
} // namespace Effect

extern "C" {
Effect::System::EffectSystem *fn_8042A5D0();
void fn_8042A444(Effect::System::EffectSystem *memory, void *block);
}

#endif
