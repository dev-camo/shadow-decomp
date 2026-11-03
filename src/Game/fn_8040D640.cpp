/* The original owner name and translation-unit filename remain unknown. */
#include "Game/fn_8040D640.h"

Fn8040D708Entry *Fn8040D640Owner::fn_8040D708(unsigned int index) {
    return &entries[index];
}

void Fn8040D640Owner::fn_8040D640(
    std::auto_ptr<Sonicteam::System::Peripheral::IMouseManagerImp> source, unsigned int index) {
    slots[index] = source;
}
