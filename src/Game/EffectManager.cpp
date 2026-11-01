/* EffectManager methods; class identity comes from RTTI, original filename unknown. */
#include "Game/EffectManager.h"
#include "Game/EffectLaunchInfo.h"
#include "Game/UnkEffectFileData.h"

namespace Effect {
namespace System {

boost::weak_ptr<EffectElement> EffectManager::Create(EffectLaunchInfo *info, int id) {
    boost::weak_ptr<EffectElement> result;
    field_0x28 = 0;
    if (info == 0) {
        field_0x28 = 1;
        return result;
    }
    if (!fn_80425860(info->field_0x4)) {
        return result;
    }

    UnkEffectElementData *definition = info->field_0x4->first;
    while (definition != 0) {
        if (definition->id == id) {
            break;
        }
        definition = definition->next;
    }
    if (definition == 0) {
        field_0x28 = 4;
        return result;
    }

    boost::shared_ptr<EffectElement> element(CreateElement(definition, info));
    if (element.get() == 0 || !element->HasFlags(ELEMENT_ACTIVE)) {
        field_0x28 = 5;
        return result;
    }
    if (m_updateTail.get() == 0) {
        m_updateHead = m_updateTail = element;
    } else {
        m_updateTail->m_updateNext = element;
        element->m_updatePrev = m_updateTail;
        m_updateTail = element;
    }
    if (m_renderTail.get() == 0) {
        m_renderHead = m_renderTail = element;
    } else {
        m_renderTail->m_renderNext = element;
        element->m_renderPrev = m_renderTail;
        m_renderTail = element;
    }
    return boost::weak_ptr<EffectElement>(element);
}

boost::weak_ptr<EffectElement>
EffectManager::Create(const boost::shared_ptr<EffectLaunchInfo> &info, int id) {
    return Create(info.get(), id);
}

void EffectManager::Clear() {
    boost::shared_ptr<EffectElement> current;
    boost::shared_ptr<EffectElement> next;

    current = m_updateHead;
    while (current.get() != 0) {
        next = current->m_updateNext;
        Remove(current);
        current = next;
    }
}

void EffectManager::Remove(boost::shared_ptr<EffectElement> element) {
    if (element->m_updateNext.get() != 0) {
        element->m_updateNext->m_updatePrev = element->m_updatePrev;
    }
    if (element->m_updatePrev.get() != 0) {
        element->m_updatePrev->m_updateNext = element->m_updateNext;
    }
    if (element == m_updateHead) {
        m_updateHead = element->m_updateNext;
    }
    if (element == m_updateTail) {
        m_updateTail = element->m_updatePrev;
    }
    if (element->m_renderNext.get() != 0) {
        element->m_renderNext->m_renderPrev = element->m_renderPrev;
    }
    if (element->m_renderPrev.get() != 0) {
        element->m_renderPrev->m_renderNext = element->m_renderNext;
    }
    if (element == m_renderHead) {
        m_renderHead = element->m_renderNext;
    }
    if (element == m_renderTail) {
        m_renderTail = element->m_renderPrev;
    }
}

void EffectManager::Update(float deltaTime) {
    boost::shared_ptr<EffectElement> current(m_updateHead);
    boost::shared_ptr<EffectElement> next;

    while (current.get() != 0) {
        next = current->m_updateNext;
        if (!current->HasFlags(ELEMENT_ACTIVE)) {
            Remove(current);
        }
        current = next;
    }
    if (field_0x25 != 0) {
        return;
    }
    current = m_updateHead;
    while (current.get() != 0) {
        if (current->m_timeScaleSource != 0) {
            current->m_timeScale = *current->m_timeScaleSource;
        }
        current->Update(deltaTime * current->m_timeScale);
        current->m_flags |= ELEMENT_FLAG_2;
        current = current->m_updateNext;
    }
}

void EffectManager::Render(int group) {
    boost::shared_ptr<EffectElement> current(m_renderHead);

    if (field_0x24 != 0) {
        while (current.get() != 0) {
            if (group == current->field_0x30) {
                if (!current->HasFlags(ELEMENT_FLAG_8)) {
                    current->SetFlags(ELEMENT_FLAG_8);
                    current->Render();
                    current->ClearFlags(ELEMENT_FLAG_8);
                } else {
                    current->Render();
                }
            }
            current = current->m_renderNext;
        }
    } else {
        while (current.get() != 0) {
            if (group == current->field_0x30) {
                current->Render();
            }
            current = current->m_renderNext;
        }
    }
}

} // namespace System
} // namespace Effect
