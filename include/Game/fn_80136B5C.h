#ifndef GAME_FN_80136B5C_H
#define GAME_FN_80136B5C_H

extern "C" {
/* Unless arg0 is 5, creates a 0x40-byte task (vtable at 0x80528CC4, constructor at 0x801370C0)
 * under task layer 11, passing it arg0 (behaviour inferred from its body; the original name is
 * unknown). The FireRing gadget passes 1 when a character has passed through the ring. */
void fn_80136B5C(int arg0);
}

#endif
