#include "CRI/mwsfd.h"

/* Returns the work size of the frame converter handle; it does not depend on the picture size
 * (width and height are the caller's arguments and unused). */
Sint32 MWSFSFX_CalcHnWorkSiz(Sint32 width, Sint32 height) {
    return 0x301F;
}
