#ifndef GAME_SHAREDCONSTANTS_H
#define GAME_SHAREDCONSTANTS_H

/* Float constants defined once and read by many game units, mostly by static initialisers that
 * convert angles. The header name and constant names are not original. */

/* pi (3.14159274f), in .sdata2 */
extern const float lbl_805FAD38;

/* 30000.0f, in .sdata2. Its readers convert it to an integer (meaning unknown). */
extern const float lbl_805F4BF0;

/* 200.0f, in .sdata2, outside the files split so far. The ROCKET gadget passes it as the radius of
 * its debug cylinder (meaning inferred from that use). */
extern const float lbl_805F8560;

#endif
