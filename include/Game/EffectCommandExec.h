#ifndef GAME_EFFECTCOMMANDEXEC_H
#define GAME_EFFECTCOMMANDEXEC_H

#include "Game/fn_8042A444.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Effect {
namespace System {

class EffectElement;
class EffectArranger;
class EffectManager;
class EffectLaunchInfo;

/* Encoded command types; names are inferred from the executor classes and consumers.
 * Type 0 has no identified consumer and retains a numeric placeholder. */
enum CommandType {
    COMMAND_TYPE_00 = 0x00,
    COMMAND_PTN_ANIM = 0x01,
    COMMAND_GLARE_SWITCH = 0x02,
    COMMAND_3D_POSITION = 0x03,
    COMMAND_3D_ROTATION = 0x04,
    COMMAND_2D_ROTATION = 0x05,
    COMMAND_3D_SCALE = 0x06,
    COMMAND_2D_SCALE = 0x07,
    COMMAND_3D_VELOCITY = 0x08,
    COMMAND_GRAVITY = 0x09,
    COMMAND_FRICTION = 0x0A,
    COMMAND_MAT_COLOR = 0x0B,
    COMMAND_MAT_ALPHA = 0x0C,
    COMMAND_WAIT = 0x0D,
    COMMAND_LOOP_START = 0x0E,
    COMMAND_LOOP_END = 0x0F,
    COMMAND_CALLBACK = 0x10,
    COMMAND_LAUNCH = 0x11,
    COMMAND_TIMED_LAUNCH = 0x12,
    COMMAND_REQUEST_STOP = 0x13,
    COMMAND_GEN_QUANTITY = 0x14,
    COMMAND_LOCUS_POINT = 0x15,
    COMMAND_TOP_COLOR = 0x16,
    COMMAND_TOP_ALPHA = 0x17,
    COMMAND_LAST_COLOR = 0x18,
    COMMAND_LAST_ALPHA = 0x19,
    COMMAND_LENGTH = 0x1A,
    COMMAND_WIDTH = 0x1B,
    COMMAND_2D_AMPLITUDE = 0x1C,
    COMMAND_2D_CYCLE = 0x1D,
    COMMAND_2D_WAVE_LENGTH = 0x1E,
    COMMAND_AMPLITUDE = 0x1F,
    COMMAND_CYCLE = 0x20,
    COMMAND_WAVE_LENGTH = 0x21,
};

/* Encoded strides for partial or not-yet-shared record layouts. */
enum {
    COMMAND_GLARE_SWITCH_STRIDE = 0x8,
    COMMAND_MAT_COLOR_STRIDE = 0x14,
    COMMAND_MAT_ALPHA_STRIDE = 0x10,
    COMMAND_CALLBACK_STRIDE = 0x8,
    COMMAND_LOCUS_POINT_STRIDE = 0x20
};

/* Command output flags (names inferred from the command classes and their writes). */
enum { COMMAND_STOP_REQUESTED = 1, COMMAND_GLARE_CHANGED = 2, COMMAND_GLARE_ENABLED = 4 };

/* Encoded command headers, loop and wait data; these record names are placeholders. */
struct UnkCommandData {
    /* 0x00 */ unsigned char type;
    /* 0x01 */ unsigned char hasNext;
    /* 0x02 */ unsigned char stop;
    /* 0x03 */ unsigned char field_0x3;
};
CHECK_SIZEOF(UnkCommandData, 0x4);

struct UnkLoopStartData : UnkCommandData {
    /* 0x04 */ unsigned int count;
};
CHECK_SIZEOF(UnkLoopStartData, 0x8);

struct UnkWaitData : UnkCommandData {
    /* 0x04 */ unsigned int duration;
};
CHECK_SIZEOF(UnkWaitData, 0x8);

struct UnkGlareSWData : UnkCommandData {
    /* 0x04 */ unsigned char enabled;
};
CHECK_SIZEOF(UnkGlareSWData, 0x5);

/* Common encoded value-command fields; bytes 6-7 have no identified use yet. */
struct UnkValueData : UnkCommandData {
    /* 0x04 */ unsigned char relative;
    /* 0x05 */ unsigned char sharedRandom;
    /* 0x06 */ unsigned char field_0x6[2];
    /* 0x08 */ unsigned short duration;
    /* 0x0A */ unsigned short durationRange;
};
CHECK_SIZEOF(UnkValueData, 0xC);

struct UnkSingleData : UnkValueData {
    /* 0x0C */ float value;
    /* 0x10 */ float range;
};
CHECK_SIZEOF(UnkSingleData, 0x14);

struct UnkVector2Data : UnkValueData {
    /* 0x0C */ Fn8040Vector2 value;
    /* 0x14 */ Fn8040Vector2 range;
};
CHECK_SIZEOF(UnkVector2Data, 0x1C);

struct UnkVector3Data : UnkValueData {
    /* 0x0C */ Fn8040Vector3 value;
    /* 0x18 */ Fn8040Vector3 range;
};
CHECK_SIZEOF(UnkVector3Data, 0x24);

/* Material channels use 22 fractional bits; names are inferred from the integer accesses. */
enum {
    MATERIAL_FRACTION_BITS = 22,
    MATERIAL_CHANNEL_MAX = 255 << MATERIAL_FRACTION_BITS,
    MATERIAL_CHANNEL_CUTOFF = (1 << MATERIAL_FRACTION_BITS) - 1
};

/* Accessed record prefixes; encoded strides are 0x10 and 0x14 bytes respectively. */
struct UnkMatAlphaData : UnkValueData {
    /* 0x0C */ unsigned char value;
    /* 0x0D */ unsigned char range;
};
CHECK_SIZEOF(UnkMatAlphaData, 0xE);

struct UnkMatColorData : UnkValueData {
    /* 0x0C */ unsigned char value[3];
    /* 0x0F */ unsigned char range[3];
};
CHECK_SIZEOF(UnkMatColorData, 0x12);

struct UnkPtnAnimData : UnkCommandData {
    /* 0x04 */ unsigned char mode;
    /* 0x05 */ unsigned char loop;
    /* 0x06 */ unsigned char start;
    /* 0x07 */ unsigned char startRange;
    /* 0x08 */ unsigned char end;
    /* 0x09 */ unsigned char field_0x9[3];
    /* 0x0C */ unsigned short interval;
    /* 0x0E */ unsigned short intervalRange;
};
CHECK_SIZEOF(UnkPtnAnimData, 0x10);

struct UnkLaunchData : UnkValueData {
    /* 0x0C */ int id;
    /* 0x10 */ Fn8040Vector3 translation;
    /* 0x1C */ Fn8040Vector3 translationRange;
    /* 0x28 */ Fn8040Vector3 rotation;
    /* 0x34 */ Fn8040Vector3 rotationRange;
    /* 0x40 */ Fn8040Vector3 scale;
    /* 0x4C */ Fn8040Vector3 scaleRange;
};
CHECK_SIZEOF(UnkLaunchData, 0x58);

struct UnkTimedLaunchData : UnkLaunchData {
    /* 0x58 */ unsigned char stopAtDuration;
};
CHECK_SIZEOF(UnkTimedLaunchData, 0x5C);

/* Class names are recorded in the original RTTI. Member names are inferred from use. */
class EffectCommandExec {
  public:
    EffectCommandExec(const UnkCommandData *command)
        : m_command(command), m_prev(0), m_next(0), field_0x10(0) {}
    virtual ~EffectCommandExec();
    /* Advances a command; false removes it from the executor list (name inferred). */
    virtual bool Update(float deltaTime) = 0;

    /* The selected pool determines the allocation size. */
    static void *operator new(unsigned long) throw() {
        return fn_8042A50C(fn_8042A5D0());
    }

    static void operator delete(void *block) {
        fn_8042A444(fn_8042A5D0(), block);
    }

    /* 0x04 */ const UnkCommandData *m_command;
    /* 0x08 */ EffectCommandExec *m_prev;
    /* 0x0C */ EffectCommandExec *m_next;
    /* 0x10 */ unsigned char field_0x10;
};
CHECK_SIZEOF(EffectCommandExec, 0x14);

class EffCmdCallbackExec : public EffectCommandExec {
  public:
    EffCmdCallbackExec(const UnkCommandData *command, unsigned int *flags);
    virtual ~EffCmdCallbackExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
};
CHECK_SIZEOF(EffCmdCallbackExec, 0x18);

class EffCmdLoopEndExec : public EffectCommandExec {
  public:
    EffCmdLoopEndExec(const UnkCommandData *command, unsigned int *flags, EffectCommandExec *last,
                      const UnkCommandData **restart);
    virtual ~EffCmdLoopEndExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ bool m_finished;
};
CHECK_SIZEOF(EffCmdLoopEndExec, 0x1C);

class EffCmdLoopStartExec : public EffectCommandExec {
  public:
    EffCmdLoopStartExec(const UnkCommandData *command, unsigned int *flags);
    virtual ~EffCmdLoopStartExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ unsigned int m_count;
    /* 0x1C */ unsigned int m_remaining;
    /* 0x20 */ bool m_finished;
};
CHECK_SIZEOF(EffCmdLoopStartExec, 0x24);

class EffCmdWaitExec : public EffectCommandExec {
  public:
    EffCmdWaitExec(const UnkCommandData *command, unsigned int *flags, EffectCommandExec *last);
    virtual ~EffCmdWaitExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ float m_elapsed;
    /* 0x1C */ float m_duration;
    /* 0x20 */ EffectCommandExec *m_last;
};
CHECK_SIZEOF(EffCmdWaitExec, 0x24);

class EffCmdGlareSWExec : public EffectCommandExec {
  public:
    EffCmdGlareSWExec(const UnkCommandData *command, unsigned int *flags);
    virtual ~EffCmdGlareSWExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
};
CHECK_SIZEOF(EffCmdGlareSWExec, 0x18);

class EffCmdSingleExec : public EffectCommandExec {
  public:
    EffCmdSingleExec(const UnkCommandData *command, unsigned int *flags, float *target);
    virtual ~EffCmdSingleExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ float *m_target;
    /* 0x1C */ float m_elapsed;
    /* 0x20 */ float m_duration;
    /* 0x24 */ float m_rate;
};
CHECK_SIZEOF(EffCmdSingleExec, 0x28);

class EffCmdVector2Exec : public EffectCommandExec {
  public:
    EffCmdVector2Exec(const UnkCommandData *command, unsigned int *flags, Fn8040Vector3 *target);
    virtual ~EffCmdVector2Exec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ Fn8040Vector3 *m_target;
    /* 0x1C */ float m_elapsed;
    /* 0x20 */ float m_duration;
    /* 0x24 */ Fn8040Vector3 m_rate;
};
CHECK_SIZEOF(EffCmdVector2Exec, 0x30);

class EffCmdVector3Exec : public EffectCommandExec {
  public:
    EffCmdVector3Exec(const UnkCommandData *command, unsigned int *flags, Fn8040Vector3 *target);
    virtual ~EffCmdVector3Exec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ Fn8040Vector3 *m_target;
    /* 0x1C */ float m_elapsed;
    /* 0x20 */ float m_duration;
    /* 0x24 */ Fn8040Vector3 m_rate;
};
CHECK_SIZEOF(EffCmdVector3Exec, 0x30);

/* Class identities and inheritance are recorded in the original RTTI. */
class EffCmd3DPosExec : public EffCmdVector3Exec {
  public:
    EffCmd3DPosExec(const UnkCommandData *command, unsigned int *flags, Fn8040Vector3 *target)
        : EffCmdVector3Exec(command, flags, target) {}
    virtual ~EffCmd3DPosExec();
};
CHECK_SIZEOF(EffCmd3DPosExec, 0x30);

class EffCmd3DRotExec : public EffCmdVector3Exec {
  public:
    EffCmd3DRotExec(const UnkCommandData *command, unsigned int *flags, Fn8040Vector3 *target)
        : EffCmdVector3Exec(command, flags, target) {}
    virtual ~EffCmd3DRotExec();
};
CHECK_SIZEOF(EffCmd3DRotExec, 0x30);

class EffCmd3DVelExec : public EffCmdVector3Exec {
  public:
    EffCmd3DVelExec(const UnkCommandData *command, unsigned int *flags, Fn8040Vector3 *target)
        : EffCmdVector3Exec(command, flags, target) {}
    virtual ~EffCmd3DVelExec();
};
CHECK_SIZEOF(EffCmd3DVelExec, 0x30);

class EffCmdMatAlphaExec : public EffectCommandExec {
  public:
    EffCmdMatAlphaExec(const UnkCommandData *command, unsigned int *flags, int *target);
    virtual ~EffCmdMatAlphaExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ int *m_target;
    /* 0x1C */ float m_elapsed;
    /* 0x20 */ float m_duration;
    /* 0x24 */ int m_rate;
};
CHECK_SIZEOF(EffCmdMatAlphaExec, 0x28);

class EffCmdMatColorExec : public EffectCommandExec {
  public:
    EffCmdMatColorExec(const UnkCommandData *command, unsigned int *flags, int *target);
    virtual ~EffCmdMatColorExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ int *m_target;
    /* 0x1C */ float m_elapsed;
    /* 0x20 */ float m_duration;
    /* 0x24 */ int m_rate[3];
};
CHECK_SIZEOF(EffCmdMatColorExec, 0x30);

class EffCmd2DSclExec : public EffectCommandExec {
  public:
    EffCmd2DSclExec(const UnkCommandData *command, unsigned int *flags, Fn8040Vector3 *target);
    virtual ~EffCmd2DSclExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ Fn8040Vector3 *m_target;
    /* 0x1C */ float m_elapsed;
    /* 0x20 */ float m_duration;
    /* 0x24 */ Fn8040Vector3 m_rate;
};
CHECK_SIZEOF(EffCmd2DSclExec, 0x30);

class EffCmd3DSclExec : public EffectCommandExec {
  public:
    EffCmd3DSclExec(const UnkCommandData *command, unsigned int *flags, Fn8040Vector3 *target);
    virtual ~EffCmd3DSclExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ Fn8040Vector3 *m_target;
    /* 0x1C */ float m_elapsed;
    /* 0x20 */ float m_duration;
    /* 0x24 */ Fn8040Vector3 m_rate;
};
CHECK_SIZEOF(EffCmd3DSclExec, 0x30);

class EffCmdPtnAnimExec : public EffectCommandExec {
  public:
    EffCmdPtnAnimExec(const UnkCommandData *command, unsigned int *flags, Fn8040Vector2 *lower,
                      Fn8040Vector2 *upper);
    virtual ~EffCmdPtnAnimExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ Fn8040Vector2 *m_lower;
    /* 0x1C */ Fn8040Vector2 *m_upper;
    /* 0x20 */ unsigned char m_current;
    /* 0x21 */ unsigned char m_start;
    /* 0x22 */ unsigned char m_end;
    /* 0x23 */ unsigned char m_divisions;
    /* 0x24 */ float m_elapsed;
    /* 0x28 */ float m_interval;
};
CHECK_SIZEOF(EffCmdPtnAnimExec, 0x2C);

class EffCmdRequestStopExec : public EffectCommandExec {
  public:
    EffCmdRequestStopExec(const UnkCommandData *command, unsigned int *flags,
                          EffectElement *target);
    virtual ~EffCmdRequestStopExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
};
CHECK_SIZEOF(EffCmdRequestStopExec, 0x18);

class EffCmdLaunchExec : public EffectCommandExec {
  public:
    EffCmdLaunchExec(const UnkCommandData *command, unsigned int *flags, EffectManager *manager,
                     EffectLaunchInfo *info, EffectArranger *parent);
    virtual ~EffCmdLaunchExec();
    virtual bool Update(float deltaTime);

    /* 0x14 */ unsigned int *m_flags;
    /* 0x18 */ float m_elapsed;
    /* 0x1C */ float m_duration;
    /* 0x20 */ boost::weak_ptr<EffectElement> m_child;
    /* 0x28 */ EffectArranger *m_parent;
    /* 0x2C */ Fn8040Vector3 m_translation;
    /* 0x38 */ Fn8040Vector3 m_rotation;
    /* 0x44 */ Fn8040Vector3 m_scale;
    /* 0x50 */ Fn8040Matrix34 m_transform;
};
CHECK_SIZEOF(EffCmdLaunchExec, 0x80);

} // namespace System
} // namespace Effect

#endif
