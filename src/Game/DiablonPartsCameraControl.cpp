/* The original file name and both class names are preserved in RTTI. Method and member names,
 * parameter reference spellings and state type are inferred. */
#include "Game/DiablonPartsCameraControl.h"
#include "Game/AngleConversion.h"
#include "Game/Diablon.h"
#include "Game/LamborghiniCamera.h"
#include "Game/Task.h"

namespace {

class UpdateTask : public Task {
  public:
    UpdateTask(UpdateTask **owner, Boss::Camera::Lamborghini *camera);
    virtual ~UpdateTask();
    virtual void Update(float dt);

    /* The owner slot is cleared when the task is destroyed; camera is borrowed. */
    /* 0x28 */ UpdateTask **m_owner;
    /* 0x2C */ Boss::Camera::Lamborghini *m_camera;

  private:
    /* A copy would retain the original owner's backpointer and task-tree links. */
    UpdateTask(const UpdateTask &);
    UpdateTask &operator=(const UpdateTask &);
};
CHECK_SIZEOF(UpdateTask, 0x30);

} // namespace

namespace Boss {
namespace Diablon {
namespace Parts {

class CameraControl : public PartsBase {
  public:
    CameraControl(PartsBase *parent, const boost::shared_ptr<Diablon> &diablon)
        : PartsBase(parent, "CameraControl", diablon), m_camera(0) {
        m_state = 0;
    }
    virtual ~CameraControl();
    virtual void Update(float dt);
    virtual void OnAction(const Action &action);

    /* 0x24 */ s32 m_state;
    /* 0x28 */ Camera::Lamborghini *m_camera;
    /* The original constructor leaves this slot unwritten. UpdateTask installs itself here
     * when created, and clears the slot on destruction. */
    /* 0x2C */ UpdateTask *m_task;

  private:
    /* A copy would share the owned camera and the task's owner slot. */
    CameraControl(const CameraControl &);
    CameraControl &operator=(const CameraControl &);
};
CHECK_SIZEOF(CameraControl, 0x30);

} // namespace Parts
} // namespace Diablon
} // namespace Boss

namespace {

UpdateTask::UpdateTask(UpdateTask **owner, Boss::Camera::Lamborghini *camera)
    : Task(fn_801EB304(fn_8001C4F8(), 15)), m_owner(owner), m_camera(camera) {
    m_name = "LamborghiniUpdate";
    if (m_owner != 0) {
        *m_owner = this;
    }
}

UpdateTask::~UpdateTask() {
    if (m_owner != 0) {
        *m_owner = 0;
    }
}

void UpdateTask::Update(float dt) {
    if (m_camera != 0) {
        m_camera->Update(dt);
    }
}

} // namespace

namespace Boss {
namespace Diablon {
namespace Parts {

CameraControl::~CameraControl() {
    if (m_camera != 0) {
        m_camera->Stop();
        delete m_camera;
    }
    if (m_task != 0) {
        m_task->m_camera = 0;
        m_task->m_owner = 0;
        m_task->m_flags |= 1;
    }
}

void CameraControl::Update(float dt) {
    switch (m_state) {
    case 0:
        if (!m_diablon->m_target.expired()) {
            m_camera = new Camera::Lamborghini(m_diablon->m_target, m_diablon);
            new UpdateTask(&m_task, m_camera);
            m_camera->SetFirstOffset(Fn8040Vector3(0.0f, 5.0f, 0.0f));
            m_camera->SetFirstLimits(Fn8040Vector2(10.0f, 20.0f));
            m_camera->SetSecondOffset(Fn8040Vector3(0.0f, 40.0f, 0.0f));
            m_camera->SetSecondLimits(Fn8040Vector2(60.0f, 10.0f));
            m_camera->Start(0);
            ++m_state;
        }
        break;
    }
    if (m_camera != 0) {
        boost::shared_ptr<CharInfo> target(m_diablon->m_target.lock());
        if (target) {
            float height = (target->m_position.y - 5.0f) / 100.0f;
            height = fn_80070460(height, 0.0f, 1.0f);
            m_camera->SetPitchLimits(DegreesToRadians(-5.0f) + height * DegreesToRadians(-40.0f),
                                     DegreesToRadians(15.0f));
        }
    }
}

void CameraControl::OnAction(const Action &action) {
    switch (action.m_type) {
    /* The dispatch covers contiguous cases containing 15 through 17; its full extent is unknown. */
    case 15:
    case 17:
        break;
    case 16:
        if (m_camera != 0) {
            m_camera->SetSecondOffset(Fn8040Vector3(0.0f, 10.0f, 0.0f));
            m_camera->SetSecondLimits(Fn8040Vector2(80.0f, 10.0f));
        }
        break;
    }
}

} // namespace Parts
} // namespace Diablon
} // namespace Boss

extern "C" Boss::Diablon::Parts::PartsBase *
fn_80322590(Boss::Diablon::Parts::PartsBase *parent,
            const boost::shared_ptr<Boss::Diablon::Diablon> &diablon) {
    return new Boss::Diablon::Parts::CameraControl(parent, diablon);
}
