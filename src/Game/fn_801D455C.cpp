/* UnkSoundHandle_801D3DC8::fn_801D455C; original filename and class name unknown. */
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkSoundList_801D3CC8.h"

void UnkSoundHandle_801D3DC8::fn_801D455C() {
    if (field_0x0) {
        fn_801D3CC8()->fn_801D4110(*this);
    }

    field_0x0 = 0;
    field_0x4 = 0;
    field_0x6 = 0;
    field_0x8 = 0;
    field_0xA = 0;
    field_0xB = 0;
    field_0x18 = 0;
    fn_8040CE60(&field_0xC);
}
