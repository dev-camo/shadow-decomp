#ifndef GAME_UNKWEAPONMODEL_800631FC_H
#define GAME_UNKWEAPONMODEL_800631FC_H

#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;

/* One 12-byte record of a part's sub-table (placeholder; the meanings of the words are unknown).
 * fn_80063828 reads the words at 0x0, 0x4 and 0x8 of each record and steps 0xC bytes from one to
 * the next (0x80063890-0x800638F4, USA). A negative word at 0x0 is a sentinel: fn_80063828 then
 * stores 0xFFFF in the record's output entry instead of looking the number up and adding the word
 * at 0x4 (0x80063898-0x800638B0). In the weapon files' descriptions a non-negative word at 0x0 is
 * a number that the weapon's load hook passes to fn_8006B50C; some descriptions use -1. */
struct UnkWeaponModelSubPart_80063828 {
    /* 0x0 */ int field_0x0;
    /* 0x4 */ int field_0x4;
    /* 0x8 */ int field_0x8;
};
CHECK_SIZEOF(UnkWeaponModelSubPart_80063828, 0xC);

/* One entry of a weapon model description's table of parts (placeholder; the code behind
 * fn_8006465C reads it, and the meanings of the words are unknown). In the weapon files'
 * descriptions, field_0x8, field_0xC (unless it is -1) and the first word of each of the
 * field_0x14 records that field_0x10 points to (unless it is negative, see
 * UnkWeaponModelSubPart_80063828) are numbers that the weapon's load hook passes to fn_8006B50C. */
struct UnkWeaponModelPart_8006465C {
    /* 0x00 */ int field_0x0;
    /* 0x04 */ int field_0x4;
    /* 0x08 */ int field_0x8;
    /* 0x0C */ int field_0xC;
    /* 0x10 */ const UnkWeaponModelSubPart_80063828 *field_0x10;
    /* 0x14 */ int field_0x14; /* the number of records at field_0x10 */
};
CHECK_SIZEOF(UnkWeaponModelPart_8006465C, 0x18);

/* The description a weapon model is built from (placeholder): a table of parts and a list of
 * numbers, each with its count. */
struct UnkWeaponModelDesc_8006465C {
    /* 0x00 */ const UnkWeaponModelPart_8006465C *parts; /* name inferred */
    /* 0x04 */ int partCount;                            /* name inferred */
    /* 0x08 */ const int *field_0x8;
    /* 0x0C */ int field_0xC;
};
CHECK_SIZEOF(UnkWeaponModelDesc_8006465C, 0x10);

/* The 0x24-byte object that a weapon model owns (placeholder; built by fn_8006465C). */
class UnkModel_8006465C;

/* The model object of a weapon (placeholder name: the original class is unknown), as at 0x1C of
 * Weapon::WeaponBase and 0x4 of Weapon::Item. It owns the object that fn_8006465C builds from a
 * description; the functions at 0x80063130-0x8006330C forward to that object. */
class UnkWeaponModel_800631FC {
  public:
    /* Builds the object from desc for owner (0x80063428): allocates it and stores it at 0x0
     * (0x80063444-0x80063468). The const qualifiers and the reference and pointer spellings of
     * owner and desc are inferred. */
    UnkWeaponModel_800631FC(const boost::weak_ptr<CharInfo> &owner,
                            const UnkWeaponModelDesc_8006465C *desc);
    /* Destroys the object at 0x0 (0x8006330C, through 0x80063358 and 0x800633A4). */
    ~UnkWeaponModel_800631FC();

    /* 0x00 */ UnkModel_8006465C *field_0x0;

  private:
    /* Copying has not been recovered; a shallow copy would destroy the object at 0x0 twice
     * (declared only, as in C++03). */
    UnkWeaponModel_800631FC(const UnkWeaponModel_800631FC &);
    UnkWeaponModel_800631FC &operator=(const UnkWeaponModel_800631FC &);
};
CHECK_SIZEOF(UnkWeaponModel_800631FC, 0x4);

extern "C" {
/* The functions below pass their arguments on to the object's function named in each comment
 * (behaviour from their bodies; purposes inferred where stated). */
/* fn_80063FFC; Weapon::WeaponBase's fn_8005CF64 calls it. */
void fn_80063130(UnkWeaponModel_800631FC *model);
/* fn_8006407C; Weapon::WeaponBase's fn_8005CF38 calls it. */
void fn_80063154(UnkWeaponModel_800631FC *model);
/* fn_8006414C: returns the matrix of the model's frame index (inferred from the callers, which
 * take the frame's position from it). The frames are 0x34-byte records with the matrix at 0x4,
 * in the array at 0xC of the owned object. fn_8006414C compares the signed index with the count
 * at 0x4 (0x8006414C-0x80064154) and returns frame 0's matrix when the index is at or above the
 * count (0x8006416C-0x80064174); a negative index is not rejected. The result points into that
 * array, which is destroyed with the model (0x8006461C-0x80064624). */
Fn8040Matrix34 *fn_80063234(UnkWeaponModel_800631FC *model, int index);
/* fn_80064320: advances the model by dt (inferred). */
void fn_800632A0(UnkWeaponModel_800631FC *model, float dt);
/* fn_800644DC: places the model (Weapon::Item passes a position and a rotation). */
void fn_800632E8(UnkWeaponModel_800631FC *model, const Fn8040Vector3 *position,
                 const UnkRotation3 *rotation);
}

#endif
