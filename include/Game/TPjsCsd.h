#ifndef GAME_TPJSCSD_H
#define GAME_TPJSCSD_H

#include "Game/CSD/CTexList.h"
#include "Game/CSD/csdRCObject.h"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Chao {
namespace CSD {
class CProject;
} // namespace CSD
} // namespace Chao

/* A queued motion of a TPjsCsd (placeholder name): fn_8003E388 allocates 0x2C bytes for it, clears
 * next, stores -1 in index and copies the motion's name and the flag. fn_8003FB10 selects the
 * motion by index when index is not negative, otherwise by name. */
struct UnkMotionRequest_8003E388 {
    /* 0x00 */ UnkMotionRequest_8003E388 *next;
    /* 0x04 */ s32 index;
    /* 0x08 */ char name[0x20];
    /* fn_8003FB10 gives the scene the motion-end action 0 when it is set and 1 when it is clear,
     * unless the scene has that action already. */
    /* 0x28 */ bool flag;
};
CHECK_SIZEOF(UnkMotionRequest_8003E388, 0x2C);

/* TPjsCsd (class name from RTTI): a CSD project with one named scene and a list of motions to
 * play on it one after another. The functions declared here are between 0x8003E388 and
 * 0x8004002C in the USA release, in a file that has no source yet; only what TGiHissatu uses is
 * declared, and the descriptions come from the bodies. The functions keep placeholder names; the
 * member names are inferred. */
class TPjsCsd {
  public:
    /* The first queued motion; fn_8003FA08 clears it and the constructor leaves it alone. */
    /* 0x00 */ UnkMotionRequest_8003E388 *field_0x0;

    /* Clears the project pointer and constructs the texture list pointer. */
    TPjsCsd();
    /* Calls fn_8003E490. */
    virtual ~TPjsCsd();

    /* Returns false at once when there is a project already. Otherwise creates the project and
     * loads the project file of that name (the name goes through fn_800423B8): when the load
     * fails it calls fn_8003E490 and returns false, otherwise it clears field_0x0 and field_0x50
     * and returns true (result type inferred: the body returns 0 or 1). */
    bool fn_8003FA08(const char *name);
    /* Appends a request for the motion of that name to the list (the type of flag is inferred
     * from the byte store). */
    void fn_8003E388(const char *motion, bool flag);
    /* Starts the first queued motion: selects it on the scene named m_sceneName, creating the
     * scene with that motion when the project does not have it. When the scene exists after
     * that, sets its motion-end action from the request's flag, sets its frame to 0, resumes it
     * and removes the request from the list. Returns false when nothing is queued, otherwise
     * true (result type inferred). */
    bool fn_8003FB10();
    /* Draws the project when there is one and the byte at 0x805E3780 (USA) is set. */
    void fn_8003FDF4();
    /* Without a project, returns false and leaves field_0x50 unchanged. Otherwise calls the
     * project's fn_8036DAD4; when the scene's last frame has reached the motion's end it starts
     * the next queued motion (fn_8003FB10). field_0x50 then becomes false when the scene is
     * missing or its motion has ended with nothing queued, and true otherwise. Returns false
     * without a scene, otherwise true (result type inferred). */
    bool fn_8003FE2C();
    /* With a project: frees the queued requests, replaces the texture list pointer by the null
     * one at 0x805EE110 (USA), deletes the project and clears its pointer. */
    void fn_8003E490();

    /* 0x08 */ Chao::CSD::CProject *m_project;
    /* 0x0C */ Chao::CSD::RCPtr<Chao::CSD::CTexList> m_texList;
    /* 0x10 */ char m_sceneName[0x40]; /* the size is the gap to field_0x50 */
    /* 0x50 */ bool field_0x50;
};
CHECK_SIZEOF(TPjsCsd, 0x54);

#endif
