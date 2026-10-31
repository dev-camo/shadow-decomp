/* CRI CFT (colour format transform) library init and finish (cft_lib.c, "CRI CFT/GC Ver.1.72").
 * The file name is inferred: the Sofdec libraries keep their version string and the Init/Finish
 * pair of the library in <library>_lib.c (mpv_lib.c, mps_lib.c, sfd_lib.c).
 */
#include "CRI/cft.h"

/* Names inferred from the version string and pointer of the other Sofdec libraries. */
const Char8 CFT_version_str[] =
    "\nCRI CFT/GC Ver.1.72 Build:Feb 28 2005 19:35:58\n\0Append: MW2407 GC20Apr2004Patch2\n";
static const Char8 *cri_verstr_ptr;

/* Nothing to release. The name is inferred: the counterpart of CFT_Init. */
void CFT_Finish(void) {}

/* Keeps the version string referenced and builds the static YCC to ARGB8888 coefficient tables. */
void CFT_Init(void) {
    cri_verstr_ptr = CFT_version_str;
    CFT_Ycc420plnToArgb8888Init();
}
