#ifndef GAME_TGIHDGAGE_H
#define GAME_TGIHDGAGE_H

#include "Game/Task.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Chao {
namespace CSD {
class CProject;
} // namespace CSD
} // namespace Chao

/* TGiHdGage (class name from RTTI): a task of the in-game display that shows the CSD projects
 * HeroGauge and DarkGauge through a draw layer. Each update it moves each gauge's value towards
 * the first word of the matching entry of UnkManager_80079808::field_0x70C and sets the frame of
 * the gauge's scene from it. TGameInfomation's constructor creates it. Member and method names
 * are inferred. */
class TGiHdGage : public Task {
  public:
    TGiHdGage(Task *parent);
    virtual ~TGiHdGage();
    virtual void Update(float dt);

    /* Creates the two projects, loads their files and creates their scenes. */
    void LoadProjects();
    /* Deletes the two projects. Matching workaround: see its definition. */
    void ReleaseProjects();
    /* The draw layer's callback: draws the two projects while lbl_8057D6A0's byte at 0x148 is
     * set. */
    void OnDraw(RednerDrawArgs &args) const;

    struct Gauge {
        /* 0x0 */ Chao::CSD::CProject *project;
        /* 0x4 */ s32 value; /* the displayed value, which follows the entry's first word */
    };

    /* 0x28 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x30 */ Gauge m_gauges[2];
};
CHECK_SIZEOF(TGiHdGage, 0x40);

#endif
