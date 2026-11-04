#ifndef GAME_TENEMYCOMMONBASE_H
#define GAME_TENEMYCOMMONBASE_H

#include "Game/EnemyRefBase.h"
#include "Game/EnemyParamCommon.h"
#include "Game/EnemyStatusCommon.h"
#include "layout_check.h"

/* Original RTTI name. Combines the referenced enemy with the common parameter (0x104) and status
 * (0x110) components; it adds no virtual slots or members. */
class TEnemyCommonBase : public EnemyRefBase, public EnemyParamCommon, public EnemyStatusCommon {
  public:
    virtual ~TEnemyCommonBase();
};
CHECK_SIZEOF(TEnemyCommonBase, 0x1AC);

#endif
