/* CharaColliBaseCharInfo (class described in Game/CharaColliBaseCharInfo.h). File name inferred
 * from the class. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/CharInfo.h"
#include "Game/UnkWorld_80021B08.h"

using namespace SonicteamUSA::System::CharaColli;

/* The empty pointers described in Game/CharInfo.h and Game/CharaColliBaseCharInfo.h. */
boost::shared_ptr<CharInfo> lbl_805EE698;
boost::shared_ptr<CharaColliAttack> lbl_805EE6A0;

CharaColliBaseCharInfo::CharaColliBaseCharInfo(s16 arg0, s16 arg1, boost::weak_ptr<CharInfo> owner)
    : CharaColliBase(arg0, arg1), m_owner(owner) {}

CharaColliBaseCharInfo::~CharaColliBaseCharInfo() {
    fn_800C46DC(fn_80021B08(), this);
}
