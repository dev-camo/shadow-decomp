/* Matching workaround: the "_CT" tag of event file names and the function that reads the number
 * after it are a file of their own. The original file name is unknown: this one takes the prefix
 * of the reader's file, Game/PjsEventCen.cpp (basis in Game/PjsEventCen.h; the function's only
 * callers, fn_80172248 and fn_801729F0, are in the event code that also repeats the reader's
 * extension test), and adds the tag. The split is inferred too. The function follows the .CEN
 * reader in .text, but in .sdata2 the tag's string comes after the reader's int-to-float
 * conversion constant. Built with the game's C++ options (deferred inlining), the compiler places
 * the string of a global defined in the reader's file, before or after its functions, in front of
 * that constant. A single file gives the original .sdata2 order only when it is built without
 * deferred inlining, with its functions defined in address order and the tag after the reader's
 * functions. Revisit when the original file boundary or the options of the reader's file are
 * established: if they are one file, merge this one back into Game/PjsEventCen.cpp. */
#include "Game/PjsEventCt.h"
#include "Game/UnkFileManager_800371D8.h"
#include <cstring>

const char *lbl_805ED3D0 = "_CT";

int fn_80329524(const char *name) {
    char *copy;
    int number = 0;

    /* Work on a copy in which every '.' after the first character is a terminator. The loop runs
     * length - 1 times, so an empty name is not handled. */
    if (name != NULL) {
        size_t length = strlen(name);
        copy = static_cast<char *>(fn_80049B90(length + 1, 2));
        strcpy(copy, name);
        for (int i = length - 1; i != 0; i--) {
            if (copy[i] == '.') {
                copy[i] = '\0';
            }
        }
    } else {
        copy = NULL;
    }

    char *found = strstr(copy, lbl_805ED3D0);
    if (found != NULL) {
        found += strlen(lbl_805ED3D0);
        number = (found[0] - '0') * 10 + found[1] - '0';
    }
    fn_80049B34(copy);
    return number;
}
