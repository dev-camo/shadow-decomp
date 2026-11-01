/* EffectGlobalParamRW3 methods; original filename unknown. */
#include "Game/EffectGlobalParamRW3.h"
#include "Game/EffectManagerRW3.h"
#include "Game/UnkEffectGlobalParamData.h"

namespace Effect {
namespace RW3 {

EffectGlobalParamRW3::EffectGlobalParamRW3(System::UnkEffectElementData8 *definition,
                                           System::EffectLaunchInfo *info,
                                           System::EffectManager *manager)
    : System::EffectGlobalParam(definition, info, manager) {
    m_size -= sizeof(System::EffectGlobalParam);
    m_size += sizeof(EffectGlobalParamRW3);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    EffectManagerRW3 *rwManager = dynamic_cast<EffectManagerRW3 *>(m_manager);
    if (rwManager != 0) {
        switch (field_0x108->type) {
        case 0: {
            m_initial0 = rwManager->fn_80430844();
            const System::UnkEffectGlobalParamData0 *data =
                static_cast<const System::UnkEffectGlobalParamData0 *>(field_0x108);
            if ((m_definitionFlags & 0x10000) != 0) {
                m_target0.field_0x0 = 1;
            } else {
                m_target0.field_0x0 = 0;
            }
            if ((data->flags & 1) != 0) {
                m_target0.field_0x1 = 1;
            } else {
                m_target0.field_0x1 = 0;
            }
            if ((data->flags & 2) != 0) {
                m_target0.field_0x2 = 1;
            } else {
                m_target0.field_0x2 = 0;
            }
            if ((data->flags & 4) != 0) {
                m_target0.field_0x3 = 1;
            } else {
                m_target0.field_0x3 = 0;
            }
            if ((data->flags & 8) != 0) {
                m_target0.field_0x4 = 1;
            } else {
                m_target0.field_0x4 = 0;
            }
            m_target0.field_0x5 = data->field_0x8;
            m_target0.field_0x6 = data->field_0x9;
            m_target0.field_0x8 = data->field_0xC;
            m_target0.field_0xC = data->field_0x10;
            m_target0.field_0x10 = data->field_0x14;
            m_target0.field_0x14 = data->field_0x18;
            m_target0.field_0x18 = data->field_0x1C;
            m_target0.field_0x1C = data->field_0x20;
            break;
        }
        case 1: {
            m_initial1 = rwManager->fn_8042E034();
            const System::UnkEffectGlobalParamData1 *data =
                static_cast<const System::UnkEffectGlobalParamData1 *>(field_0x108);
            if ((m_definitionFlags & 0x10000) != 0) {
                m_target1.field_0x0 = 1;
            } else {
                m_target1.field_0x0 = 0;
            }
            m_target1.field_0x1 = data->field_0x8;
            m_target1.field_0x4 = data->field_0xC;
            m_target1.field_0x8[0] = data->field_0x10;
            m_target1.field_0x8[1] = data->field_0x14;
            break;
        }
        case 2: {
            m_initial2 = rwManager->fn_804388A4();
            const System::UnkEffectGlobalParamData2 *data =
                static_cast<const System::UnkEffectGlobalParamData2 *>(field_0x108);
            if ((m_definitionFlags & 0x10000) != 0) {
                m_target2.field_0x0 = 1;
            } else {
                m_target2.field_0x0 = 0;
            }
            if ((data->flags & 0x10) != 0) {
                m_target2.field_0x1 = 1;
            } else {
                m_target2.field_0x1 = 0;
            }
            if ((data->flags & 0x20) != 0) {
                m_target2.field_0x2 = 1;
            } else {
                m_target2.field_0x2 = 0;
            }
            m_target2.field_0x3.red = data->field_0x8.red;
            m_target2.field_0x3.green = data->field_0x8.green;
            m_target2.field_0x3.blue = data->field_0x8.blue;
            m_target2.field_0x3.alpha = data->field_0x8.alpha;
            m_target2.field_0x7.red = data->field_0xC.red;
            m_target2.field_0x7.green = data->field_0xC.green;
            m_target2.field_0x7.blue = data->field_0xC.blue;
            m_target2.field_0x7.alpha = data->field_0xC.alpha;
            break;
        }
        case 3: {
            m_initial3 = rwManager->fn_80436F2C();
            const System::UnkEffectGlobalParamData3 *data =
                static_cast<const System::UnkEffectGlobalParamData3 *>(field_0x108);
            if ((m_definitionFlags & 0x10000) != 0) {
                m_target3.field_0x0 = 1;
            } else {
                m_target3.field_0x0 = 0;
            }
            m_target3.field_0x8.field_0x0 = data->field_0xC;
            m_target3.field_0x8.field_0x4 = data->field_0x24;
            m_target3.field_0x8.field_0x8 = data->field_0x2C;
            m_target3.field_0x8.field_0xC = data->field_0x34;
            m_target3.field_0x8.field_0x10 = data->field_0x14;
            m_target3.field_0x8.field_0x14 = data->field_0x1C;
            m_target3.field_0x20.field_0x0 = data->field_0x10;
            m_target3.field_0x20.field_0x4 = data->field_0x28;
            m_target3.field_0x20.field_0x8 = data->field_0x30;
            m_target3.field_0x20.field_0xC = data->field_0x38;
            m_target3.field_0x20.field_0x10 = data->field_0x18;
            m_target3.field_0x20.field_0x14 = data->field_0x20;
            break;
        }
        }
    }
}

EffectGlobalParamRW3::~EffectGlobalParamRW3() {
    EffectManagerRW3 *manager = dynamic_cast<EffectManagerRW3 *>(m_manager);
    if (manager != 0) {
        switch (field_0x108->type) {
        case 0:
            manager->fn_804308C0(m_target0);
            break;
        case 1:
            manager->fn_8042E098(m_target1);
            break;
        case 2:
            manager->fn_8043891C(m_target2);
            break;
        case 3:
            manager->fn_80436FC0(m_target3);
            break;
        }
    }
}

void EffectGlobalParamRW3::Update(float deltaTime) {
    System::EffectGlobalParam::Update(deltaTime);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    if ((m_flags & System::ELEMENT_FLAG_4) != 0) {
        return;
    }
    EffectManagerRW3 *manager = dynamic_cast<EffectManagerRW3 *>(m_manager);
    if (manager == 0) {
        return;
    }
    switch (field_0x108->type) {
    case 0: {
        UnkEffectRW3Param0 result = m_target0;
        if (m_progress < 0.9999f) {
            if (result.field_0x0 || m_initial0.field_0x0) {
                result.field_0x0 = 1;
            } else {
                result.field_0x0 = 0;
            }
            if (result.field_0x1 || m_initial0.field_0x1) {
                result.field_0x1 = 1;
            } else {
                result.field_0x1 = 0;
            }
            if (result.field_0x2 || m_initial0.field_0x2) {
                result.field_0x2 = 1;
            } else {
                result.field_0x2 = 0;
            }
            if (result.field_0x5 != m_initial0.field_0x5) {
                result.field_0x5 = static_cast<int>(result.field_0x5 * m_progress +
                                                    m_initial0.field_0x5 * (1.0f - m_progress));
            }
            if (result.field_0x6 != m_initial0.field_0x6) {
                result.field_0x6 = static_cast<int>(result.field_0x6 * m_progress +
                                                    m_initial0.field_0x6 * (1.0f - m_progress));
            }
            result.field_0x8 =
                result.field_0x8 * m_progress + m_initial0.field_0x8 * (1.0f - m_progress);
            result.field_0xC =
                result.field_0xC * m_progress + m_initial0.field_0xC * (1.0f - m_progress);
            if (result.field_0x10 != m_initial0.field_0x10) {
                result.field_0x10 = static_cast<int>(result.field_0x10 * m_progress +
                                                     m_initial0.field_0x10 * (1.0f - m_progress));
            }
            if (result.field_0x14 != m_initial0.field_0x14) {
                result.field_0x14 = static_cast<int>(result.field_0x14 * m_progress +
                                                     m_initial0.field_0x14 * (1.0f - m_progress));
            }
            if (result.field_0x18 != m_initial0.field_0x18) {
                result.field_0x18 = static_cast<int>(result.field_0x18 * m_progress +
                                                     m_initial0.field_0x18 * (1.0f - m_progress));
            }
            if (result.field_0x1C != m_initial0.field_0x1C) {
                result.field_0x1C = static_cast<int>(result.field_0x1C * m_progress +
                                                     m_initial0.field_0x1C * (1.0f - m_progress));
            }
        }
        manager->fn_804308C0(result);
        break;
    }
    case 1: {
        UnkEffectRW3Param1 result = m_target1;
        if (m_progress < 0.9999f) {
            if (result.field_0x0 || m_initial1.field_0x0) {
                result.field_0x0 = 1;
            } else {
                result.field_0x0 = 0;
            }
            if (result.field_0x1 != m_initial1.field_0x1) {
                result.field_0x1 = static_cast<int>(result.field_0x1 * m_progress +
                                                    m_initial1.field_0x1 * (1.0f - m_progress));
            }
            result.field_0x4 =
                result.field_0x4 * m_progress + m_initial1.field_0x4 * (1.0f - m_progress);
            result.field_0x8[0] =
                result.field_0x8[0] * m_progress + m_initial1.field_0x8[0] * (1.0f - m_progress);
            result.field_0x8[1] =
                result.field_0x8[1] * m_progress + m_initial1.field_0x8[1] * (1.0f - m_progress);
        }
        manager->fn_8042E098(result);
        break;
    }
    case 2: {
        UnkEffectRW3Param2 result = m_target2;
        if (m_progress < 0.9999f) {
            if (result.field_0x0 || m_initial2.field_0x0) {
                result.field_0x0 = 1;
            } else {
                result.field_0x0 = 0;
            }
            if (result.field_0x3.red != m_initial2.field_0x3.red) {
                result.field_0x3.red =
                    static_cast<int>(result.field_0x3.red * m_progress +
                                     m_initial2.field_0x3.red * (1.0f - m_progress));
            }
            if (result.field_0x3.green != m_initial2.field_0x3.green) {
                result.field_0x3.green =
                    static_cast<int>(result.field_0x3.green * m_progress +
                                     m_initial2.field_0x3.green * (1.0f - m_progress));
            }
            if (result.field_0x3.blue != m_initial2.field_0x3.blue) {
                result.field_0x3.blue =
                    static_cast<int>(result.field_0x3.blue * m_progress +
                                     m_initial2.field_0x3.blue * (1.0f - m_progress));
            }
            if (result.field_0x3.alpha != m_initial2.field_0x3.alpha) {
                result.field_0x3.alpha =
                    static_cast<int>(result.field_0x3.alpha * m_progress +
                                     m_initial2.field_0x3.alpha * (1.0f - m_progress));
            }
            if (result.field_0x7.red != m_initial2.field_0x7.red) {
                result.field_0x7.red =
                    static_cast<int>(result.field_0x7.red * m_progress +
                                     m_initial2.field_0x7.red * (1.0f - m_progress));
            }
            if (result.field_0x7.green != m_initial2.field_0x7.green) {
                result.field_0x7.green =
                    static_cast<int>(result.field_0x7.green * m_progress +
                                     m_initial2.field_0x7.green * (1.0f - m_progress));
            }
            if (result.field_0x7.blue != m_initial2.field_0x7.blue) {
                result.field_0x7.blue =
                    static_cast<int>(result.field_0x7.blue * m_progress +
                                     m_initial2.field_0x7.blue * (1.0f - m_progress));
            }
            if (result.field_0x7.alpha != m_initial2.field_0x7.alpha) {
                result.field_0x7.alpha =
                    static_cast<int>(result.field_0x7.alpha * m_progress +
                                     m_initial2.field_0x7.alpha * (1.0f - m_progress));
            }
        }
        manager->fn_8043891C(result);
        break;
    }
    case 3: {
        UnkEffectRW3Param3 result = m_target3;
        if (m_progress < 0.9999f) {
            if (result.field_0x0 || m_initial3.field_0x0) {
                result.field_0x0 = 1;
            } else {
                result.field_0x0 = 0;
            }
            result.field_0x8.field_0x4 = result.field_0x8.field_0x4 * m_progress +
                                         m_initial3.field_0x8.field_0x4 * (1.0f - m_progress);
            result.field_0x8.field_0x8 = result.field_0x8.field_0x8 * m_progress +
                                         m_initial3.field_0x8.field_0x8 * (1.0f - m_progress);
            result.field_0x8.field_0xC = result.field_0x8.field_0xC * m_progress +
                                         m_initial3.field_0x8.field_0xC * (1.0f - m_progress);
            result.field_0x8.field_0x10 = result.field_0x8.field_0x10 * m_progress +
                                          m_initial3.field_0x8.field_0x10 * (1.0f - m_progress);
            result.field_0x8.field_0x14 = result.field_0x8.field_0x14 * m_progress +
                                          m_initial3.field_0x8.field_0x14 * (1.0f - m_progress);
            result.field_0x20.field_0x4 = result.field_0x20.field_0x4 * m_progress +
                                          m_initial3.field_0x20.field_0x4 * (1.0f - m_progress);
            result.field_0x20.field_0x8 = result.field_0x20.field_0x8 * m_progress +
                                          m_initial3.field_0x20.field_0x8 * (1.0f - m_progress);
            result.field_0x20.field_0xC = result.field_0x20.field_0xC * m_progress +
                                          m_initial3.field_0x20.field_0xC * (1.0f - m_progress);
            result.field_0x20.field_0x10 = result.field_0x20.field_0x10 * m_progress +
                                           m_initial3.field_0x20.field_0x10 * (1.0f - m_progress);
            result.field_0x20.field_0x14 = result.field_0x20.field_0x14 * m_progress +
                                           m_initial3.field_0x20.field_0x14 * (1.0f - m_progress);
        }
        manager->fn_80436FC0(result);
        break;
    }
    }
}

} // namespace RW3
} // namespace Effect
