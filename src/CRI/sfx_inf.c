/* CRI Sofdec SFX stream information (sfx_inf.c): the component layout lookup of a frame.
 *
 * The tag lookup is reduced to a constant in this game: the converter asks for the "COMPO" tag of
 * the frame and always gets the planar YCC 4:2:0 layout.
 */
#include "CRI/sfx.h"

Sint32 SFXINF_GetStmInf(SFX_SRCFRM *frm, const Char8 *tag) {
    return SFX_COMPO_YCC420PLN;
}
