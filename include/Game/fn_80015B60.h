#ifndef GAME_FN_80015B60_H
#define GAME_FN_80015B60_H

#include "Game/fn_8040_vector_types.h"

/* Joint-chain helpers after Unk80015A7C's functions (placeholder names; the original names and the
 * owning file are unknown). The parameter roles are inferred from Player::Shadow::ShadowMotion's
 * calls: the matrices of a chain's joints, a target position, a frame matrix and the chain's
 * segment lengths. */
extern "C" {
/* Solves the three-joint chain first, second, third towards target in frame (behaviour inferred;
 * fn_80015DBC is the variant that passes 0 as the last argument of fn_80015EC8). */
void fn_80015B60(Fn8040Matrix34 *first, Fn8040Matrix34 *second, Fn8040Matrix34 *third,
                 const Fn8040Vector3 *target, const Fn8040Matrix34 *frame, float length0,
                 float length1, float length2);
/* The arc cosine of value, 0 for 1 or more and pi for -1 or less (behaviour from its body). */
float fn_80015D68(float value);
void fn_80015DBC(Fn8040Matrix34 *first, Fn8040Matrix34 *second, Fn8040Matrix34 *third,
                 const Fn8040Vector3 *target, const Fn8040Matrix34 *frame, float length0,
                 float length1, float length2);
/* Solves the two-joint chain first, second towards target from the node matrix (behaviour
 * inferred). The reference is inferred: ShadowMotion passes the vector that fn_8040FB08 returns in
 * place (0x800B16E0-0x800B1704). The lengths are declared before target and node: in that order
 * the arguments of ShadowMotion's calls are evaluated as in the original (0x800B16EC-0x800B1700). */
void fn_80015DE0(Fn8040Matrix34 *first, Fn8040Matrix34 *second, float length0, float length1,
                 const Fn8040Vector3 &target, const Fn8040Matrix34 *node);
}

#endif
