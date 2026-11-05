#ifndef GAME_GONG_H
#define GAME_GONG_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "Game/UnkObject_801695B8.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Chao {
namespace CSD {
class CProject;
} // namespace CSD
} // namespace Chao

namespace Gong {

/* The gadget's parameters, in the order of the descriptor's table ("speed"). */
struct GongParams {
    GongParams() : speed(0.0f) {}

    /* 0x0 */ float speed; /* model 1's hit callback divides the character's speed by it */
};
CHECK_SIZEOF(GongParams, 0x4);

/* Gong::Gong (class name from RTTI; the gadget's descriptor is "GONG"): three models loaded from
 * GONG.BIN, placed at the gadget's character info. Model N (m_model0 to m_model2) selects the
 * entry with id N (fn_80162A68) and attaches the character info to its part N
 * (fn_80161FC8); models 1 and 2 have collision callbacks that launch the characters they hit.
 * Member and method names are inferred. */
class Gong : public Gadget {
  public:
    Gong(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Gong();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, the rotation and the parameters from the owner and clears the state,
     * the hit count and the flags. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the three models at the character info's world matrix when all three exist. */
    void UpdateModelMatrix();
    /* The collision callbacks of models 1 and 2: they pass the hits to the hit callbacks with a
     * hit context. */
    void OnModel1Collision(ColliDetectArgs &args);
    void OnModel2Collision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model0;
    /* 0x40 */ boost::shared_ptr<GadgetModelInfo> m_model1;
    /* 0x48 */ boost::shared_ptr<GadgetModelInfo> m_model2;
    /* 0x50 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x58 */ GongParams m_params;
    /* 0 after ReadOwner, 1 from the first update on; UpdateBody has empty cases for 1 and 2, and
     * nothing here sets 2. */
    /* 0x5C */ s32 m_state;
    /* 0x60 */ s32 m_hitCount; /* the model 1 collisions that hit a character */
    /* 0x64 */ u32 m_flags;    /* bit 0: a model 2 hit has given its item */
};
CHECK_SIZEOF(Gong, 0x68);

namespace CSD {

/* Gong::CSD::GongCSD (class name from RTTI): a task that plays a scene of the CSD project
 * PR0302RINGANM through a draw layer and finishes when the scene's motion ends. fn_80136B5C
 * creates it. Member and method names are inferred. */
class GongCSD : public Task {
  public:
    /* motion: the index of the scene's motion. */
    GongCSD(Task *parent, int motion);
    virtual ~GongCSD();
    virtual void Update(float dt);

    /* The draw layer's callback: draws the project. */
    void OnDraw(RednerDrawArgs &args);

    /* 0x28 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x30 */ Chao::CSD::CProject *m_project;
    /* 0x34 */ float m_soundTimer;
    /* 0x38 */ UnkBitFlags_80014E04 m_soundFlags; /* bit 0: repeat a sound every 0.2 seconds */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(GongCSD, 0x40);

} // namespace CSD

} // namespace Gong

#endif
