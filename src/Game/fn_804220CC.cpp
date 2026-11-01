/* Command-stream traversal adjacent to Effect::System RTTI data. The original file and
 * record type names are unknown; lengths are encoded record sizes in bytes. */
#include "Game/fn_804220CC.h"

unsigned int fn_804220CC(const unsigned char *command) {
    unsigned int count = 0;

    while (command != 0) {
        unsigned int size = 0;

        switch (command[0]) {
        case 0:
        case 2:
            break;
        case 1:
            size = 0x10;
            break;
        case 3:
            size = 0x24;
            break;
        case 4:
            size = 0x24;
            break;
        case 5:
            size = 0x14;
            break;
        case 6:
            size = 0x24;
            break;
        case 7:
            size = 0x1C;
            break;
        case 8:
            size = 0x24;
            break;
        case 9:
            size = 0x14;
            break;
        case 10:
            size = 0x14;
            break;
        case 11:
            size = 0x14;
            break;
        case 12:
            size = 0x10;
            break;
        case 13:
            size = 0x8;
            break;
        case 14:
            size = 0x8;
            break;
        case 15:
            size = 0x4;
            break;
        case 16:
            size = 0x8;
            break;
        case 17:
            size = 0x58;
            break;
        case 18:
            size = 0x5C;
            break;
        case 19:
            size = 0x4;
            break;
        case 20:
            size = 0x14;
            break;
        case 21:
            size = 0x20;
            break;
        case 22:
            size = 0x14;
            break;
        case 23:
            size = 0x10;
            break;
        case 24:
            size = 0x14;
            break;
        case 25:
            size = 0x10;
            break;
        case 26:
            size = 0x14;
            break;
        case 27:
            size = 0x14;
            break;
        case 28:
            size = 0x1C;
            break;
        case 29:
            size = 0x1C;
            break;
        case 30:
            size = 0x1C;
            break;
        case 31:
            size = 0x14;
            break;
        case 32:
            size = 0x14;
            break;
        case 33:
            size = 0x14;
            break;
        }

        if (command[1] != 0 && size != 0) {
            command += size;
        } else {
            command = 0;
        }
        count++;
    }
    return count;
}
