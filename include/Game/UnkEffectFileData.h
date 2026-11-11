#ifndef GAME_UNKEFFECTFILEDATA_H
#define GAME_UNKEFFECTFILEDATA_H

#include "layout_check.h"

namespace Effect {
namespace System {

/* Accessed record prefix; the remaining serialized layout is not yet known. */
struct UnkEffectElementData {
    /* 0x00 */ unsigned char field_0x0[0x20];
    /* 0x20 */ short field_0x20;
    /* 0x24 */ int id;
    /* 0x28 */ UnkEffectElementData *next;
};
CHECK_SIZEOF(UnkEffectElementData, 0x2C);

/* Accessed file prefix. Unknown byte arrays are serialized storage, not padding. */
struct UnkEffectFileData {
    /* 0x00 */ char signature[4];
    /* 0x04 */ short field_0x4;
    /* 0x06 */ short field_0x6;
    /* 0x08 */ unsigned char field_0x8[0x24];
    /* 0x2C */ UnkEffectElementData *first;
};
CHECK_SIZEOF(UnkEffectFileData, 0x30);

struct UnkCommandData;
struct UnkEffectGlobalParamData;
struct UnkEffectShimmerData;

/* Particle definition subrecords. Names and types are inferred from the base Particle
 * constructor, spawning path and command users. These are accessed views; the full serialized
 * record extents are unknown. */
struct UnkEffectParticleShape {
    /* 0x00 */ int type;
    /* 0x04 */ unsigned int flags;
};
CHECK_SIZEOF(UnkEffectParticleShape, 0x8);

/* Accessed shape prefixes. Type names, field names and qualifiers are inferred; complete
 * serialized extents are unknown. Each variant is read only after its original type test. */
struct UnkEffectParticleShape0 : UnkEffectParticleShape {
    /* 0x08 */ float radius;
    /* 0x0C */ float angle;
    /* 0x10 */ float spread;
    /* 0x14 */ float height;
};
CHECK_SIZEOF(UnkEffectParticleShape0, 0x18);

struct UnkEffectParticleShape1 : UnkEffectParticleShape {
    /* 0x08 */ float radius;
    /* 0x0C */ float angle;
    /* 0x10 */ float spread;
};
CHECK_SIZEOF(UnkEffectParticleShape1, 0x14);

struct UnkEffectParticleShape2 : UnkEffectParticleShape {
    /* 0x08 */ float width;
    /* 0x0C */ float height;
    /* 0x10 */ float depth;
};
CHECK_SIZEOF(UnkEffectParticleShape2, 0x14);

struct UnkEffectParticleData124 {
    /* 0x00 */ unsigned int flags;
    /* 0x04 */ float speed;
    /* 0x08 */ float speedRange;
    /* 0x0C */ float lifetime;
    /* 0x10 */ float scale;
    /* 0x14 */ float gravity;
    /* 0x18 */ float friction;
    /* 0x1C */ const UnkCommandData *command;
};
CHECK_SIZEOF(UnkEffectParticleData124, 0x20);

struct UnkEffectParticleData120 {
    /* 0x00 */ unsigned int flags;
    /* 0x04 */ short capacity;
    /* 0x06 */ short commandCount;
    /* 0x08 */ float duration;
    /* 0x0C */ const UnkCommandData *command;
    /* 0x10 */ float generationRate;
    /* 0x14 */ const UnkEffectParticleShape *shape;
};
CHECK_SIZEOF(UnkEffectParticleData120, 0x18);

/* Accessed variant prefixes, not complete serialized record sizes. Unknown arrays represent
 * uninterpreted encoded storage; automatic alignment gaps are left to the compiler. */
struct UnkEffectElementData2 : UnkEffectElementData {
    /* 0x2C */ UnkEffectParticleData124 particle;
    /* 0x4C */ unsigned int field_0x4C;
    /* 0x50 */ int field_0x50;
    /* 0x54 */ unsigned char field_0x54[4];
    /* 0x58 */ const char *field_0x58;
    /* 0x5C */ UnkEffectParticleData120 emitter;
};
CHECK_SIZEOF(UnkEffectElementData2, 0x74);

struct UnkEffectElementData3 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ short field_0x30;
    /* 0x32 */ short field_0x32;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ const UnkCommandData *field_0x38;
};
CHECK_SIZEOF(UnkEffectElementData3, 0x3C);

/* Accessed prefix; the complete serialized extent is unknown. */
struct UnkEffectElementData4 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ int field_0x30;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ unsigned char field_0x38;
    /* 0x39 */ unsigned char field_0x39[3];
    /* 0x3C */ void *field_0x3C;
    /* 0x40 */ const UnkCommandData *field_0x40;
};
CHECK_SIZEOF(UnkEffectElementData4, 0x44);

struct UnkEffectElementData5 : UnkEffectElementData {
    /* 0x2C */ UnkEffectParticleData124 particle;
    /* 0x4C */ unsigned int field_0x4C;
    /* 0x50 */ unsigned char field_0x50[4];
    /* 0x54 */ unsigned char field_0x54[4];
    /* 0x58 */ void *field_0x58;
    /* 0x5C */ void *field_0x5C;
    /* 0x60 */ UnkEffectParticleData120 emitter;
};
CHECK_SIZEOF(UnkEffectElementData5, 0x78);

struct UnkEffectElementData6 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ float field_0x38;
    /* 0x3C */ float field_0x3C;
    /* 0x40 */ float field_0x40;
    /* 0x44 */ float field_0x44;
    /* 0x48 */ unsigned int field_0x48;
    /* 0x4C */ unsigned char field_0x4C[4];
    /* 0x50 */ unsigned char field_0x50[4];
    /* 0x54 */ const UnkCommandData *field_0x54;
    /* 0x58 */ UnkEffectParticleData120 emitter;
};
CHECK_SIZEOF(UnkEffectElementData6, 0x70);

struct UnkEffectElementData7 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ int field_0x34;
    /* 0x38 */ unsigned int field_0x38;
    /* 0x3C */ int field_0x3C;
    /* 0x40 */ unsigned char field_0x40[4];
    /* 0x44 */ unsigned int field_0x44;
    /* 0x48 */ short field_0x48;
    /* 0x4A */ short field_0x4A;
    /* 0x4C */ float field_0x4C;
    /* 0x50 */ const UnkCommandData *field_0x50;
    /* 0x54 */ const UnkCommandData *field_0x54;
};
CHECK_SIZEOF(UnkEffectElementData7, 0x58);

struct UnkEffectElementData8 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ const UnkEffectGlobalParamData *field_0x34;
};
CHECK_SIZEOF(UnkEffectElementData8, 0x38);

struct UnkEffectElementData9 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ unsigned char field_0x30;
    /* 0x31 */ unsigned char field_0x31;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ float field_0x38;
    /* 0x3C */ float field_0x3C;
    /* 0x40 */ float field_0x40;
    /* 0x44 */ void *field_0x44;
    /* 0x48 */ void *field_0x48;
    /* 0x4C */ void *field_0x4C;
};
CHECK_SIZEOF(UnkEffectElementData9, 0x50);

struct UnkEffectElementData10 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ unsigned char field_0x30;
    /* 0x32 */ short field_0x32;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ UnkEffectShimmerData *field_0x38;
    /* 0x3C */ const UnkCommandData *field_0x3C;
};
CHECK_SIZEOF(UnkEffectElementData10, 0x40);

} // namespace System
} // namespace Effect

#endif
