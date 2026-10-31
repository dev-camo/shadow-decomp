/* CRI Sofdec MW player seek hooks (mwsfdsee.c): the seek-start entries are empty in this library.
 *
 * Functions and order from RE4's map (lib/mwsfdsee.c follows lib/mwstm.c there as here). The
 * parameter lists are inferred from the call sites; the bodies ignore them.
 */
#include "CRI/mwsfd.h"

/* Empty in this library; the player's decode server calls it with the handle. */
void MWSFSEE_ChkSupply(MWPLY mwply) {}

/* Empty in this library. The empty bodies do not show the argument count or types: the three
 * unsigned parameters are inferred. */
void MWSFSEE_StartFnameSub2(unsigned int arg0, unsigned int arg1, unsigned int arg2) {}

/* Empty in this library. */
void MWSFSEE_StartFnameSub1(unsigned int arg0, unsigned int arg1, unsigned int arg2) {}
