#ifndef GAME_EFFECTGLOBALPARAMRW3_H
#define GAME_EFFECTGLOBALPARAMRW3_H

#include "Game/EffectGlobalParam.h"
#include "Game/UnkEffectRW3Parameters.h"

namespace Effect {
namespace RW3 {

/* Class and base names are recorded in RTTI; member names and parameter qualifiers are inferred. */
class EffectGlobalParamRW3 : public System::EffectGlobalParam {
  public:
    EffectGlobalParamRW3(System::UnkEffectElementData8 *definition, System::EffectLaunchInfo *info,
                         System::EffectManager *manager);
    virtual ~EffectGlobalParamRW3();
    virtual void Update(float deltaTime);

    /* 0x10C */ UnkEffectRW3Param0 m_initial0;
    /* 0x12C */ UnkEffectRW3Param0 m_target0;
    /* 0x14C */ UnkEffectRW3Param1 m_initial1;
    /* 0x15C */ UnkEffectRW3Param1 m_target1;
    /* 0x16C */ UnkEffectRW3Param2 m_initial2;
    /* 0x177 */ UnkEffectRW3Param2 m_target2;
    /* 0x184 */ UnkEffectRW3Param3 m_initial3;
    /* 0x1BC */ UnkEffectRW3Param3 m_target3;
};
CHECK_SIZEOF(EffectGlobalParamRW3, 0x1F4);

} // namespace RW3
} // namespace Effect

#endif
