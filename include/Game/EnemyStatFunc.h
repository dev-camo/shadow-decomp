#ifndef GAME_ENEMYSTATFUNC_H
#define GAME_ENEMYSTATFUNC_H

/* Original RTTI name; virtual prefix through 0xA8. Method types and qualifiers are inferred. */
#include "Game/CharInfo.h"
#include "Game/UnkBitFlags_80014E04.h"

struct Unk801A8020Argument;

class EnemyStatFunc {
  public:
    virtual void fn_801A9138(); // 0x08
    virtual void fn_801A8CE8(); // 0x0C
    virtual void
    fn_801A86A8(float);         // 0x10; float supported by original single-precision arithmetic
    virtual void fn_801A8440(); // 0x14
    virtual void
    fn_801A83D8(float);         // 0x18; float supported by original single-precision arithmetic
    virtual void fn_801A833C(); // 0x1C
    virtual bool
    fn_8017B61C(int,
                bool); // 0x20; bool flag supported; exact original bool/byte spelling not unique
    virtual int fn_8017B6F8();                              // 0x24
    virtual unsigned char fn_8017B708();                    // 0x28
    virtual unsigned char fn_8017B700();                    // 0x2C
    virtual void fn_8017B710(unsigned char);                // 0x30
    virtual unsigned char fn_8017B720();                    // 0x34
    virtual void fn_801A7FA0();                             // 0x38
    virtual void fn_8017B718(unsigned char);                // 0x3C
    virtual unsigned char fn_8017B728();                    // 0x40
    virtual bool fn_801A8494();                             // 0x44
    virtual unsigned char fn_8017B730();                    // 0x48
    virtual unsigned char fn_8017B738();                    // 0x4C
    virtual unsigned char fn_8017B740();                    // 0x50
    virtual unsigned char fn_8017B748();                    // 0x54
    virtual unsigned char fn_8017B750();                    // 0x58
    virtual unsigned char fn_8017B758();                    // 0x5C
    virtual unsigned char fn_8017B760();                    // 0x60
    virtual unsigned char fn_8017B768();                    // 0x64
    virtual unsigned char fn_8017B770();                    // 0x68
    virtual unsigned char fn_8017B778();                    // 0x6C
    virtual void fn_801A8020(Unk801A8020Argument *);        // 0x70; pointee identity unresolved
    virtual int fn_8017B780();                              // 0x74
    virtual bool fn_801A8388(float);                        // 0x78
    virtual void fn_8017B7D8(unsigned char, unsigned char); // 0x7C
    virtual void fn_8017B788(unsigned char);                // 0x80
    virtual void fn_8017B790(unsigned int);                 // 0x84; raw word setter
    virtual void fn_8017B7B4(unsigned int);                 // 0x88; raw word setter
    virtual void fn_8017B7FC(bool);                         // 0x8C; boolean choice
    virtual void fn_801A7F64(bool);                         // 0x90; boolean choice
    virtual void fn_801A8608(int);                          // 0x94
    virtual void fn_801A85B4(const boost::shared_ptr<CharInfo> &); // 0x98
    virtual void fn_801A8504(const boost::shared_ptr<CharInfo> &); // 0x9C
    virtual void fn_801A84DC();                                    // 0xA0
    virtual void fn_801A84D8();                                    // 0xA4
    virtual UnkBitFlags_80014E04 *fn_8017B83C();                   // 0xA8
};
CHECK_SIZEOF(EnemyStatFunc, 0x4);

#endif
