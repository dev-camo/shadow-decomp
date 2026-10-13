#ifndef GAME_GADGETMODELINFO_H
#define GAME_GADGETMODELINFO_H

#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>

class GadgetCharInfo;

/* GadgetModelInfo (class name from RTTI): the 0x74-byte model of a gadget, loaded from a named
 * model file. Its members are not mapped yet; the methods keep placeholder names and argument types
 * taken from their callers. */
class GadgetModelInfo {
  public:
    GadgetModelInfo(int arg0, const char *fileName);
    virtual ~GadgetModelInfo();

    void fn_8016108C();
    void fn_801604FC();
    /* The four methods returning bool report success (behaviour inferred). */
    bool fn_80160B18(int arg0);
    bool fn_80160C28(int arg0);
    void fn_80161880();
    /* Attaches a character info to the model node named nodeName (behaviour inferred). */
    bool fn_80161F08(const char *nodeName, const boost::weak_ptr<GadgetCharInfo> &info);
    void fn_80162804();
    bool fn_80162A68(int arg0, int arg1);
    void fn_80162EF4();
    /* Advances the model by dt (decrements its timers; behaviour inferred). */
    void fn_80162FB0(float dt);
    /* Sets the model's world matrix (behaviour inferred). */
    void fn_801633DC(const Fn8040Matrix34 *matrix);

    /* 0x04 */ u8 field_0x4[0x70];
};

#endif
