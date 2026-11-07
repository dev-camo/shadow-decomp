#ifndef GAME_EFFLOCUSMULTIVTX_H
#define GAME_EFFLOCUSMULTIVTX_H

#include "Game/DebugDisp.h"
#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/scoped_ptr.hpp"
#include "layout_check.h"

/* The 0x30-byte object that Test::EffLocusMultiVtx owns (placeholder name; its constructor is at
 * 0x802DF0E0). The task's fn_802DE9BC and fn_802DE9CC store 0 and 1 in its byte at 0x2C, and
 * fn_802DE9DC clears its word at 0x4, the points pointer. */
class UnkLocus_802DF0E0;

namespace Test {

/* Test::EffLocusMultiVtx (class name from RTTI, with Task as its only base): a task that owns an
 * UnkLocus_802DF0E0 (the destructor destroys the pointer at 0x28 with fn_802DEAD0, and
 * fn_802DEA08 returns it). Only what Weapon::DarkHammer uses is declared; the method names are
 * placeholders and the constructor's parameter names are inferred. */
class EffLocusMultiVtx : public Task {
  public:
    /* Constructor at 0x802DEB40: creates the task under the parent that fn_80046178's manager
     * holds at 0x20 and passes arg0, count, points, matrix and color to the constructor of the
     * object at 0x28 (0x802DF0E0). That object copies the four color bytes (0x802DF118-0x802DF128)
     * but keeps the points and matrix addresses (0x802DF10C, 0x802DF114): while the points
     * pointer is set, its update reads both (0x802DEF00-0x802DEF40, matrix passed to fn_8040B67C)
     * and skips them once fn_802DE9DC clears the points pointer, so the points, and the matrix
     * when there is one, must stay valid while the task can run until that call. fn_802DE9DC
     * does not clear the matrix pointer. Weapon::DarkHammer passes its two line points and no
     * matrix. points and matrix are pointers (null-checked at 0x802DEF04 and 0x802DEF14); their
     * const qualifiers, and the const and reference spelling of color, are inferred. */
    EffLocusMultiVtx(int arg0, int count, const Fn8040Vector3 *points, const Fn8040Matrix34 *matrix,
                     const UnkDebugColor_802E0E84 &color);
    virtual ~EffLocusMultiVtx();
    virtual void Update(float dt);

    /* Clears the points pointer of the object at 0x28 (0x802DE9F4), so that its update stops
     * reading the points and the matrix. */
    void fn_802DE9DC();
    void fn_802DE9CC();
    void fn_802DE9BC();

    /* 0x28 */ boost::scoped_ptr<UnkLocus_802DF0E0> m_locus;
};
CHECK_SIZEOF(Test::EffLocusMultiVtx, 0x30);

} // namespace Test

#endif
