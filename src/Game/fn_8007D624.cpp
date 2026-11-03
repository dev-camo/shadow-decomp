/* Sound-handle copy assignment; original filename unknown. */
#include "Game/UnkSoundHandle_801D3DC8.h"

UnkSoundHandle_801D3DC8 &UnkSoundHandle_801D3DC8::operator=(const UnkSoundHandle_801D3DC8 &other) {
    field_0x0 = other.field_0x0;
    field_0x4 = other.field_0x4;
    field_0x6 = other.field_0x6;
    field_0x8 = other.field_0x8;
    field_0xA = other.field_0xA;
    field_0xB = other.field_0xB;
    fn_800091D0(&field_0xC, &other.field_0xC);
    field_0x18 = other.field_0x18;
    return *this;
}
