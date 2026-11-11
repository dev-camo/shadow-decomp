#ifndef GAME_FRAMEEFFECTPOOL_H
#define GAME_FRAMEEFFECTPOOL_H

#include "rw/rpworld.h"
#include "layout_check.h"

/* Keeps effects on frames of a clump (see Game/FrameEffectPool.cpp). The original name is unknown
 * and the method names are inferred. fn_8016F990, in code that is not recovered yet, constructs
 * the only object in a four-byte .sbss slot of the file that contains fn_8016F990 and returns
 * its address; its callers call the members below on that address. No member reads or writes the
 * object: the effects are in this file's array. Whether the class has data members is unknown. */
class UnkFrameEffectPool_803284B4 {
  public:
    /* Clears the matrix pointer of every array entry. */
    UnkFrameEffectPool_803284B4();
    /* Both only call StopEffects; what distinguishes them is unknown. */
    void fn_80328494();
    void fn_80328474();
    /* For every array entry: when its effect still locks, sets the effect's stop-requested flag
     * (twice: see the definition), clears its matrix, translation and rotation sources and
     * clears its active flag; then resets the entry's effect and, when the entry has a matrix,
     * deletes it and sets the entry's matrix pointer to null. Only the matrix copies that the
     * array still records are deleted (see AttachEffects). */
    void StopEffects();
    /* For each child frame of the clump's frame whose id (fn_8044758C) is one of the three ids
     * of the file's table: asks for that id's effect at a new copy of the frame's world matrix
     * and, when the array has an entry whose effect no longer locks, records the effect, the
     * frame's matrix and the copy in the first such entry. The entry's previous matrix pointer
     * is overwritten without being deleted. When all 0x80 entries are in use, nothing is
     * recorded: the result of the insertion (fn_803280E8, called at USA (GUPE8P) 0x8032824C) is
     * not used, so StopEffects can neither stop that effect nor delete the new copy. Returns
     * the number of effects it asked for, which is counted in both cases. */
    int AttachEffects(RpClump *clump);
    /* For every array entry whose effect still locks and that has both matrices: copies the
     * frame's current world matrix into the entry's own matrix. */
    void UpdateMatrices();
};
/* The size MWCC gives the members declared above (no data members). It is not the size of the
 * original class, which is unknown. */
CHECK_SIZEOF(UnkFrameEffectPool_803284B4, 0x1);

extern "C" {
/* Listed in a table at 0x804C4A18 (USA (GUPE8P) addresses) in records that also point to the
 * name of a stage; each record holds four such functions, from the highest address to the lowest
 * (the meaning of each slot is unknown). The five records are those of "PrisonIsland" (functions
 * at 0x804C4D44), "Space" (0x804C5064), "ARKPast2" (0x804C50B4), "DoomsBase1" (0x804C5154) and
 * "EggmansBase" (0x804C51A4). Each function only returns 1 (return type inferred). Nothing ties
 * these functions to the class above except their addresses; they may belong to separate stage
 * files. */
/* PrisonIsland */
int fn_80328048(void);
int fn_80328050(void);
int fn_80328058(void);
int fn_80328060(void);
/* ARKPast2 */
int fn_80328068(void);
int fn_80328070(void);
int fn_80328078(void);
int fn_80328080(void);
/* Space */
int fn_80328088(void);
int fn_80328090(void);
int fn_80328098(void);
int fn_803280A0(void);
/* DoomsBase1 */
int fn_803280A8(void);
int fn_803280B0(void);
int fn_803280B8(void);
int fn_803280C0(void);
/* EggmansBase */
int fn_803280C8(void);
int fn_803280D0(void);
int fn_803280D8(void);
int fn_803280E0(void);
}

#endif
