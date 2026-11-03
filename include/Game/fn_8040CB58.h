#ifndef GAME_FN_8040CB58_H
#define GAME_FN_8040CB58_H

#include "layout_check.h"

// Placeholder class name; the original name is unknown.
class Unk8040CB2C {
  public:
    Unk8040CB2C(unsigned int value);

    // Advances the four-word state and returns a float (behaviour inferred).
    float fn_8040CB58();

    /* 0x00 */ unsigned int m_words[4];
};
CHECK_SIZEOF(Unk8040CB2C, 0x10);

#endif
