#ifndef GAME_CSD_CSDCHUNKTAGS_H
#define GAME_CSD_CSDCHUNKTAGS_H

#include <dolphin/types.h>

/* The four-character tags of the library's project file format. They are variables that the
 * library's RenderWare initialisation (InitLibraryRW) fills in as multi-character constants;
 * the names are the addresses in the USA release and the values are given with each. The file
 * name is not an original one. */
extern u32 lbl_805859E0; /* "NGIF", the first word of a project file */
extern u32 lbl_805859E4; /* "NGTL" */
extern u32 lbl_805859E8; /* "NGOB" */
extern u32 lbl_805859EC; /* "NGMO" */
extern u32 lbl_805859F0; /* "NGMM" */
extern u32 lbl_805859F4; /* "NGMC" */
extern u32 lbl_805859F8; /* "NGML" */
extern u32 lbl_805859FC; /* "NGMA" */
extern u32 lbl_80585A00; /* "NGLI" */
extern u32 lbl_80585A04; /* "NGCA" */
extern u32 lbl_80585A08; /* "NGMT" */
extern u32 lbl_80585A0C; /* "NGNN" */
extern u32 lbl_80585A10; /* "NOF0" */
extern u32 lbl_80585A14; /* "NEND", the tag that ends the list of chunks */

#endif
