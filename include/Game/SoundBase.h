#ifndef GAME_SOUNDBASE_H
#define GAME_SOUNDBASE_H

#include "Game/Task.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Player {
class PlayerBase;
}

/* One entry of a sound table list (placeholder name; 0x18 bytes, the stride with which the lists
 * are indexed). Player::SoundBase reads the fields at 0x0 and 0xC-0x14 (0x802DBBE0, 0x802DBC8C);
 * neither those functions nor Player::Npc::Sound read 0x4 or 0x8. */
struct UnkSoundEntry_805020F8 {
    /* An entry is used only when this byte is not zero (0 or 1 in the tables). */
    /* 0x00 */ u8 field_0x0;
    /* 0x04 */ u32 field_0x4;
    /* 0x08 */ u32 field_0x8;
    /* When positive, a value taken from the counter Player::SoundBase::field_0x30 is added to the
     * sound id: the counter modulo this value at 0x802DBBE0, the counter ANDed with it at
     * 0x802DBC8C. */
    /* 0x0C */ int field_0xC;
    /* The sound id; no sound is started when it is negative. */
    /* 0x10 */ int field_0x10;
    /* Passed to fn_801D51A0 at 0x802DBBE0 when it is not negative. */
    /* 0x14 */ int field_0x14;
};
CHECK_SIZEOF(UnkSoundEntry_805020F8, 0x18);

/* A sound table (placeholder name): two lists of entries. The tables are read-only data in
 * .sdata2 (0x805F8F90-0x805F8FF8) and their lists are in .rodata. */
struct UnkSoundTable_805F8F90 {
    /* 0x0 */ const UnkSoundEntry_805020F8 *field_0x0;
    /* 0x4 */ const UnkSoundEntry_805020F8 *field_0x4;
};
CHECK_SIZEOF(UnkSoundTable_805F8F90, 0x8);

namespace Player {

/* Player::SoundBase (class name from RTTI; base Task): the base of a character's sound task.
 * Player::Npc::Sound and Player::Shadow::Sound (class names and bases from RTTI) implement the
 * three pure slots with their own sound tables. The constructor (0x802DBDF0) stores the class name
 * "Sound", copies the player, stores the new task in the word at 0x240 of the player and clears
 * field_0x30; the destructor (0x802DBD7C) clears that word of the player again. The pure slots are
 * named after their Player::Npc::Sound implementations; their purpose is inferred from those. The
 * size is 0x38: Task's 64-bit member gives the class 8-byte alignment, and both subclasses start
 * their own members at 0x38. */
class SoundBase : public Task {
  public:
    SoundBase(Task *parent, const boost::shared_ptr<PlayerBase> &player);
    virtual ~SoundBase();
    /* Pure; the subclasses differ: Player::Shadow::Sound's implementations (USA (GUPE8P)
     * 0x802DBA38 and 0x802DB8B4) handle some indices specially, e.g. index 16 plays sound 1 or 2
     * without its table entry. The descriptions below are Player::Npc::Sound's. Npc: plays entry
     * index of its table's first list when that entry is used (the index and id offset types and
     * the void return are inferred). */
    virtual void fn_80305CE4(int index, int idOffset) = 0;
    /* Pure. Npc: plays entry index of its table's second list when that entry is used (the index
     * type and the void return are inferred). */
    virtual void fn_80305CA8(int index) = 0;
    /* Pure. Npc: through fn_802DBC8C, starts the sound of entry index of its table's second list
     * and returns its handle when that entry is used and its sound id is not negative, otherwise
     * returns a default-constructed handle (the index and owner types are inferred; callers pass
     * themselves as owner). */
    virtual UnkSoundHandle_801D3DC8 fn_80305C50(int index, void *owner) = 0;

  protected:
    /* When entry index of the table's second list is used and its sound id is not negative,
     * starts that sound through fn_801D3EF0 with owner and returns its handle; otherwise returns a
     * default-constructed handle (behaviour inferred from its body). */
    UnkSoundHandle_801D3DC8 fn_802DBC8C(const UnkSoundTable_805F8F90 *table, int index,
                                        void *owner);
    /* Play entry index of the table's second list, with the id offset 0, and of its first list,
     * with idOffset (behaviour inferred from their bodies, through 0x802DBBE0; the void returns
     * are inferred: no caller reads r3). */
    void fn_802DBD1C(const UnkSoundTable_805F8F90 *table, int index);
    void fn_802DBD4C(const UnkSoundTable_805F8F90 *table, int index, int idOffset);

    /* 0x28 */ boost::shared_ptr<PlayerBase> m_player;
    /* Cleared by the constructor. 0x802DBBE0 and 0x802DBC8C increment it for a used entry with a
     * sound id that is not negative and a positive field_0xC, before they start the sound (the
     * increment does not depend on what the start returns; behaviour inferred from their
     * bodies). */
    /* 0x30 */ int field_0x30;

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered: the constructor stores the task in the word at 0x240 of the player and the
     * destructor clears that word, so a memberwise copy would leave the player pointing at the
     * original and either destructor would clear the other's registration. Declared and not
     * defined. */
    SoundBase(const SoundBase &);
    SoundBase &operator=(const SoundBase &);
};
CHECK_SIZEOF(SoundBase, 0x38);

} // namespace Player

#endif
