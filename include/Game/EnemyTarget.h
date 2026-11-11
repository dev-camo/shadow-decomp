#ifndef GAME_ENEMYTARGET_H
#define GAME_ENEMYTARGET_H

#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class Enemy;
class Target;
struct UnkEnemyParaEntry_801A6230;

/* EnemyTarget (class name from the RTTI name strings of its shared pointer's counter; the class
 * has no vtable): the Target objects of one enemy, one for each of the first two records that slot
 * 0x74 of the enemy's parameter interface returns. A record whose first word is negative gets no
 * object. Create allocates 0x1C bytes for it. The member names are inferred. */
class EnemyTarget {
  public:
    /* The number of records read and of object slots (name inferred). */
    static const int kTargetCount = 2;

    EnemyTarget(Enemy *enemy, const UnkEnemyParaEntry_801A6230 *entries);
    ~EnemyTarget();

    /* Creates the object in a shared pointer (name inferred; that it is a static member is
     * inferred too). */
    static boost::shared_ptr<EnemyTarget> Create(Enemy *enemy,
                                                 const UnkEnemyParaEntry_801A6230 *entries);

    /* For each of the first m_count slots that holds an object with bit 0 of its flag word set:
     * asks slot 0x24 of the enemy's display interface for the matrix of the record's first word
     * and, when it returns one, copies that matrix's translation to the object's position (name
     * inferred). */
    void UpdatePositions();
    /* Sets bit 0 of the flag word of each object in the first m_count slots. */
    void fn_803199D0();
    /* Clears bit 0 of the flag word of each object in the first m_count slots. */
    void fn_80319964();
    /* Returns m_targets[index], or an empty pointer when index is negative or not below m_count
     * (name inferred). */
    boost::shared_ptr<Target> GetTarget(int index);

    /* 0x00 */ Enemy *m_enemy;
    /* 0x04 */ const UnkEnemyParaEntry_801A6230 *m_entries;
    /* 0x08 */ boost::shared_ptr<Target> m_targets[kTargetCount];
    /* 0x18 */ int m_count; /* The number of objects created. */
};
CHECK_SIZEOF(EnemyTarget, 0x1C);

#endif
