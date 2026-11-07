/* The SavePoint gadget (classes described in Game/SavePoint.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The command handler, the collision callback and the two callbacks of the SavePointSelectTask
 * are boost::function objects built with boost::bind(&SavePoint::method, this, _1); their
 * managers, invokers and copy functions come from the boost headers. */
/* Matching workaround: the unit's .data holds the base lists of the type records of TNumEdit's
 * and CharaColliBase's derived classes, which these two headers emit through their stand-in
 * inline functions (see TNumEdit.h). TNumEdit.h comes before CharaColliBaseCharInfo.h, in the
 * order their names appear in the unit's .rodata. Revisit when the original inline functions or
 * their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/SavePoint.h"
#include "Game/BASwitch.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugMissionClearCollision.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SavePointSelect.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/Player.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkManager_8006F754.h"
#include "Game/UnkManager_80074470.h"
#include "Game/UnkManager_80079808.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkManager_80117514.h"
#include "Game/UnkManager_80117578.h"
#include "Game/UnkObject_800BADE8.h"
#include "Game/UnkSingleton.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/WarpHole.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_801A1FA0.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

/* The height above the gadget, in its frame, of the position that a save records (name inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original sets it with a start-up function in .ctors (a constant float initialiser would be
 * emitted as initialised .sdata data instead), and the shared stand-in reproduces that function.
 * Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_saveHeight(10.0f);

/* What the hit callback needs to know about the gadget (placeholder name): its character info and
 * its "priority" parameter, and, as output, whether the callback saved. */
struct UnkSaveContext_80118F68 {
    UnkSaveContext_80118F68() : m_priority(0), m_saved(false) {}

    /* 0x00 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ const s32 *m_priority;
    /* 0x0C */ bool m_saved;
};
CHECK_SIZEOF(UnkSaveContext_80118F68, 0x10);

namespace SavePoint {

const char *PauseWatch::s_name = "PauseWatch::PauseWatch";

PauseWatch::PauseWatch(Task *parent, UnkBitFlags_80014E04 *flags) : Task(parent) {
    m_name = s_name;
    m_gadgetFlags = flags;
    m_flags |= 0x100;
}

PauseWatch::~PauseWatch() {
    m_gadgetFlags = 0;
}

void PauseWatch::Update(float dt) {
    if (m_gadgetFlags != 0 && lbl_8057D6A0.field_0x84 != 0) {
        m_gadgetFlags->Reset(0);
        m_gadgetFlags->Reset(1);
    }
}

} // namespace SavePoint

/* The callback that fn_8015AA84 calls for a character info it takes from the hits. Only infos whose
 * fn_80078B4C (vtable offset 0x28, called at 0x8015ABA4) returns true reach it; for the others
 * fn_8015AA84 skips the call (0x8015ABAC). It does nothing when there is no context or the
 * context's m_saved is set. While the gadget's character info is alive, it transforms the point
 * (0, s_saveHeight, 0) by the gadget's position and its y, x and z rotations, passes that point,
 * the gadget's rotation and the "priority" parameter to fn_8016F3D4 for the object of fn_80079808
 * (fn_8016F3D4 ignores an index of 32 or more), and passes sound 0xE013 to fn_801D560C. Whether or
 * not that info is alive, it then calls fn_80074AEC, fn_8016DFF8, fn_800CB508, the fn_801A23F4
 * method of the Unk801A2AF4 instance and fn_801690F8 for their managers, and passes the character
 * to fn_8015F6EC with 6, 1 and 1 when the word fn_8016F280 reads (at 0x28 of entry 0 of the table
 * of fn_80079808's object) is 100 or more, with 0, 20 and 1 when it is 50 or more and with 0, 10
 * and 1 when it is 0 or more (nothing for a negative value). Last it calls fn_80301F2C and sets
 * m_saved (behaviour inferred). */
static void fn_80118F68(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkSaveContext_80118F68 *context = static_cast<UnkSaveContext_80118F68 *>(arg);

    if (context != 0 && !context->m_saved) {
        if (boost::shared_ptr<GadgetCharInfo> info = context->m_charInfo.lock()) {
            Fn8040Vector3 position(0.0f, s_saveHeight.value, 0.0f);
            Fn8040Matrix34 matrix;

            fn_8040B6F8(&matrix);
            fn_8040B59C(&matrix, info->m_position);
            fn_8040B848(&matrix, info->m_rotation.y);
            fn_8040B884(&matrix, info->m_rotation.x);
            fn_8040B80C(&matrix, info->m_rotation.z);
            position = matrix * position;
            fn_8016F3D4(fn_80079808(), &position, &info->m_rotation, *context->m_priority);
            fn_801D560C(fn_80053164(), 0xE013, 0, 0);
        }

        fn_80074AEC(fn_8006F754());
        fn_8016DFF8(fn_8006D6A8());
        fn_800CB508(fn_800C9B84());
        UnkSingleton<Unk801A2AF4>::Instance()->fn_801A23F4();
        fn_801690F8(fn_800CB69C());

        int value = fn_8016F280(&fn_80178B5C(fn_80079808(), 0)->field_0x28);

        if (value >= 100) {
            fn_8015F6EC(hit, 6, 1, 1);
        } else if (value >= 50) {
            fn_8015F6EC(hit, 0, 20, 1);
        } else if (value >= 0) {
            fn_8015F6EC(hit, 0, 10, 1);
        }

        fn_80301F2C();
        context->m_saved = true;
    }
}

namespace SavePoint {

const char *SavePoint::s_name = "SavePoint";

SavePoint::SavePoint(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_priority(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(12, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&SavePoint::OnCommand, this, _1));

    ReadOwner(owner);

    m_model = fn_8015F9D8(0, 0xDC);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&SavePoint::OnCollision, this, _1));

    UpdateModelMatrix();
    new PauseWatch(this, &m_flags);
}

SavePoint::~SavePoint() {}

void SavePoint::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    if (m_secondModel.get() != 0) {
        m_secondModel->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0: {
        u32 ownerFlags;

        fn_800C9EC4(m_owner, &ownerFlags);

        if (ownerFlags & 0x400000) {
            if (m_secondModel.get() == 0) {
                m_secondModel = fn_8015F9D8(0, 0xDC);
                m_secondModel->fn_80162A68(1, 0);
                UpdateModelMatrix();
            }

            m_state = 2;
        } else {
            m_state = 1;
        }
        break;
    }

    case 2:
        if (m_flags.Test(1)) {
            if (CanSave()) {
                if (fn_80330CB0() && fn_8016F3A0(fn_80079808()) > 0) {
                    /* Matching workaround: object is declared before parent (here or at the top
                     * of the function gives the same code) and assigned below. Declared where it
                     * is assigned, it gets r29 instead of the original's r31, and 3 words differ
                     * (the two mr and the stw at USA 0x80118000, 0x80118008 and 0x80118010).
                     * Revisit with the original form of this block. */
                    UnkObject_800BADE8 *object;
                    Task *parent = fn_801EB304(fn_8001C4F8(), 11);
                    SavePointSelect::SavePointSelectTask *task =
                        new SavePointSelect::SavePointSelectTask(parent, m_priority);

                    task->field_0x28 = SavePointSelect::SelectCallback(
                        boost::bind(&SavePoint::OnSelect, this, _1));
                    task->field_0x34 = SavePointSelect::SelectCallback(
                        boost::bind(&SavePoint::OnCancel, this, _1));
                    m_state = 3;
                    fn_80203970(&lbl_8057D6A0.field_0xA8, true);
                    m_flags.Reset(1);

                    object = fn_800BADE8();
                    fn_801E2F28(object);
                    fn_801E2EDC(object);
                    object->field_0x30 = 0;
                }
            } else {
                m_flags.Reset(1);
            }
        }
        break;

    /* Matching workaround: the switch also lists states 1 and 3, which this function stores (USA
     * 0x80117E40 for 1, 0x80117FE0 for 3) and in which an update does nothing more. Listing them
     * gives the original's comparison order; without them the function is 2 words longer. Revisit
     * if another form gives the same order. */
    case 1:
    case 3:
        break;
    }

    if (m_flags.Test(0)) {
        m_flags.Reset(0);
        m_flags.Set(1);
    }
}

void SavePoint::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_8016108C();
    }
}

void SavePoint::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_priority, sizeof(m_priority));
    m_state = 0;
    fn_800C9EB0(owner, 0x200);
}

void SavePoint::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);

    if (m_model.get() != 0) {
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }

    if (m_secondModel.get() != 0) {
        m_secondModel->fn_801633DC(&matrix);
    }
}

void SavePoint::OnCollision(ColliDetectArgs &args) {
    if (m_state == 1) {
        UnkSaveContext_80118F68 context;

        context.m_charInfo = m_charInfo;
        context.m_priority = &m_priority;
        fn_8015AA84(&args, fn_80118F68, &context);

        if (context.m_saved) {
            if (m_secondModel.get() == 0) {
                m_secondModel = fn_8015F9D8(0, 0xDC);
                m_secondModel->fn_80162A68(1, 0);
                UpdateModelMatrix();
            }

            fn_800C9EB0(m_owner, 0x400000);
            m_state = 2;
        }
    }
}

void SavePoint::OnSelect(SavePointSelect::Args &args) {
    if (args.field_0x0 != -1) {
        Fn8040Vector3 position = *fn_8016F344(fn_80079808(), args.field_0x0, 0);
        boost::weak_ptr<CharInfo> player = fn_800A8B74(fn_80074194(), lbl_805EE588);

        if (boost::shared_ptr<CharInfo> info = player.lock()) {
            boost::shared_ptr<Player::Player> target =
                boost::dynamic_pointer_cast<Player::Player>(info);

            WarpHole::ManagerTask::Create(position, target);
        }
    }

    m_state = 2;
    m_model->fn_80161308(10.0f);
}

void SavePoint::OnCancel(SavePointSelect::Args &args) {
    m_state = 2;

    if (fn_80074470()->field_0x8 != 6) {
        fn_80203970(&lbl_8057D6A0.field_0xA8, false);
    }

    fn_800BADE8()->field_0x30 = 1;
}

void SavePoint::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0xD:
        if (m_state == 2) {
            m_flags.Set(0);
        }
        break;
    }
}

bool SavePoint::CanSave() {
    if (lbl_8057D6A0.field_0x84 != 0) {
        return false;
    }

    if (!fn_8020261C(UnkSingleton<UnkManager_80117578>::Instance())) {
        return false;
    }

    if (fn_80074470()->field_0x8 == 6) {
        return false;
    }

    if (fn_802E5810(UnkSingleton<UnkManager_80117514>::Instance())) {
        return true;
    }

    return false;
}

} // namespace SavePoint

extern "C" void fn_80117468(void) {
    fn_8006B50C(0xDC, 1);
    fn_80330CC4();
}

extern "C" void fn_80117448(void) {
    fn_80330CB8();
}

void SavePoint::SavePoint::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SavePoint(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "priority", 1, 1, 0x1F, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80526EA0 = {
    "SAVEPOINT", fn_80117468, fn_80117448, SavePoint::SavePoint::Create, 0, 0x200, 5, 1,
    0,           10,          s_paramInfo,
};
