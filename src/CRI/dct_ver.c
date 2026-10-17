/* CRI DCT library version (dct_ver.c, "CRI DCT/GC Ver.1.951").
 * Body from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/dct.h"

const Char8 DCT_version_str[] =
    "\nCRI DCT/GC Ver.1.951 Build:Feb 28 2005 19:36:07\n\0Append: MW2407 GC20Apr2004Patch2\n";

static const Char8 *cri_verstr_ptr;

/* Returns the DCT build string (kept referenced through cri_verstr_ptr). */
const Char8 *DCT_GetVerStr(void) {
    cri_verstr_ptr = DCT_version_str;
    return DCT_version_str;
}
