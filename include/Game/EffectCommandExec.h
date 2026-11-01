#ifndef GAME_EFFECTCOMMANDEXEC_H
#define GAME_EFFECTCOMMANDEXEC_H

#include "Game/fn_8042A444.h"
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

} // namespace System
} // namespace Effect

#endif
