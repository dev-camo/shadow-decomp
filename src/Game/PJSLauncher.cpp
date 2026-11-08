/* Effect::PJS::Launcher and its Loader task (classes described in Game/PJSLauncher.h). File name
 * inferred from the classes. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first). */
#include "Game/PJSLauncher.h"
#include "Game/EffectElement.h"
#include "Game/EffectManager.h"
#include "Game/ResourceOneFile.h"
#include "Game/UnkResource_80012700.h"

using Effect::PJS::Launcher;
using Effect::PJS::Loader;
using Effect::System::EffectElement;

boost::weak_ptr<EffectElement> fn_80305900(Launcher *launcher, const Fn8040Matrix34 *matrix, int id,
                                           const float *timeScale) {
    boost::weak_ptr<EffectElement> result;

    result = fn_800A31F0()->Create(launcher, id);

    boost::shared_ptr<EffectElement> effect(result.lock());

    if (effect.get() != 0) {
        effect->SetMatrixSource(matrix);
        effect->ClearFlags(Effect::System::ELEMENT_FLAG_4);

        if (timeScale != 0) {
            effect->SetTimeScaleSource(timeScale);
        }
    }

    return result;
}

boost::weak_ptr<EffectElement> fn_803057EC(Launcher *launcher, const Fn8040Vector3 *translation,
                                           const Fn8040Vector3 *rotation, int id,
                                           const float *timeScale) {
    boost::weak_ptr<EffectElement> result;

    result = fn_800A31F0()->Create(launcher, id);

    boost::shared_ptr<EffectElement> effect(result.lock());

    if (effect.get() != 0) {
        effect->SetTranslationSource(translation);
        effect->SetRotationSource(rotation);
        effect->ClearFlags(Effect::System::ELEMENT_FLAG_4);

        if (timeScale != 0) {
            effect->SetTimeScaleSource(timeScale);
        }
    }

    return result;
}

boost::weak_ptr<EffectElement> fn_803056F8(Launcher *launcher, int id, const float *timeScale) {
    boost::weak_ptr<EffectElement> result;

    result = fn_800A31F0()->Create(launcher, id);

    boost::shared_ptr<EffectElement> effect(result.lock());

    if (effect.get() != 0) {
        effect->ClearFlags(Effect::System::ELEMENT_FLAG_4);

        if (timeScale != 0) {
            effect->SetTimeScaleSource(timeScale);
        }
    }

    return result;
}

boost::weak_ptr<EffectElement> fn_803055F8(Launcher *launcher, const Fn8040Matrix34 *matrix, int id,
                                           const float *timeScale) {
    boost::weak_ptr<EffectElement> result;

    result = fn_800A31F0()->Create(launcher, id);

    boost::shared_ptr<EffectElement> effect(result.lock());

    if (effect.get() != 0) {
        fn_80425E68(effect.get(), *matrix);
        effect->ClearFlags(Effect::System::ELEMENT_FLAG_4);

        if (timeScale != 0) {
            effect->SetTimeScaleSource(timeScale);
        }
    }

    return result;
}

boost::weak_ptr<EffectElement> fn_803054D0(Launcher *launcher, const Fn8040Vector3 *position,
                                           const UnkRotation3 *rotation, int id,
                                           const float *timeScale) {
    boost::weak_ptr<EffectElement> result;

    result = fn_800A31F0()->Create(launcher, id);

    boost::shared_ptr<EffectElement> effect(result.lock());

    if (effect.get() != 0) {
        fn_80425E4C(effect.get(), position);
        fn_80425D94(effect.get(), Fn8040Vector3(rotation->x, rotation->y, 0.0f));
        effect->ClearFlags(Effect::System::ELEMENT_FLAG_4);

        if (timeScale != 0) {
            effect->SetTimeScaleSource(timeScale);
        }
    }

    return result;
}

boost::weak_ptr<EffectElement> fn_8030536C(Launcher *launcher, const Fn8040Vector3 *position,
                                           const Fn8040Vector3 *direction, int id,
                                           const float *timeScale) {
    boost::weak_ptr<EffectElement> result;

    result = fn_800A31F0()->Create(launcher, id);

    boost::shared_ptr<EffectElement> effect(result.lock());

    if (effect.get() != 0) {
        Fn8040Vector2 angles = fn_8040F664(fn_80016284(*direction));

        fn_80425E4C(effect.get(), position);
        fn_80425D94(effect.get(), Fn8040Vector3(angles.x, angles.y, 0.0f));
        effect->ClearFlags(Effect::System::ELEMENT_FLAG_4);

        if (timeScale != 0) {
            effect->SetTimeScaleSource(timeScale);
        }
    }

    return result;
}

Launcher::Launcher(int key, ResourceOneFile *resource)
    : EffectLaunchInfoUSA(0, resource), m_resourceKey(key), field_0x10(0) {}

Launcher::~Launcher() {}

bool fn_803052B4(const Launcher *launcher) {
    return launcher->field_0x4 != 0;
}

/* The deleter of the launchers' shared pointers: passes the launcher's key to fn_800127F8 and then
 * deletes the launcher (behaviour from the bodies). Only when a resource is registered under the
 * key does fn_800127F8 call its virtual slot 0x10, delete it and remove its entry; for an absent
 * key it branches past that cleanup (0x80012838). */
static void fn_80305260(Launcher *launcher) {
    fn_800127F8(fn_800122D4(), launcher->m_resourceKey);
    delete launcher;
}

Loader::Loader(const boost::shared_ptr<Launcher> &launcher)
    : Task(fn_801EB304(fn_8001C4F8(), 3)), m_launcher(launcher) {
    m_name = "Launcher Loader";
}

Loader::~Loader() {}

void Loader::Update(float dt) {
    UnkResourceManager_800122D4 *manager = fn_800122D4();

    if (fn_80012408(manager, m_launcher->m_resourceKey) == 3) {
        m_launcher->field_0x4 = static_cast<Effect::System::UnkEffectFileData *>(
            fn_8001277C(manager, m_launcher->m_resourceKey, 2));
        m_flags |= 1;
    }
}

boost::shared_ptr<Launcher> fn_80304F40(const char *path) {
    UnkResourceManager_800122D4 *manager = fn_800122D4();
    ResourceOneFile *resource = new ResourceOneFile(path, false);
    int key = fn_800128D8(manager, resource);
    boost::shared_ptr<Launcher> launcher(new Launcher(key, resource), fn_80305260);

    /* The new task is not stored; its constructor creates it under the layer-3 parent task. */
    new Loader(launcher);

    return launcher;
}
