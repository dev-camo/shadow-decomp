/* The reader of an event's .CEN file: a list of timed positions that offsets the event's base
 * position (behaviour in the header). The original file and class names are unknown: the file
 * name is inferred from the extension and from TPjsEventActionScene, the RTTI name of the task
 * that holds the reader (evidence in the header). Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). */
#include "Game/PjsEventCen.h"
#include <cstring>
#include "Game/DebugMissionClearCollision.h"
#include "Game/UnkArchive_8004CA28.h"
#include "Game/UnkFileManager_800371D8.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80074470.h"
#include "Game/UnkManager_800CBAA4.h"
#include "Game/UnkManager_80170D8C.h"

const char lbl_805F9CA8[] = ".CEN";

UnkCenFile_80329514::UnkCenFile_80329514() {
    m_state = 0;
    m_keyCount = 0;
}

UnkCenFile_80329514::~UnkCenFile_80329514() {}

bool UnkCenFile_80329514::Load(UnkArchive_8004CA28 *archive, int index) {
    if (HasExtension(fn_8004CA28(archive, index)->name, lbl_805F9CA8)) {
        strcpy(m_name, fn_8004CA28(archive, index)->name);
        fn_8004C358(archive, index, Parse, this);
        m_state = 1;
        return true;
    }
    return false;
}

bool UnkCenFile_80329514::Update(float time) {
    bool found = false;
    int previous = lbl_805EF2B8;

    if (m_keyCount == 0) {
        memset(&lbl_80580224, 0, sizeof(lbl_80580224));
        found = true;
        lbl_805EF2B8 = 0;
    } else {
        for (lbl_805EF2B8 = 0; lbl_805EF2B8 < m_keyCount - 1; lbl_805EF2B8++) {
            if (time >= m_keys[lbl_805EF2B8].time && time < m_keys[lbl_805EF2B8 + 1].time) {
                found = true;
                break;
            }
        }
        if (!found) {
            lbl_805EF2B8 = previous;
            return false;
        }
        lbl_80580224 = m_keys[lbl_805EF2B8].position;
    }

    UnkPoints_80170D8C *points = fn_80170D8C()->field_0x8;
    switch (lbl_805EF2A0) {
    case 950:
    case 970:
    case 972:
    case 974:
    case 976:
    case 978:
        lbl_80580218 = fn_800CBAA4()->field_0x10[lbl_8057D6A0.field_0xA8.field_0x4].field_0xB4;
        break;
    default:
        if (fn_80074470()->field_0x8 == 6 || fn_80074470()->field_0x8 == 14) {
            if (lbl_8057D6A0.field_0x125) {
                lbl_80580218 =
                    fn_800CBAA4()->field_0x10[lbl_8057D6A0.field_0xA8.field_0x4].field_0xA8;
            } else if (lbl_8057D6A0.field_0xA8.field_0xA) {
                lbl_80580218 = fn_800CBAA4()
                                   ->field_0x10[lbl_8057D6A0.field_0xA8.field_0x4]
                                   .field_0x60[1]
                                   .position;
            } else {
                int point =
                    lbl_8057D6A0.field_0xA8.field_0x9 ? 1 : lbl_8057D6A0.field_0xA8.field_0x1C;
                lbl_80580218 = fn_800CBAA4()
                                   ->field_0x10[lbl_8057D6A0.field_0xA8.field_0x4]
                                   .field_0x60[point]
                                   .position;
            }
        } else if (points != NULL) {
            if (lbl_805EF2A4 < 0) {
                lbl_80580218 = points->field_0x8[0];
            } else {
                lbl_80580218 = points->field_0x8[lbl_805EF2A4];
            }
        }
        /* Otherwise lbl_80580218 keeps its value. */
        break;
    }

    lbl_80580230.x = lbl_80580218.x + lbl_80580224.x;
    lbl_80580230.y = lbl_80580218.y + lbl_80580224.y;
    lbl_80580230.z = lbl_80580218.z + lbl_80580224.z;
    return found;
}

void UnkCenFile_80329514::Parse(void *data, void *user) {
    UnkCenFile_80329514 *file = static_cast<UnkCenFile_80329514 *>(user);
    const char *text;
    bool negative = false;
    bool hasDigit = false;
    int field = 0;
    int value = 0;

    file->m_keyCount = 0;
    for (text = static_cast<const char *>(data); *text != 'E'; text++) {
        char c = *text;
        if (c == '-') {
            negative = true;
        } else if (c >= '0' && c <= '9') {
            hasDigit = true;
            value = value * 10 + c - '0';
        } else if (hasDigit) {
            /* Any other character ends a number. */
            if (negative) {
                value *= -1;
            }
            switch (field) {
            case 0:
                file->m_keys[file->m_keyCount].time = value;
                field++;
                break;
            case 1:
                file->m_keys[file->m_keyCount].position.x = value;
                field++;
                break;
            case 2:
                file->m_keys[file->m_keyCount].position.y = value;
                field++;
                break;
            case 3:
                file->m_keys[file->m_keyCount].position.z = value;
                field = 0;
                file->m_keyCount++;
                break;
            }
            negative = false;
            value = 0;
            hasDigit = false;
        }
    }

    fn_80049B34(data);
    file->m_state = 2;
}
