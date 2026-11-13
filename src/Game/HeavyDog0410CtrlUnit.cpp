/* The control units of the HEAVY_DOG_0410 boss other than its mode units (class names from RTTI):
 * HeavyDog0410CtrlUnit_Move, HeavyDog0410CtrlUnit_Mine and HeavyDog0410CtrlUnit_Dust. File name
 * inferred from the classes. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first). */
#include "Game/BossCtrlUnit.h"
#include "Game/CharaColliAttack.h"
#include "Game/CommonBomb.h"
#include "Game/EffectManager.h"
#include "Game/HeavyDog0410CtrlUnit.h"
#include "Game/HeavyDogCtrlUnit.h"
#include "Game/HeavyDogVarParam.h"
#include "Game/SetClumpWorld.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_80037A80.h"
#include "Game/fn_8003A840.h"
#include "Game/fn_80169350.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rpworld.h"
#include "rw/rwcore.h"

using Effect::System::EffectElement;
using Effect::System::EffectLaunchInfo;
using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

HeavyDog0410CtrlUnit_Move::HeavyDog0410CtrlUnit_Move(HeavyDog0410VarParam *param)
    : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
    field_0x4 = 2;
}

HeavyDog0410CtrlUnit_Move::~HeavyDog0410CtrlUnit_Move() {}

/* While HeavyDogVarParam::field_0x114 is zero, moves the boss for the step arg (behaviour from
 * the body; the meanings of the values are inferred): the acceleration field_0x128 becomes
 * (field_0xF8 - position) * m_params.field_0x0 - field_0x11C * m_params.field_0x4, the velocity
 * field_0x11C gains the acceleration times arg, the position of the boss gains the velocity times
 * arg, and field_0x118 grows by the length of the velocity times arg. While field_0x114 is not
 * zero, field_0x11C and field_0x128 are set to the zero vector instead. Either way field_0x108
 * gets the distance of the boss from field_0xF8, and the result is false. */
bool HeavyDog0410CtrlUnit_Move::Update(float arg) {
    if (!m_heavyDog0410Param->field_0x114) {
        Fn8040Vector3 difference(m_heavyDog0410Param->field_0xF8 -
                                 m_heavyDog0410Param->m_charInfo->m_position);
        m_heavyDog0410Param->field_0x128 = difference * m_heavyDog0410Param->m_params.field_0x0;
        SubtractVector(&m_heavyDog0410Param->field_0x128,
                       m_heavyDog0410Param->field_0x11C * m_heavyDog0410Param->m_params.field_0x4);
        AddVector(&m_heavyDog0410Param->field_0x11C, m_heavyDog0410Param->field_0x128 * arg);
        AddVector(&m_heavyDog0410Param->m_charInfo->m_position,
                  m_heavyDog0410Param->field_0x11C * arg);
        m_heavyDog0410Param->field_0x118 += fn_8040CF50(m_heavyDog0410Param->field_0x11C * arg);
    } else {
        m_heavyDog0410Param->field_0x11C = lbl_805E07A0;
        m_heavyDog0410Param->field_0x128 = lbl_805E07A0;
    }

    m_heavyDog0410Param->field_0x108 =
        fn_8040CF50(m_heavyDog0410Param->field_0xF8 - m_heavyDog0410Param->m_charInfo->m_position);
    return false;
}

/* Clones the mine's two models (resources 8 and 9 of resource key 0x16; a model whose resource
 * is missing is not cloned), puts each clone in the world that the table at 0x8051D208 has for
 * the index fn_80169350 gives for it, and moves the clone's frame to position when the clone has
 * one. It then creates the mine's own character info and a probe sphere of radius 0.25 at
 * position with that character info as its owner. When there is a probe, it sets the probe's
 * byte at 0x2C and asks for the probe's attack object; when that object still exists, its second
 * callback becomes the binding of fn_8023357C. Last it creates the Target (behaviour from the
 * body; the meanings of the numbers passed to the factories are unknown). */
HeavyDog0410CtrlUnit_Mine::HeavyDog0410CtrlUnit_Mine(HeavyDog0410VarParam *param,
                                                     const Fn8040Vector3 *position,
                                                     const Fn8040Vector3 *velocity,
                                                     const Fn8040Vector3 *destination)
    : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
    m_position = *position;
    m_destination = *destination;
    m_velocity = *velocity;
    m_state = 0;
    m_time = 0.0f;

    RpClump *source = static_cast<RpClump *>(fn_8001277C(fn_800122D4(), 0x16, 8));
    if (source != 0) {
        m_clumps[0] = fn_80457090(source);
    }
    source = static_cast<RpClump *>(fn_8001277C(fn_800122D4(), 0x16, 9));
    if (source != 0) {
        m_clumps[1] = fn_80457090(source);
    }

    for (int i = 0; i < 2; i++) {
        if (m_clumps[i] != 0) {
            int index = fn_80169350(fn_80169364(m_clumps[i]));
            RpWorld *world = fn_800593D0(fn_8003A840(), index);
            SetClumpWorld(world, m_clumps[i]);

            m_frames[i] = static_cast<RwFrame *>(m_clumps[i]->object.parent);
            if (m_frames[i] != 0) {
                RwV3d *translation = &m_frames[i]->modelling.pos;
                translation->x = m_position.x;
                translation->y = m_position.y;
                translation->z = m_position.z;
                fn_8048722C(m_frames[i]);
            }
        }
    }

    m_charInfo = boost::shared_ptr<BossCharInfo>(new BossCharInfo(0xA9));

    UnkWorld_80021B08 *probeWorld = fn_80021B08();
    m_probe = fn_800C7EA8(probeWorld, &m_position, 0x80, 4, 0,
                          boost::weak_ptr<CharInfo>(m_charInfo), &lbl_805E07A0, 0.25f);
    if (m_probe) {
        m_probe->SetField0x2C(1);

        boost::weak_ptr<CharaColliAttack> attackWeak(fn_800C4374(probeWorld, m_probe));
        boost::shared_ptr<CharaColliAttack> attack(attackWeak.lock());
        if (attack) {
            attack->field_0x54 = 0;
            attack->field_0x3C = 1;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 = fn_80335A80();
        }
    }

    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x2C00);
}

/* Takes each clone out of its world and destroys it, and releases the Target, the probe and the
 * character info, in that order. */
HeavyDog0410CtrlUnit_Mine::~HeavyDog0410CtrlUnit_Mine() {
    for (int i = 0; i < 2; i++) {
        if (m_clumps[i] != 0) {
            RpWorld *world = fn_8046256C(m_clumps[i]);
            if (world != 0) {
                fn_804623EC(world, m_clumps[i]);
            }
            fn_80457468(m_clumps[i]);
            m_clumps[i] = 0;
            m_frames[i] = 0;
        }
    }

    m_target.reset();
    m_probe.reset();
    m_charInfo.reset();
}

/* Adds arg to m_time and runs the state (behaviour from the body; what the states stand for is
 * inferred from it):
 * - 0: becomes state 2 and clears m_time once m_time is above HeavyDog0410VarParam::field_0x1E8.
 * - 1: releases the Target when there is one and continues as state 2; the state stays 1 until
 *   that changes it.
 * - 2: becomes state 3 and clears m_time once m_time is above field_0x1EC. Until then the colour
 *   for the models gets alpha 0 while the number of whole tenths in m_time is even, and sound
 *   0x7D38 is passed to fn_801D54F4 at the mine's position in the update where that number turns
 *   even.
 * - 3: requests a CommonBomb at the mine's position (fn_801DD0BC with 0, 1 and 1.0), releases the
 *   Target and the probe when they exist and becomes state 4. This change does not clear m_time.
 * - 4: the colour gets alpha 0. Once m_time is above field_0x1F0 the function returns true at
 *   once, without the steps below; until then the mine does not move.
 * The colour, white with alpha 255 unless the state changed it, is then passed to fn_80037B84
 * for each model that exists.
 * Unless the state was 4 at the start of the call, the mine then moves: the acceleration is
 * (m_destination - m_position) * field_0x1F4 - m_velocity * field_0x1F8, m_velocity gains the
 * acceleration times arg and m_position gains m_velocity times arg. The update that leaves state
 * 3 still moves the mine.
 * Last each frame that exists, the Target when there is one and the probe when there is one are
 * moved to m_position, and the result is false. */
bool HeavyDog0410CtrlUnit_Mine::Update(float arg) {
    m_time += arg;

    bool stopped = false;
    RwRGBA color = {255, 255, 255, 255};

    switch (m_state) {
    case 0:
        if (m_time > m_heavyDog0410Param->field_0x1E8) {
            m_state = 2;
            m_time = 0.0f;
        }
        break;
    case 1:
        if (m_target) {
            m_target.reset();
        }
        /* fallthrough */
    case 2:
        if (m_time > m_heavyDog0410Param->field_0x1EC) {
            m_state = 3;
            m_time = 0.0f;
        } else if (static_cast<int>(m_time / 0.1f) % 2 == 0) {
            if (static_cast<int>((m_time - arg) / 0.1f) % 2 != 0) {
                fn_801D54F4(fn_80053164(), 0x7D38, &m_position, 0, 1);
            }
            color.alpha = 0;
        }
        break;
    case 3:
        fn_801DD0BC(&m_position, 0, 1, 1.0f);
        if (m_target) {
            m_target.reset();
        }
        if (m_probe) {
            m_probe.reset();
        }
        m_state = 4;
        break;
    case 4:
        color.alpha = 0;
        if (m_time > m_heavyDog0410Param->field_0x1F0) {
            return true;
        }
        stopped = true;
        break;
    }

    /* One counter for both loops, as the original's register use shows: with a counter declared
     * in each for statement the second loop's counter takes the first one's register. */
    int i;
    for (i = 0; i < 2; i++) {
        if (m_clumps[i] != 0) {
            fn_80037B84(m_clumps[i], &color);
        }
    }

    if (!stopped) {
        Fn8040Vector3 difference(m_destination - m_position);
        Fn8040Vector3 acceleration(difference * m_heavyDog0410Param->field_0x1F4);
        SubtractVector(&acceleration, m_velocity * m_heavyDog0410Param->field_0x1F8);
        AddVector(&m_velocity, acceleration * arg);
        AddVector(&m_position, m_velocity * arg);
    }

    for (i = 0; i < 2; i++) {
        if (m_frames[i] != 0) {
            RwV3d *translation = &m_frames[i]->modelling.pos;
            translation->x = m_position.x;
            translation->y = m_position.y;
            translation->z = m_position.z;
            fn_8048722C(m_frames[i]);
        }
    }

    if (m_target) {
        fn_800CCE38(m_target.get(), m_position);
    }
    if (m_probe) {
        m_probe->SetPosition(&m_position);
    }
    return false;
}

/* The second callback of the attack object of m_probe. Stores 1 in param.field_0x54 and reads
 * the word at 0x40 of the attack object that param.field_0x0 refers to: with bit 3 set, state 0
 * becomes state 1 and every other state becomes state 3; otherwise the state becomes 3 when the
 * low three bits are all clear. m_time is cleared in every case (behaviour from the body; the
 * meanings of the bits are unknown). The attack object is used without a check that it still
 * exists. */
void HeavyDog0410CtrlUnit_Mine::fn_8023357C(AttackCallbackParam &param) {
    param.field_0x54 = 1;

    boost::shared_ptr<CharaColliAttack> attack(param.field_0x0.lock());
    u32 flags = attack->field_0x40;
    if (flags & 8) {
        if (m_state == 0) {
            m_state = 1;
        } else {
            m_state = 3;
        }
    } else if ((flags & 7) == 0) {
        m_state = 3;
    }
    m_time = 0.0f;
}

HeavyDog0410CtrlUnit_Dust::HeavyDog0410CtrlUnit_Dust(HeavyDog0410VarParam *param)
    : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
    field_0x4 = 3;
    m_effect = fn_800A31F0()->Create(
        boost::shared_ptr<EffectLaunchInfo>(m_heavyDog0410Param->field_0x6C), 0x16);

    boost::shared_ptr<EffectElement> effect(m_effect.lock());
    if (effect) {
        effect->SetTranslationSource(&m_heavyDog0410Param->m_charInfo->m_position);
        effect->SetTimeScaleSource(&lbl_8057D6A0.field_0x24);
    }
}

HeavyDog0410CtrlUnit_Dust::~HeavyDog0410CtrlUnit_Dust() {
    boost::shared_ptr<EffectElement> effect(m_effect.lock());
    if (effect) {
        effect->RequestStop();
        effect.reset();
    }
}

bool HeavyDog0410CtrlUnit_Dust::Update(float arg) {
    fn_801D53C4(fn_80053164(), 0x7D3C, &m_heavyDog0410Param->m_charInfo->m_position, this, 3, 0, 3);
    return false;
}
