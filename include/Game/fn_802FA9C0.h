#ifndef GAME_FN_802FA9C0_H
#define GAME_FN_802FA9C0_H

#include <dolphin/types.h>
#include "layout_check.h"

/* A set of message slots whose message request is attempted once each (placeholder name; the
 * original class and file names are unknown). The enemy whose code is at 0x801979B0-0x80198830
 * (USA (GUPE8P)) keeps one at 0x35C, constructs it in 0x80198738 and destroys it in 0x80198638. */
class Unk802FA9C0 {
  public:
    Unk802FA9C0();
    ~Unk802FA9C0();
    /* When bit index of m_requested is clear, requests the message of slot index (fn_802FAA58) and
     * then sets that bit whatever the request did: nothing is requested for a stage without
     * messages, and fn_801FAC44 can refuse the request (it returns -1 without queueing). */
    void fn_802FA9C0(int index);

    /* 0x0 */ u32 m_requested; /* one bit per slot, set by the first attempt (name inferred) */
};
CHECK_SIZEOF(Unk802FA9C0, 0x4);

#endif
