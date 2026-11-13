/* Game/PlayerOption.cpp (the file name is not known: it follows the names of the file's two
 * classes, Player::ExternalOption and Player::InternalOption). Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first). */
#include "Game/PlayerOption.h"
#include "Game/CharCommand.h"
#include "Game/CharInfo.h"
#include "Game/PlayerBase.h"
#include "layout_check.h"

namespace Player {

/* A character info that holds a shared pointer to a player (class name from RTTI). Its
 * constructor copies the player's word at 0x5C, the characters at 0x3C and the word at 0x64. It
 * passes the commands that it is sent on to the player. Its first three queries return true and
 * the last two return whether the player's word at 0x5C is 1. */
class InternalOption : public PlayerObject {
  public:
    InternalOption(const boost::shared_ptr<PlayerBase> &player, int arg1);
    virtual ~InternalOption();
    virtual void fn_801DC980(CharCommand *command);
    virtual void fn_801DC97C(CharCommand *command);
    virtual bool fn_80078B2C() {
        return true;
    }
    virtual bool fn_80078B34() {
        return true;
    }
    virtual bool fn_80078B3C() {
        return true;
    }
    virtual bool fn_80078B44() {
        return m_player->field_0x5C == 1;
    }
    virtual bool fn_80078B4C() {
        return m_player->field_0x5C == 1;
    }

    /* 0x68 */ boost::shared_ptr<PlayerBase> m_player; /* name inferred */
};
CHECK_SIZEOF(InternalOption, 0x70);

/* A second character info of the same shape (class name from RTTI). It differs from
 * InternalOption in fn_801DC97C, which calls the command's fn_801D9CFC with a weak pointer to
 * the player where InternalOption passes the command on, and in its queries: only the second
 * returns true. */
class ExternalOption : public PlayerObject {
  public:
    ExternalOption(const boost::shared_ptr<PlayerBase> &player, int arg1);
    virtual ~ExternalOption();
    virtual void fn_801DC980(CharCommand *command);
    virtual void fn_801DC97C(CharCommand *command);
    virtual bool fn_80078B2C() {
        return false;
    }
    virtual bool fn_80078B34() {
        return true;
    }
    virtual bool fn_80078B3C() {
        return false;
    }
    virtual bool fn_80078B44() {
        return false;
    }
    virtual bool fn_80078B4C() {
        return false;
    }

    /* 0x68 */ boost::shared_ptr<PlayerBase> m_player; /* name inferred */
};
CHECK_SIZEOF(ExternalOption, 0x70);

InternalOption::InternalOption(const boost::shared_ptr<PlayerBase> &player, int arg1)
    : PlayerObject(arg1), m_player(player) {
    SetField0x5C(m_player->GetField0x5C());
    fn_801DC984(m_player->field_0x3C);
    SetField0x64(m_player->GetField0x64());
}

InternalOption::~InternalOption() {}

void InternalOption::fn_801DC980(CharCommand *command) {
    m_player->fn_801DC980(command);
}

/* When the player has an object at 0x22C, a command of type 0x1C goes to fn_800A78FC first, and a
 * command with bit 1 of its flags set goes no further. */
void InternalOption::fn_801DC97C(CharCommand *command) {
    if (m_player->field_0x22C != 0) {
        if (command->m_type == 0x1C) {
            fn_800A78FC(m_player->field_0x22C, command);
        }
        if (command->m_flags & 2) {
            return;
        }
    }
    m_player->fn_801DC97C(command);
}

ExternalOption::ExternalOption(const boost::shared_ptr<PlayerBase> &player, int arg1)
    : PlayerObject(arg1), m_player(player) {
    SetField0x5C(m_player->GetField0x5C());
    fn_801DC984(m_player->field_0x3C);
    SetField0x64(m_player->GetField0x64());
}

ExternalOption::~ExternalOption() {}

void ExternalOption::fn_801DC980(CharCommand *command) {
    m_player->fn_801DC980(command);
}

/* As InternalOption's, up to the last call. */
void ExternalOption::fn_801DC97C(CharCommand *command) {
    if (m_player->field_0x22C != 0) {
        if (command->m_type == 0x1C) {
            fn_800A78FC(m_player->field_0x22C, command);
        }
        if (command->m_flags & 2) {
            return;
        }
    }
    command->fn_801D9CFC(m_player);
}

} // namespace Player

UnkPlayerOptions_800AAF64::UnkPlayerOptions_800AAF64(
    const boost::shared_ptr<Player::PlayerBase> &player)
    : m_player(player) {
    m_options[0] = boost::shared_ptr<CharInfo>(new Player::InternalOption(m_player, 2));
    m_options[1] = boost::shared_ptr<CharInfo>(new Player::ExternalOption(m_player, 3));
}

UnkPlayerOptions_800AAF64::~UnkPlayerOptions_800AAF64() {}

void UnkPlayerOptions_800AAF64::fn_800AAE14() {
    for (int i = 0; i < 2; i++) {
        boost::shared_ptr<CharInfo> option(m_options[i]);
        option->m_position = m_player->m_position;
        option->m_rotation = m_player->m_rotation;
        option->field_0x18 = m_player->field_0x18;
        option->field_0x24 = m_player->field_0x24;
        option->field_0x30 = m_player->field_0x30;
        option->SetField0x60(m_player->GetField0x60());
    }
}

boost::weak_ptr<CharInfo> UnkPlayerOptions_800AAF64::fn_800AADA4(int index) {
    return m_options[index];
}

void UnkPlayerOptions_800AAF64::fn_800AAD44(int arg0) {
    for (int i = 0; i < 2; i++) {
        m_options[i]->fn_8007A4CC(arg0);
    }
}
