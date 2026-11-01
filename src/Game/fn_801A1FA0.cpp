/* Registration and counting methods; original filename and owning class name unknown. */
#include "Game/fn_801A1FA0.h"
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/GunSoldierStateData.h"
#include "Game/PlayerCommands.h"
#include "Game/ResourceOneFile.h"
#include "Game/SBkLarvaStateData.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_8006F754.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkResource_80012700.h"
#include <cstring>

GadgetTypeInfo lbl_8053A9A8 = {
    "EnemyEntryStart", fn_801A1FD0, fn_801A1FA8, 0, 0, 0x80, 0x258E, 2, 0, 0, 0,
};
GadgetTypeInfo lbl_8053A9CC = {
    "EnemyEntryEnd", fn_801A1FA4, fn_801A1FA0, 0, 0, 0x80, 0x258E, 2, 0, 0, 0,
};

Unk801A2AF4::Unk801A2AF4() {
    fn_801A2A68();
}

void Unk801A2AF4::fn_801A2A68() {
    m_enemies.clear();
    m_counts.clear();
    m_total = 0;
    std::memset(m_totals, 0, sizeof m_totals);
    std::memset(m_current, 0, sizeof m_current);
    std::memset(m_savedCurrent, 0, sizeof m_savedCurrent);
    std::memset(m_byPlayer, 0, sizeof m_byPlayer);
    std::memset(m_savedByPlayer, 0, sizeof m_savedByPlayer);
}

int Unk801A2AF4::fn_801A29B0(Enemy *enemy) {
    bool found = false;
    int count = m_registrationCount++;
    int id;
    for (id = 0; id < count; ++id) {
        if (m_enemies[id] == 0) {
            m_enemies[id] = enemy;
            found = true;
            break;
        }
    }
    if (!found) {
        id = m_registrationCount;
        m_enemies[id] = enemy;
    }
    return id;
}

void Unk801A2AF4::fn_801A2968(int id) {
    m_enemies[id] = 0;
    --m_registrationCount;
}

void Unk801A2AF4::fn_801A2940() {
    m_registrationCount = 0;
    fn_801A27E4();
}

void Unk801A2AF4::fn_801A27E4() {
    fn_801A2A68();
    for (int id = 0; id < 256; ++id) {
        UnkGadgetOwner_800C9CD0 *owner = fn_800CB560(fn_800C9B84(), id);
        int count = 0;
        while (owner != 0) {
            unsigned short kind = fn_800C9D24(owner);
            if (kind >= 100 && kind < 150) {
                if (kind == 100) {
                    UnkParameters_8018ED00 parameters;
                    fn_800C9DBC(owner, &parameters, sizeof parameters);
                    if (parameters.field_0x24 != 5) {
                        ++count;
                        ++m_totals[0];
                    }
                } else if (kind == 145) {
                    UnkParameters_8018A3D4 parameters;
                    fn_800C9DBC(owner, &parameters, sizeof parameters);
                    m_totals[2] += parameters.field_0x1C;
                    count += parameters.field_0x1C;
                } else {
                    int category;
                    if (kind < 120) {
                        category = 0;
                    } else if (kind < 140) {
                        category = 1;
                    } else {
                        category = 2;
                    }
                    ++count;
                    ++m_totals[category];
                }
            }
            owner = owner->m_next;
        }
        if (count != 0) {
            m_counts[id] = count;
            m_total += count;
        }
    }
}

void Unk801A2AF4::fn_801A2674() {
    m_counts.clear();
    for (int id = 0; id < 256; ++id) {
        UnkGadgetOwner_800C9CD0 *owner = fn_800CB560(fn_800C9B84(), id);
        int count = 0;
        while (owner != 0) {
            unsigned short kind = fn_800C9D24(owner);
            if (kind >= 100 && kind < 150) {
                if (kind == 100) {
                    if (owner->m_adapterState != 0) {
                        GunSoldier::STemporaryData *state =
                            dynamic_cast<GunSoldier::STemporaryData *>(owner->m_adapterState);
                        if (state != 0 && state->field_0x4 != 1) {
                            ++count;
                        }
                    } else {
                        UnkParameters_8018ED00 parameters;
                        fn_800C9DBC(owner, &parameters, sizeof parameters);
                        if (parameters.field_0x24 != 5) {
                            ++count;
                        }
                    }
                } else if (kind == 145) {
                    if (owner->m_adapterState != 0) {
                        SBkLarvaStateData *state =
                            dynamic_cast<SBkLarvaStateData *>(owner->m_adapterState);
                        if (state != 0) {
                            count += state->field_0x4;
                        }
                    } else {
                        UnkParameters_8018A3D4 parameters;
                        fn_800C9DBC(owner, &parameters, sizeof parameters);
                        count += parameters.field_0x1C;
                    }
                } else {
                    ++count;
                }
            }
            owner = owner->m_next;
        }
        if (count != 0) {
            m_counts[id] = count;
        }
    }
}

void Unk801A2AF4::fn_801A2574(unsigned int flags, int category) {
    ++m_current[category];
    if ((flags & 4) != 0) {
        ++m_byPlayer[category];
        UnkManager_8006F754 *manager = fn_8006F754();
        boost::weak_ptr<CharInfo> target = fn_8007488C(manager, fn_800747E0(manager));
        boost::shared_ptr<CharInfo> info = target.lock();
        if (info.get() != 0) {
            info->fn_801DC980(
                new Player::Npc::EnemyDeadByPlayerCommand(category, m_byPlayer[category]));
        }
    }
}

void Unk801A2AF4::fn_801A2540() {
    fn_801A2A68();
    m_registrationCount = 0;
}

void Unk801A2AF4::fn_801A2520() {
    fn_801A27E4();
}

void Unk801A2AF4::fn_801A24EC() {
    fn_801A22FC();
    fn_801A2674();
}

void Unk801A2AF4::fn_801A23F4() {
    for (int i = 0; i < 3; ++i) {
        m_savedCurrent[i] = m_current[i];
        m_savedByPlayer[i] = m_byPlayer[i];
    }
    for (int id = 0; id < 256; ++id) {
        UnkGadgetOwner_800C9CD0 *owner = fn_800CB560(fn_800C9B84(), id);
        while (owner != 0) {
            unsigned short kind = fn_800C9D24(owner);
            SetAdapterState *base = owner->m_adapterState;
            if (base != 0) {
                if (kind == 100) {
                    GunSoldier::STemporaryData *state =
                        dynamic_cast<GunSoldier::STemporaryData *>(base);
                    if (state != 0) {
                        state->field_0x8 = state->field_0x4;
                    }
                } else if (kind == 145) {
                    SBkLarvaStateData *state = dynamic_cast<SBkLarvaStateData *>(base);
                    if (state != 0) {
                        state->field_0x8 = state->field_0x4;
                    }
                }
            }
            owner = owner->m_next;
        }
    }
}

void Unk801A2AF4::fn_801A22FC() {
    for (int i = 0; i < 3; ++i) {
        m_current[i] = m_savedCurrent[i];
        m_byPlayer[i] = m_savedByPlayer[i];
    }
    for (int id = 0; id < 256; ++id) {
        UnkGadgetOwner_800C9CD0 *owner = fn_800CB560(fn_800C9B84(), id);
        while (owner != 0) {
            unsigned short kind = fn_800C9D24(owner);
            SetAdapterState *base = owner->m_adapterState;
            if (base != 0) {
                if (kind == 100) {
                    GunSoldier::STemporaryData *state =
                        dynamic_cast<GunSoldier::STemporaryData *>(base);
                    if (state != 0) {
                        state->field_0x4 = state->field_0x8;
                    }
                } else if (kind == 145) {
                    SBkLarvaStateData *state = dynamic_cast<SBkLarvaStateData *>(base);
                    if (state != 0) {
                        state->field_0x4 = state->field_0x8;
                    }
                }
            }
            owner = owner->m_next;
        }
    }
}

void Unk801A2AF4::fn_801A2150(UnkGadgetOwner_800C9CD0 *owner, int category, unsigned int flags) {
    if (flags == 0) {
        return;
    }
    unsigned char id = 0;
    if (owner == 0) {
        return;
    }
    fn_800C9E60(owner, &id);
    CountMap::iterator it = m_counts.find(id);
    if (it != m_counts.end()) {
        // The original queries the kind here even though its result is not used.
        fn_800C9D24(owner);
        if ((flags & 2) != 0) {
            --m_current[category];
            ++m_counts[id];
        } else if ((flags & 1) != 0) {
            fn_801A2574(flags, category);
            if (--m_counts[id] <= 0) {
                fn_8016907C(fn_800CB69C(), id, 1);
            }
        }
    }
}

int Unk801A2AF4::fn_801A2140(int category) {
    return m_current[category];
}

int fn_801A2064() {
    switch (lbl_8057D6A0.field_0xA8) {
    case 500:
    case 503:
        return 0;
    case 401:
    case 504:
        return 1;
    case 100:
    case 202:
    case 400:
        return 2;
    case 404:
        return 4;
    case 501:
    case 600:
    case 800:
    case 801:
        return 6;
    case 201:
        return 7;
    default:
        return 0;
    }
}

bool fn_801A2030() {
    if (lbl_8057D6A0.field_0xA8 == 300 || lbl_8057D6A0.field_0xA8 == 402 ||
        lbl_8057D6A0.field_0xA8 == 403) {
        return true;
    }
    return false;
}

void fn_801A1FD0() {
    UnkResourceManager_800122D4 *manager = fn_800122D4();
    fn_8001299C(manager, 6, new ResourceOneFile("enemy/EnemyPath.one", false));
}

void fn_801A1FA8() {
    fn_800127F8(fn_800122D4(), 6);
}

void fn_801A1FA4() {}

void fn_801A1FA0() {}
