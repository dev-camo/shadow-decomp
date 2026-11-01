#ifndef GAME_EFFECTSYSTEM_H
#define GAME_EFFECTSYSTEM_H

#include "layout_check.h"

namespace Effect {
namespace System {

struct UnkEffectPool;

/* Class name from RTTI; virtual method and member names are inferred from their uses.
 * The original interface has seven virtual methods and no virtual destructor. */
class EffectSystem {
  public:
    virtual void Init(void *buffer, unsigned int size) = 0;
    virtual void *Finish() = 0;
    virtual void *Allocate(unsigned int size) = 0;
    virtual void Free(void *block) = 0;
    virtual void Fill(void *destination, signed char value, unsigned int size) = 0;
    virtual void Copy(void *destination, const void *source, unsigned int size) = 0;
    virtual float Random() = 0;

    /* 0x04 */ UnkEffectPool *m_pool;
};
CHECK_SIZEOF(EffectSystem, 0x8);

} // namespace System
} // namespace Effect

#endif
