#ifndef GAME_FN_801A4430_H
#define GAME_FN_801A4430_H

#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/function.hpp"
#include <map>
#include <utility>
#include "layout_check.h"

typedef std::pair<unsigned short, void *> Unk801A4430Event;

/* Original name unknown; the wrapper invokes its callback only when nonempty. */
class Unk801A4430Callback {
  public:
    Unk801A4430Callback();
    ~Unk801A4430Callback();
    bool fn_801A4A74() const;
    void fn_801A4A48(const Unk801A4430Event &event);

    /* 0x00 */ boost::function1<void, const Unk801A4430Event &> m_function;
};
CHECK_SIZEOF(Unk801A4430Callback, 0xC);

/* Original type name unknown; member meanings are inferred from the motion API. */
class Unk801A4430 {
  public:
    typedef std::map<unsigned int, Fn8040Matrix34> MatrixMap;
    Unk801A4430();
    ~Unk801A4430();
    Fn8040Matrix34 fn_801A4430(const Fn8040Vector3 &rotation);
    unsigned short fn_801A4494();
    void fn_801A44DC(bool enabled);
    void fn_801A4548(bool enabled);
    void fn_801A4584(const Fn8040Vector3 &rotation);
    void fn_801A45E0(const Fn8040Vector3 *offset);
    void fn_801A4604(const Fn8040Matrix34 &matrix);
    void fn_801A4640(const Fn8040Vector3 &position, const Fn8040Vector3 &rotation);
    void fn_801A469C(Fn8040Vector3 *vector);
    void fn_801A46C0(Fn8040Vector3 *vector);
    Fn8040Matrix34 *fn_801A46E4(unsigned int nodeId);
    bool fn_801A4810();
    void fn_801A4848(float speed);
    void fn_801A4898(unsigned short motionId, int target, float blendFrames);
    void fn_801A4934();
    void fn_801A4AF0();
    void fn_801A4B30(unsigned int resourceId, unsigned int firstMotionId, int arg);

    /* 0x00 */ _stMtnPlayWork *m_control;
    /* 0x04 */ unsigned int m_resourceId;
    /* 0x08 */ unsigned int m_firstMotionId;
    /* 0x0C */ bool m_enabled;
    /* 0x10 */ MatrixMap m_matrices;
    /* 0x20 */ Unk801A4430Callback m_callback;
};
CHECK_SIZEOF(Unk801A4430, 0x2C);

#endif
