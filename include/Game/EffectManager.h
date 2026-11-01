#ifndef GAME_EFFECTMANAGER_H
#define GAME_EFFECTMANAGER_H

#include "Game/EffectElement.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Effect {
namespace System {

/* Class name from RTTI; method names, qualifiers and member names are inferred from use. */
class EffectManager {
  public:
    virtual ~EffectManager();
    virtual boost::weak_ptr<EffectElement> Create(EffectLaunchInfo *info, int id);
    virtual boost::weak_ptr<EffectElement> Create(const boost::shared_ptr<EffectLaunchInfo> &info,
                                                  int id);
    virtual EffectElement *CreateElement(const UnkEffectElementData *definition,
                                         EffectLaunchInfo *info);

    /* 0x04 */ boost::shared_ptr<EffectElement> m_updateHead;
    /* 0x0C */ boost::shared_ptr<EffectElement> m_updateTail;
    /* 0x14 */ boost::shared_ptr<EffectElement> m_renderHead;
    /* 0x1C */ boost::shared_ptr<EffectElement> m_renderTail;
    /* 0x24 */ unsigned char field_0x24;
    /* 0x25 */ unsigned char field_0x25;
    /* 0x28 */ unsigned int field_0x28;
};
CHECK_SIZEOF(EffectManager, 0x2C);

} // namespace System
} // namespace Effect

#endif
