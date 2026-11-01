#ifndef GAME_FN_804220CC_H
#define GAME_FN_804220CC_H

namespace Effect {
namespace System {
struct UnkCommandData;
}
} // namespace Effect

/* Counts through the first record without a next flag or whose size this walker does not
 * supply (including types 0 and 2). */
extern "C" unsigned int fn_804220CC(const Effect::System::UnkCommandData *command);

#endif
