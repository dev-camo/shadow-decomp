#ifndef GAME_FN_8040D608_H
#define GAME_FN_8040D608_H

#include "layout_check.h"

struct Fn8040D594Object;

#ifdef __cplusplus
namespace Sonicteam {
namespace System {
namespace Peripheral {
class IKeyboardManagerImp;
} // namespace Peripheral
} // namespace System
} // namespace Sonicteam
typedef Sonicteam::System::Peripheral::IKeyboardManagerImp Fn8040D594Provider;
#else
typedef struct Fn8040D594Provider Fn8040D594Provider;
#endif

/* Provisional member types for the manager's 0x30-stride records. The manager
 * constructor fn_8040D594 constructs two records with fn_8040D608/fn_8040D558;
 * its destructor fn_8040D4B8 destroys them with fn_8040D558. */
typedef struct Fn8040D608Object {
    /* 0x00 */ unsigned char field_0x0;
    /* 0x01 */ unsigned char field_0x1; /* Record index (0 or 1). */
    /* 0x02 */ unsigned short field_0x2;
    /* 0x04 */ unsigned short field_0x4;
    /* 0x08 */ unsigned int field_0x8;
    /* 0x0C */ unsigned int field_0xC;
    /* 0x10 */ unsigned int field_0x10;
    /* 0x14 */ unsigned int field_0x14;
    /* 0x18 */ unsigned int field_0x18;
    /* Unrecovered storage; the original member composition is unknown. */
    /* 0x1C */ unsigned char field_0x1C[0x10];
    /* 0x2C */ struct Fn8040D594Object *field_0x2C; /* Containing manager. */
} Fn8040D608Object;
CHECK_SIZEOF(Fn8040D608Object, 0x30);

/* Provisional manager layout through its owned interface pointer. */
typedef struct Fn8040D594Object {
    /* 0x00 */ Fn8040D608Object field_0x0[2];
    /* 0x60 */ Fn8040D594Provider *field_0x60;
} Fn8040D594Object;
CHECK_SIZEOF(Fn8040D594Object, 0x64);
CHECK_OFFSETOF(Fn8040D594Object, field_0x0, 0x00);
CHECK_OFFSETOF(Fn8040D594Object, field_0x60, 0x60);

#ifdef __cplusplus
extern "C" {
#endif

// fn_8040D498.c
void *fn_8040D498(void **slot);
Fn8040D608Object *fn_8040D4AC(Fn8040D594Object *manager, unsigned int index);

// fn_8040D608.c
void fn_8040D608(Fn8040D608Object *object);

#ifdef __cplusplus
}
#endif

#endif
