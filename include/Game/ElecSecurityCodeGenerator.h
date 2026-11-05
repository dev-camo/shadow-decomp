#ifndef GAME_ELECSECURITYCODEGENERATOR_H
#define GAME_ELECSECURITYCODEGENERATOR_H

#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "boost/shared_ptr.hpp"
#include <list>
#include "layout_check.h"

class ElecSecurityCode;

/* The gadget parameters in the order of the ELEC_SECURITY descriptor's table, whose entry names
 * give the member names. fn_800C9DBC reads them from the owner. */
struct ElecSecurityCodeGeneratorParams {
    /* 0x00 */ s32 type; /* 0 to 2; the model entry id and the first code's type */
    /* 0x04 */ float range;
};
CHECK_SIZEOF(ElecSecurityCodeGeneratorParams, 0x8);

/* ElecSecurityCodeGenerator (class name from RTTI; the gadget's descriptor is "ELEC_SECURITY"):
 * once the nearest character info comes within its range, it creates an ElecSecurityCode turned
 * towards that character info, and it updates its codes, deleting each one whose update returns
 * false, until none is left. Then it removes its owner from the owner set; when the set is empty
 * afterwards and the com id is not 0, it sets that id's state to 1. It then finishes. It shows
 * SECURITY.BIN only in the debug display (behaviour inferred). Method names marked "(name
 * inferred)" are not original. */
class ElecSecurityCodeGenerator : public Gadget {
  public:
    ElecSecurityCodeGenerator(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecSecurityCodeGenerator();
    virtual void UpdateBody(float dt);
    /* Creates the model if there is none yet, places it and draws a debug sphere of the search
     * range (behaviour inferred). */
    virtual void fn_800CBC1C(float);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the generator under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation, parameters and com id into the generator (name
     * inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position (name inferred). */
    void UpdateModelMatrix();
    /* True when the nearest character info is within the range (name inferred). */
    bool IsCharInfoInRange();

    static const char *s_name;

    /* 0x30 */ std::list<ElecSecurityCode *> m_codes;
    /* Assigned by ElecSecurityCode's update (fn_80156C78); when set, the generator passes it to
     * fn_8020642C with 0x31 or 0x32 as it finishes. */
    /* 0x3C */ boost::shared_ptr<CharInfo> field_0x3C;
    /* 0x44 */ ElecSecurityCodeGeneratorParams m_params;
    /* 0x4C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x54 */ GadgetModelInfo *m_model; /* created by the debug display only */
    /* 0x58 */ u8 m_comId;               /* the owner's com id (fn_800C9E60) */
    /* 0x5C */ s32 m_loadedType;         /* the type the model was set up with */
    /* The state (inferred from the update): 0 waiting for a character info in range, 1 updating
     * the codes. */
    /* 0x60 */ s32 m_state;
    /* Unclassified storage in the original 0x68-byte allocation; no accesses are identified. */
    /* 0x64 */ u8 field_0x64[4];
};
CHECK_SIZEOF(ElecSecurityCodeGenerator, 0x68);
CHECK_SIZEOF(std::list<void *>, 0xC);
CHECK_SIZEOF(std::list<ElecSecurityCode *>, 0xC);

#endif
