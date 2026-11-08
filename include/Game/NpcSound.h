#ifndef GAME_NPCSOUND_H
#define GAME_NPCSOUND_H

#include "Game/SoundBase.h"
#include "layout_check.h"

namespace Player {
namespace Npc {

/* Player::Npc::Sound (class name from RTTI; bases Player::SoundBase and Task): a character's sound
 * task that plays the entries of the sound table it is created with ("Npc" read as non-player
 * character, inferred). */
class Sound : public SoundBase {
  public:
    Sound(Task *parent, const boost::shared_ptr<PlayerBase> &player,
          const UnkSoundTable_805F8F90 *table)
        : SoundBase(parent, player), m_table(table) {}
    virtual ~Sound();
    virtual void fn_80305CE4(int index, int idOffset);
    virtual void fn_80305CA8(int index);
    virtual UnkSoundHandle_801D3DC8 fn_80305C50(int index, void *owner);

    /* Creates the sound task and returns it, or null when the allocation fails (the constructor
     * is skipped then). The name and the pointer type are inferred. */
    static SoundBase *Create(Task *parent, const boost::shared_ptr<PlayerBase> &player,
                             const UnkSoundTable_805F8F90 *table);

    /* 0x38 */ const UnkSoundTable_805F8F90 *m_table;
};
CHECK_SIZEOF(Sound, 0x40);

} // namespace Npc
} // namespace Player

#endif
