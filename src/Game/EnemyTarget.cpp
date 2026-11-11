/* EnemyTarget (class name from the RTTI name strings of its shared pointer's counter): the Target
 * objects of an enemy. File name inferred from the class. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). */
#include "Game/EnemyTarget.h"
#include "Game/CharInfo.h"
#include "Game/Enemy.h"
#include "Game/EnemyCharInfo.h"
#include "Game/EnemyDispFunc.h"
#include "Game/EnemyParaFunc.h"
#include "Game/Target.h"
#include "Game/UnkEnemyParaData_8017B8E4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

boost::shared_ptr<EnemyTarget> EnemyTarget::Create(Enemy *enemy,
                                                   const UnkEnemyParaEntry_801A6230 *entries) {
    /* The original builds a temporary, copies it to a second object and copies that to the
     * result, so the result is returned through a copy-initialised local. */
    boost::shared_ptr<EnemyTarget> target =
        boost::shared_ptr<EnemyTarget>(new EnemyTarget(enemy, entries));

    return target;
}

EnemyTarget::EnemyTarget(Enemy *enemy, const UnkEnemyParaEntry_801A6230 *entries)
    : m_enemy(enemy), m_entries(entries), m_count(0) {
    /* The bit that the value of the parameter interface's slot 0x8 selects; other values select
     * none. The meaning of the values and of the bits is unknown. The cases are in the order of
     * the original's code; in numeric order the code for 1 and 2 is placed the other way round. */
    unsigned int flags = 0;

    switch (m_enemy->m_parameter->fn_8019B464()) {
    case 0:
        flags |= 0x10000;
        break;
    case 2:
        flags |= 0x20000;
        break;
    case 1:
        flags |= 0x40000;
        break;
    }

    /* An object keeps its record's index, and m_count counts the objects created. */
    for (int i = 0; i < kTargetCount; i++) {
        if (m_entries[i].field_0x0 >= 0) {
            m_targets[i] = fn_801E7128(boost::weak_ptr<CharInfo>(fn_8017D014(m_enemy)),
                                       flags | m_entries[i].field_0x4);
            m_targets[i]->SetFlags(1);
            m_count++;
        }
    }

    /* Japan (GUPJ8P) and PAL (GUPP8P) end the constructor with this call; USA (GUPE8P) has no
     * call here. */
#if !defined(VERSION_GUPE8P)
    UpdatePositions();
#endif
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here (without the pragma checked_delete<EnemyTarget> is
 * expanded in checked_deleter<EnemyTarget>::operator()). A tooling constraint, not a claim about
 * the original source; revisit if a destructor called from checked_delete turns out to be inlined
 * in the original. */
#pragma auto_inline on
EnemyTarget::~EnemyTarget() {
    /* Resets the slots from m_count - 1 down to 0, before the array's destructors run. */
    for (int i = m_count - 1; i >= 0; i--) {
        m_targets[i].reset();
    }
}
#pragma auto_inline reset

void EnemyTarget::UpdatePositions() {
    for (int i = 0; i < m_count; i++) {
        boost::shared_ptr<Target> target = m_targets[i];

        if (target.get() != 0 && target->TestFlags(1)) {
            Fn8040Matrix34 *matrix = m_enemy->m_display->fn_8019448C(m_entries[i].field_0x0);

            if (matrix != 0) {
                fn_800CCE38(target.get(), fn_8040FB08(matrix));
            }
        }
    }
}

void EnemyTarget::fn_803199D0() {
    for (int i = 0; i < m_count; i++) {
        if (m_targets[i].get() != 0) {
            m_targets[i]->SetFlags(1);
        }
    }
}

void EnemyTarget::fn_80319964() {
    for (int i = 0; i < m_count; i++) {
        if (m_targets[i].get() != 0) {
            m_targets[i]->ClearFlags(1);
        }
    }
}

boost::shared_ptr<Target> EnemyTarget::GetTarget(int index) {
    if (index >= 0 && index < m_count) {
        return m_targets[index];
    }

    return boost::shared_ptr<Target>();
}
