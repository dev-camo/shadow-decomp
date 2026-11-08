/* Player::Npc::Sound (class name from RTTI): a character's sound task driven by a sound table
 * ("Npc" read as non-player character, inferred). Each slot passes its table on to the
 * Player::SoundBase function that plays or starts the requested entry only when that entry is
 * used; otherwise the slot that returns a handle returns a default-constructed one. File name
 * inferred from the class. The methods below are in reverse address order (deferred inlining
 * emits functions last-defined first). */
#include "Game/NpcSound.h"

namespace Player {
namespace Npc {

Sound::~Sound() {}

void Sound::fn_80305CE4(int index, int idOffset) {
    if (m_table->field_0x0[index].field_0x0 != 0) {
        fn_802DBD4C(m_table, index, idOffset);
    }
}

void Sound::fn_80305CA8(int index) {
    if (m_table->field_0x4[index].field_0x0 != 0) {
        fn_802DBD1C(m_table, index);
    }
}

UnkSoundHandle_801D3DC8 Sound::fn_80305C50(int index, void *owner) {
    if (m_table->field_0x4[index].field_0x0 != 0) {
        return fn_802DBC8C(m_table, index, owner);
    }
    return UnkSoundHandle_801D3DC8();
}

SoundBase *Sound::Create(Task *parent, const boost::shared_ptr<PlayerBase> &player,
                         const UnkSoundTable_805F8F90 *table) {
    return new Sound(parent, player, table);
}

} // namespace Npc
} // namespace Player
