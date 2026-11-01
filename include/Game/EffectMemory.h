#ifndef GAME_EFFECTMEMORY_H
#define GAME_EFFECTMEMORY_H

/* The effect allocator's layout and original class name are not yet recovered. */
class UnkEffectMemory;

extern "C" {
UnkEffectMemory *fn_8042A5D0();
void fn_8042A444(UnkEffectMemory *memory, void *block);
}

#endif
