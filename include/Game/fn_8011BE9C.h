#ifndef GAME_FN_8011BE9C_H
#define GAME_FN_8011BE9C_H

#include <dolphin/types.h>

/* Returns how many of the five words of the table at 0x8057FB80 (USA) are not -1 (behaviour
 * inferred from its body). The function and the table belong to the file of the SECRETKEY gadget,
 * which follows the SECRETDOOR gadget's; the SECRETDOOR gadget reads the result as the number of
 * keys collected (inferred). The unsigned return type is inferred from that caller, which compares
 * the result with a signed member as unsigned (cmplw at 0x8011B8D4). */
extern "C" u32 fn_8011BE9C(void);

#endif
