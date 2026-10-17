/* Static initialisation of the 2D vector lbl_805F1358 (the fallback that fn_8040CC6C uses for a
 * vector too short to normalise). The constructor call puts the vector in .sbss and initialises it
 * at startup (a __sinit_ function and a .ctors entry). */
#include "Game/fn_8040_vector_types.h"

/* lbl_805FAD78 is the 0.0f literal shared with fn_8040CC5C, so it stays an external symbol while
 * that function is a separate unit. */
Fn8040Vector2 lbl_805F1358(lbl_805FAD78, lbl_805FAD78);
