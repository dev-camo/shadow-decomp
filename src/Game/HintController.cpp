/* HintController::HintController (class name from RTTI): the HINT CONTROLLER gadget, which
 * requests the messages of the stage's hint file in turn, and the hint requests that player code
 * makes (behaviour in the header). File name inferred from the class. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first). */
#include "Game/HintController.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Npc.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkFileManager_800371D8.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkTable_8040E61C.h"
#include "Game/fn_8040_random.h"
#include <cstdio>

/* A row of message ids (placeholder name; the rows are indexed by the word at 0xAC of
 * lbl_8057D6A0). fn_802F2C1C picks one of the first three by the npc's fn_800736FC, fn_802F299C
 * and fn_802F2A9C one of the next four at random, and fn_802F2B40 takes the last. A negative id
 * requests nothing. */
struct UnkHintRow_805095A8 {
    /* 0x00 */ int field_0x0[3];
    /* 0x0C */ int field_0xC[4];
    /* 0x1C */ int field_0x1C;
};
CHECK_SIZEOF(UnkHintRow_805095A8, 0x20);

/* The last three rows are all zero: indexed, they would request message 0. */
static const UnkHintRow_805095A8 s_rows[59] = {
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{520, 530, 540}, {439, 440, 442, 443}, 1833},
    {{580, 581, 582}, {439, 440, 444, 445}, 1834},
    {{566, 567, 568}, {439, 440, 446, 447}, 1835},
    {{554, 555, 556}, {439, 440, 448, 449}, 1836},
    {{593, 594, 595}, {439, 440, 450, 451}, 1837},
    {{607, 608, 609}, {439, 440, 452, 453}, 1838},
    {{600, 601, 602}, {439, 440, 454, 455}, 1839},
    {{560, 561, 562}, {439, 440, 456, 457}, 1840},
    {{627, 628, 629}, {439, 440, 458, 459}, 1841},
    {{573, 574, 575}, {439, 440, 460, 461}, 1842},
    {{586, 587, 588}, {439, 440, 462, 463}, 1843},
    {{614, 615, 616}, {439, 440, 464, 465}, 1844},
    {{634, 635, 636}, {439, 440, 466, 467}, 1845},
    {{659, 660, 661}, {439, 440, 468, 469}, 1846},
    {{620, 621, 622}, {439, 440, 470, 471}, 1847},
    {{640, 641, 642}, {439, 440, 472, 473}, 1848},
    {{647, 648, 649}, {439, 440, 474, 475}, 1849},
    {{666, 667, 668}, {439, 440, 476, 477}, 1850},
    {{678, 679, 680}, {439, 440, 478, 479}, 1851},
    {{672, 673, 674}, {439, 440, 480, 481}, 1852},
    {{653, 654, 655}, {439, 440, 482, 483}, 1853},
    {{684, 685, 686}, {439, 440, 484, 485}, 1854},
    {{-1, -1, -1}, {439, 440, 486, 486}, -1},
    {{-1, -1, -1}, {439, 440, 487, 488}, -1},
    {{-1, -1, -1}, {439, 440, 489, 490}, -1},
    {{-1, -1, -1}, {439, 440, 491, 492}, -1},
    {{-1, -1, -1}, {439, 440, 493, 494}, -1},
    {{-1, -1, -1}, {439, 440, 495, 496}, -1},
    {{-1, -1, -1}, {439, 440, 497, 498}, -1},
    {{-1, -1, -1}, {439, 440, 499, 500}, -1},
    {{-1, -1, -1}, {439, 440, 501, 502}, -1},
    {{-1, -1, -1}, {439, 440, 503, 504}, -1},
    {{-1, -1, -1}, {439, 440, 505, 506}, -1},
    {{-1, -1, -1}, {439, 440, 507, 508}, -1},
    {{-1, -1, -1}, {439, 440, 509, 510}, -1},
    {{-1, -1, -1}, {439, 440, 511, 512}, -1},
    {{-1, -1, -1}, {439, 440, 513, 514}, -1},
    {{-1, -1, -1}, {439, 440, 515, 516}, -1},
    {{-1, -1, -1}, {439, 440, 517, 518}, -1},
    {{-1, -1, -1}, {439, 440, 519, 519}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{-1, -1, -1}, {-1, -1, -1, -1}, -1},
    {{0, 0, 0}, {0, 0, 0, 0}, 0},
    {{0, 0, 0}, {0, 0, 0, 0}, 0},
    {{0, 0, 0}, {0, 0, 0, 0}, 0},
};

/* The first message id of each npc type (the word at 0x64 of the npc), -1 for none. The entries
 * of types 15 to 19 are 0. */
static const int s_npcMessages[20] = {-1,   1681, 1694, 1707, 1720, 1733, 1746, 1759, 1785, 1798,
                                      1772, 1824, 1811, -1,   -1,   0,    0,    0,    0,    0};

/* The hint file's entries, the table built from it and the loaded file (names inferred). */
static void *s_data;
static UnkTable_8040E61C *s_table;
static const UnkHintEntry_802F2F5C *s_entries;

namespace HintController {

HintController::HintController(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0, 4, s_name);
    ReadOwner(owner);
}

HintController::~HintController() {}

void HintController::UpdateBody(float dt) {
    switch (m_state) {
    case 0:
        if (s_entries[m_index].messageId != -1) {
            m_entry = &s_entries[m_index];
            m_timer = m_entry->delay;
            m_index++;
            m_state = 1;
        } else {
            m_state = 2;
        }
        break;

    case 1:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            if (m_entry->branch >= 0) {
                fn_801FAF00(fn_80109D64(), m_entry->messageId, m_entry->branch, 0, 0);
            } else {
                switch (fn_8006D6A8()->field_0x20) {
                case 1:
                    fn_801FAF00(fn_80109D64(), m_entry->messageId, 1, 0, 0);
                    break;

                case 2:
                    fn_801FAF00(fn_80109D64(), m_entry->messageId, 2, 0, 0);
                    break;

                case 0:
                    fn_801FAF00(fn_80109D64(), m_entry->messageId, 0, 0, 0);
                    break;
                }
            }

            m_state = 0;
        }
        break;

    case 2:
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }
}

void HintController::fn_800CBC1C(float dt) {}

void HintController::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {}

void HintController::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    m_state = 0;
    m_timer = 0.0f;
    m_entry = 0;
    m_index = 0;
}

} // namespace HintController

/* The descriptor's first hook: loads stg<stage>/stg<stage>_hint.bin (the stage number is the word
 * at 0xA8 of lbl_8057D6A0) and, when the file is found, builds the table from it and keeps its
 * first array as the entries. Its signature is inferred from the descriptor's hook type
 * (GadgetTypeInfo::hookA). */
extern "C" void fn_802F2E44(void) {
    /* A function-local static: the original has s_format$132 and its guard init$133 in .sbss. */
    static const char *s_format = "stg%04d/stg%04d_hint.bin";
    char path[64];
    u32 size;

    sprintf(path, s_format, lbl_8057D6A0.field_0xA8.field_0x0, lbl_8057D6A0.field_0xA8.field_0x0);
    s_data = 0;
    s_table = 0;
    s_entries = 0;

    s_data = fn_8004509C(fn_800371D8(), path, &size);
    if (s_data != 0) {
        s_table = new UnkTable_8040E61C(s_data);
        s_entries = static_cast<const UnkHintEntry_802F2F5C *>(s_table->field_0x14);
    }
}

/* The descriptor's second hook: deletes the table and frees the file when they exist, and clears
 * the entries. Its signature is inferred from the descriptor's hook type
 * (GadgetTypeInfo::hookB). */
extern "C" void fn_802F2DEC(void) {
    if (s_table != 0) {
        delete s_table;
        s_table = 0;
    }

    if (s_data != 0) {
        fn_80049B34(s_data);
        s_data = 0;
    }

    s_entries = 0;
}

void HintController::HintController::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new HintController(parent, owner);
}

/* Returns a when a is less than b, otherwise b (name and file-local linkage inferred). */
static const int &fn_802F2D7C(const int &a, const int &b) {
    return a < b ? a : b;
}

/* Requests message messageId with branch as fn_801FAF00's third argument when messageId is not
 * negative (name and file-local linkage inferred). */
static void fn_802F2D34(int messageId, int branch) {
    if (messageId >= 0) {
        fn_801FAF00(fn_80109D64(), messageId, branch, 1, 0);
    }
}

extern "C" void fn_802F2C1C(const boost::weak_ptr<Player::PlayerBase> &player) {
    if (!player.expired()) {
        const UnkHintRow_805095A8 *row = &s_rows[lbl_8057D6A0.field_0xA8.field_0x4];
        boost::shared_ptr<Player::PlayerBase> base = player.lock();
        boost::shared_ptr<Player::Npc::Npc> npc =
            boost::dynamic_pointer_cast<Player::Npc::Npc>(base);
        int value = npc->fn_800736FC();
        int messageId = row->field_0x0[fn_802F2D7C(value, 2)];
        if (messageId < 0) {
            return;
        }

        int branch = fn_801FAAC4(npc->fn_80073A08());
        fn_802F2D34(messageId, branch);
    }
}

extern "C" void fn_802F2B40(const boost::weak_ptr<Player::PlayerBase> &player) {
    if (!player.expired()) {
        const UnkHintRow_805095A8 *row = &s_rows[lbl_8057D6A0.field_0xA8.field_0x4];
        if (row->field_0x1C >= 0) {
            boost::shared_ptr<Player::PlayerBase> base = player.lock();
            boost::shared_ptr<Player::Npc::Npc> npc =
                boost::dynamic_pointer_cast<Player::Npc::Npc>(base);
            int messageId = row->field_0x1C;
            int branch = fn_801FAAC4(npc->fn_80073A08());
            fn_802F2D34(messageId, branch);
        }
    }
}

extern "C" void fn_802F2A9C(float chance) {
    int branch = fn_801FAAC4(1);
    int messageId;
    if (fn_8040EE90() <= chance) {
        messageId = 441;
    } else {
        messageId = s_rows[lbl_8057D6A0.field_0xA8.field_0x4]
                        .field_0xC[fn_8040EA90(lbl_8057D6A0.field_0xA8.field_0x9 ? 2 : 4)];
    }

    fn_802F2D34(messageId, branch);
}

extern "C" void fn_802F299C(const boost::weak_ptr<Player::PlayerBase> &player, float chance) {
    if (!player.expired()) {
        boost::shared_ptr<Player::PlayerBase> base = player.lock();
        boost::shared_ptr<Player::Npc::Npc> npc =
            boost::dynamic_pointer_cast<Player::Npc::Npc>(base);
        int branch = fn_801FAAC4(npc->fn_80073A08());
        int messageId;
        if (fn_8040EE90() <= chance) {
            messageId = 441;
        } else {
            messageId = s_rows[lbl_8057D6A0.field_0xA8.field_0x4].field_0xC[fn_8040EA90(4)];
        }

        fn_802F2D34(messageId, branch);
    }
}

extern "C" void fn_802F28A8(const boost::shared_ptr<Player::Npc::Npc> &npc, int kind, int count) {
    int step = count - 1;
    if (step % 3 == 0) {
        int index = step / 3 % 3;
        int type = npc->field_0x64;
        if (type == 9) {
            switch (kind) {
            case 0:
                if (lbl_8057D6A0.field_0xA8.field_0x0 == 504) {
                    index += 3;
                }
                break;

            case 2:
                index += 6;
                break;
            }
        } else {
            switch (kind) {
            case 2:
                index += 3;
                break;

            case 1:
                index += 6;
                break;
            }
        }

        if (s_npcMessages[type] >= 0) {
            fn_802F2D34(index + s_npcMessages[type], fn_801FAAC4(npc->fn_80073A08()));
        }
    }
}

const char *HintController::HintController::s_name = "HintController";

GadgetTypeInfo lbl_80559FC0 = {
    "HINT CONTROLLER",
    fn_802F2E44,
    fn_802F2DEC,
    HintController::HintController::Create,
    0,
    0x800,
    0x2596,
    1,
    0,
    10,
    0,
};
