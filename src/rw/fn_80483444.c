/* A RenderWare file path test. The original file name is unknown. */
#include "rw/rwcore.h"

/* Whether a path is absolute: it starts with a drive letter and a colon, or with a backslash */
RwBool fn_80483444(const RwChar *path) {
    if (path[1] == ':' &&
        ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z'))) {
        return TRUE;
    }
    return path[0] == '\\';
}
