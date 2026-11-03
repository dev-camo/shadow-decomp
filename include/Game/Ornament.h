#ifndef GAME_ORNAMENT_H
#define GAME_ORNAMENT_H

#include "Game/Gadget.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class GadgetCharInfo;
class GadgetModelInfo;
class UnkTarget_80166AF4;
class UnkResource_80012700;
namespace GadgetBuilder {
class CrashInfoCtrl;
}

namespace ArtWork {
/* Grouping and type name inferred from the 16-byte owner copy; field names follow its labels. */
struct OrnamentParams {
    OrnamentParams() : model(0), scaleX(1.0f), scaleY(1.0f), scaleZ(1.0f) {}
    /* 0x00 */ int model;
    /* 0x04 */ float scaleX;
    /* 0x08 */ float scaleY;
    /* 0x0C */ float scaleZ;
};
CHECK_SIZEOF(OrnamentParams, 0x10);

/* Class name from RTTI. Original member layout/alignment at 0x89-0x8F is unrecovered. */
class Ornament : public Gadget {
  public:
    static const char *s_name;

    Ornament(Task *parent, UnkGadgetOwner_800C9CD0 *owner, const char *fileName);
    virtual ~Ornament();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    void fn_800E5B18();
    void fn_800E5BB8(UnkGadgetOwner_800C9CD0 *owner);

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<GadgetBuilder::CrashInfoCtrl> m_crashInfo;
    /* 0x48 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x50 */ int field_0x50;
    /* 0x54 */ OrnamentParams m_params;
    /* 0x64 */ UnkResource_80012700 *m_resource;
    /* 0x68 */ int m_state;
    /* 0x6C */ UnkSoundHandle_801D3DC8 m_sound;
    /* 0x88 */ bool field_0x88;
    /* Unaccessed allocation extent; original members/alignment unknown. */
    /* 0x89 */ u8 unknown_0x89[7];

  private:
    /* Copying is unrecovered. */
    Ornament(const Ornament &);
    Ornament &operator=(const Ornament &);
};
CHECK_SIZEOF(Ornament, 0x90);
} // namespace ArtWork

#endif
