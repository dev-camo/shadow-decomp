#ifndef GAME_EFFECTCOMMANDEXEC_H
#define GAME_EFFECTCOMMANDEXEC_H

#include "Game/fn_8042A444.h"
#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

namespace Effect {
namespace System {

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

/* Class names are recorded in the original RTTI. Member names are inferred from use. */
class EffectCommandExec {
  public:
    EffectCommandExec(const UnkCommandData *command)
        : m_command(command), m_prev(0), m_next(0), field_0x10(0) {}
    virtual ~EffectCommandExec();
    /* Advances a command; false removes it from the executor list (name inferred). */
    virtual bool Update(float deltaTime) = 0;

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

} // namespace System
} // namespace Effect

#endif
