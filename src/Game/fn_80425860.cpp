/* Effect loader and manager lifecycle; original filename unknown. */
#include "Game/EffectManager.h"
#include "Game/UnkEffectFileData.h"
#include <cstdint>
#include <cstring>

namespace Effect {
namespace System {
namespace {

/* The file uses 32-bit relative offsets in slots that become native GameCube pointers. */
template <class T> inline void RelocateEffectPointer(T *&pointer, unsigned char *base) {
    if (pointer != 0) {
        pointer = reinterpret_cast<T *>(reinterpret_cast<std::uintptr_t>(pointer) +
                                        reinterpret_cast<std::uintptr_t>(base));
    }
}

} // namespace

EffectManager::EffectManager() : field_0x24(0), field_0x25(0), field_0x28(0) {}

EffectManager::~EffectManager() {}

bool EffectManager::fn_80425860(UnkEffectFileData *file) {
    if (file == 0) {
        return false;
    }
    if (std::strncmp(file->signature, "EFFD", 4) != 0) {
        field_0x28 = 2;
        return false;
    }
    if (file->field_0x4 < 1) {
        field_0x28 = 3;
        return false;
    }
    if (file->field_0x6 != 0) {
        return true;
    }
    unsigned char *base = reinterpret_cast<unsigned char *>(file);
    RelocateEffectPointer(file->first, base);
    for (UnkEffectElementData *record = file->first; record != 0; record = record->next) {
        switch (record->field_0x20) {
        case 2: {
            UnkEffectElementData2 *data = static_cast<UnkEffectElementData2 *>(record);
            RelocateEffectPointer(data->field_0x48, base);
            RelocateEffectPointer(data->field_0x58, base);
            RelocateEffectPointer(data->field_0x70, base);
            RelocateEffectPointer(data->field_0x68, base);
            break;
        }
        case 3: {
            UnkEffectElementData3 *data = static_cast<UnkEffectElementData3 *>(record);
            RelocateEffectPointer(data->field_0x38, base);
            break;
        }
        case 4: {
            UnkEffectElementData4 *data = static_cast<UnkEffectElementData4 *>(record);
            RelocateEffectPointer(data->field_0x3C, base);
            RelocateEffectPointer(data->field_0x40, base);
            break;
        }
        case 5: {
            UnkEffectElementData5 *data = static_cast<UnkEffectElementData5 *>(record);
            RelocateEffectPointer(data->field_0x48, base);
            RelocateEffectPointer(data->field_0x58, base);
            RelocateEffectPointer(data->field_0x5C, base);
            RelocateEffectPointer(data->field_0x74, base);
            RelocateEffectPointer(data->field_0x6C, base);
            break;
        }
        case 6: {
            UnkEffectElementData6 *data = static_cast<UnkEffectElementData6 *>(record);
            RelocateEffectPointer(data->field_0x54, base);
            RelocateEffectPointer(data->field_0x6C, base);
            RelocateEffectPointer(data->field_0x64, base);
            break;
        }
        case 7: {
            UnkEffectElementData7 *data = static_cast<UnkEffectElementData7 *>(record);
            RelocateEffectPointer(data->field_0x50, base);
            RelocateEffectPointer(data->field_0x54, base);
            break;
        }
        case 8: {
            UnkEffectElementData8 *data = static_cast<UnkEffectElementData8 *>(record);
            RelocateEffectPointer(data->field_0x34, base);
            break;
        }
        case 9: {
            UnkEffectElementData9 *data = static_cast<UnkEffectElementData9 *>(record);
            RelocateEffectPointer(data->field_0x44, base);
            RelocateEffectPointer(data->field_0x48, base);
            RelocateEffectPointer(data->field_0x4C, base);
            break;
        }
        case 10: {
            UnkEffectElementData10 *data = static_cast<UnkEffectElementData10 *>(record);
            RelocateEffectPointer(data->field_0x38, base);
            RelocateEffectPointer(data->field_0x3C, base);
            break;
        }
        }
        RelocateEffectPointer(record->next, base);
    }
    file->field_0x6 = 1;
    return true;
}

} // namespace System
} // namespace Effect
