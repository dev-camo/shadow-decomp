#ifndef GAME_FN_801A1FA0_H
#define GAME_FN_801A1FA0_H

#include <map>
#include "layout_check.h"

class Enemy;
class UnkGadgetOwner_800C9CD0;

/* Name unknown; fields inferred. This maps a 0x64-byte prefix; complete extent is unknown. */
class Unk801A2AF4 {
  public:
    typedef std::map<int, Enemy *> EnemyMap;
    typedef std::map<short, short> CountMap;

    Unk801A2AF4();
    int fn_801A2140(int category);
    void fn_801A2150(UnkGadgetOwner_800C9CD0 *owner, int category, unsigned int flags);
    void fn_801A22FC();
    void fn_801A23F4();
    void fn_801A24EC();
    void fn_801A2520();
    void fn_801A2540();
    void fn_801A2574(unsigned int flags, int category);
    void fn_801A2674();
    void fn_801A27E4();
    void fn_801A2940();
    void fn_801A2968(int id);
    int fn_801A29B0(Enemy *enemy);
    void fn_801A2A68();

    /* 0x00 */ EnemyMap m_enemies;
    /* 0x10 */ CountMap m_counts;
    /* 0x20 */ int m_registrationCount;
    /* 0x24 */ int m_total;
    /* 0x28 */ int m_totals[3];
    /* 0x34 */ int m_current[3];
    /* 0x40 */ int m_byPlayer[3];
    /* 0x4C */ int m_savedCurrent[3];
    /* 0x58 */ int m_savedByPlayer[3];
};
CHECK_SIZEOF(Unk801A2AF4::EnemyMap, 0x10);
CHECK_SIZEOF(Unk801A2AF4::CountMap, 0x10);
CHECK_SIZEOF(Unk801A2AF4, 0x64); // Mapped prefix only.

extern "C" {
void fn_801A1FA0();
void fn_801A1FA4();
void fn_801A1FA8();
void fn_801A1FD0();
bool fn_801A2030();
int fn_801A2064();
}

#endif
