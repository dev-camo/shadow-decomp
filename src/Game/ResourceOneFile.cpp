/* ResourceOneFile is named by RTTI; the original source filename is unknown. */
#include "Game/ResourceOneFile.h"

ResourceOneFile::ResourceOneFile(const char *name, bool asynchronous) {
    field_0x20 = 0;
    field_0x24 = 0;
    fn_80014CF4(&field_0x8, name);
    field_0x4 = 0;
    field_0x1C.Clear();
    if (asynchronous) {
        field_0x1C.Set(0);
    }
}
