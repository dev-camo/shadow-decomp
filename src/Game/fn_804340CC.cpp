/* RW3 Arranger and launch-info methods; original filename unknown. */
#include "Game/EffectArrangerRW3.h"
#include "Game/EffectLaunchInfoRW3.h"

namespace Effect {
namespace RW3 {

EffectLaunchInfoRW3::EffectLaunchInfoRW3(System::UnkEffectFileData *file,
                                         RwTexDictionary *dictionary, RpClump *clump,
                                         unsigned int field10)
    : System::EffectLaunchInfo(file), field_0x8(dictionary), field_0xC(clump), field_0x10(field10) {
}

EffectArrangerRW3::EffectArrangerRW3(const System::UnkEffectElementData3 *definition,
                                     System::EffectLaunchInfo *info, System::EffectManager *manager)
    : System::EffectArranger(definition, info, manager) {
    m_size -= sizeof(System::EffectArranger);
    m_size += sizeof(EffectArrangerRW3);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    EffectLaunchInfoRW3 *rwInfo = dynamic_cast<EffectLaunchInfoRW3 *>(info);
    if (rwInfo != 0) {
        m_launchInfo = new EffectLaunchInfoRW3(rwInfo->field_0x4, rwInfo->field_0x8,
                                               rwInfo->field_0xC, rwInfo->field_0x10);
    }
}

EffectArrangerRW3::~EffectArrangerRW3() {
    if (m_launchInfo != 0) {
        if (dynamic_cast<EffectLaunchInfoRW3 *>(m_launchInfo) != 0) {
            delete m_launchInfo;
            m_launchInfo = 0;
        }
    }
}

} // namespace RW3
} // namespace Effect
