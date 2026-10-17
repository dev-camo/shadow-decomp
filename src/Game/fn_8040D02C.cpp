/* Static initialisation of the 3D vector lbl_805E07A0 (the fallback that fn_8040CE74 uses for a
 * vector too short to normalise). The constructor call puts the vector in .bss and initialises it
 * at startup (a __sinit_ function and a .ctors entry). */
#include "Game/fn_8040_vector_types.h"

/* lbl_805FAD80 is the 0.0f literal shared with fn_8040CE74, so it stays an external symbol while
 * that function is a separate unit. */
Fn8040Vector3 lbl_805E07A0(lbl_805FAD80, lbl_805FAD80, lbl_805FAD80);
