/* EffectManagerRW3 methods; class identity comes from RTTI, original filename unknown. */
#include "Game/EffectManagerRW3.h"
#include "Game/EffectArrangerRW3.h"
#include "Game/EffectGlobalParamRW3.h"
#include "Game/EffectLaunchInfoRW3.h"
#include "Game/EffectLensFlareRW3.h"
#include "Game/EffectObjectRW3.h"
#include "Game/EffectParticle3DRW3.h"
#include "Game/EffectParticleRW3.h"
#include "Game/EffectRayRW3.h"
#include "Game/EffectShimmerRW3.h"
#include "Game/EffectSystemRW3.h"
#include "Game/UnkEffectFileData.h"
#include "rw/rpptank.h"

namespace Effect {
namespace RW3 {

EffectManagerRW3::EffectManagerRW3() : field_0x2C(0), field_0x30(0), field_0x34(0) {}

EffectManagerRW3::~EffectManagerRW3() {
    if (field_0x34 != 0) {
        fn_80489BDC(field_0x34);
        field_0x34 = 0;
    }
}

System::EffectElement *EffectManagerRW3::CreateElement(System::UnkEffectElementData *definition,
                                                       System::EffectLaunchInfo *info) {
    EffectLaunchInfoRW3 *rwInfo = dynamic_cast<EffectLaunchInfoRW3 *>(info);
    if (rwInfo != 0) {
        switch (definition->field_0x20) {
        case 2:
            return new EffectParticleRW3(static_cast<System::UnkEffectElementData2 *>(definition),
                                         rwInfo, this);
        case 3:
            return new EffectArrangerRW3(static_cast<System::UnkEffectElementData3 *>(definition),
                                         rwInfo, this);
        case 4:
            return new EffectObjectRW3(static_cast<System::UnkEffectElementData4 *>(definition),
                                       rwInfo, this);
        case 5:
            return new EffectParticle3DRW3(static_cast<System::UnkEffectElementData5 *>(definition),
                                           rwInfo, this);
        case 6:
            return new EffectRayRW3(static_cast<System::UnkEffectElementData6 *>(definition),
                                    rwInfo, this);
        case 8:
            return new EffectGlobalParamRW3(
                static_cast<System::UnkEffectElementData8 *>(definition), rwInfo, this);
        case 9:
            return new EffectLensFlareRW3(static_cast<System::UnkEffectElementData9 *>(definition),
                                          rwInfo, this);
        case 10:
            return new EffectShimmerRW3(static_cast<System::UnkEffectElementData10 *>(definition),
                                        rwInfo, this);
        }
    }
    return System::EffectManager::CreateElement(definition, info);
}

void EffectManagerRW3::fn_804325E8() {
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system != 0) {
        RwCamera *camera = system->GetCamera();
        if (camera != 0) {
            int width = camera->frameBuffer->width;
            int height = camera->frameBuffer->height;
            RwRect rect = {0, 0, 0, 0};
            rect.w = width;
            rect.h = height;
            field_0x34 = fn_80489E74(0, 0, 0, rwRASTERTYPECAMERATEXTURE);
            fn_80489DC4(
                field_0x34,
                fn_80489E74(width, height, 0, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT8888),
                &rect);
            field_0x38 = fn_80489E74(0, 0, 0, rwRASTERTYPECAMERATEXTURE);
            fn_80489DC4(
                field_0x38,
                fn_80489E74(width, height, 0, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT8888),
                &rect);
        }
    }
}

void EffectManagerRW3::fn_80432578() {
    if (field_0x34 != 0) {
        fn_80489BDC(field_0x34->parent);
        fn_80489BDC(field_0x34);
        field_0x34 = 0;
    }
    if (field_0x38 != 0) {
        fn_80489BDC(field_0x38->parent);
        fn_80489BDC(field_0x38);
        field_0x38 = 0;
    }
}

RpAtomic *EffectManagerRW3::fn_804324C4(int count) {
    if (field_0x2C == 0) {
        return 0;
    }
    UnkEffectManagerRW3Data2C *previous = 0;
    UnkEffectManagerRW3Data2C *node = field_0x2C;
    while (node != 0) {
        if (node->count == count) {
            RpAtomic *atomic = node->atomic;
            if (previous != 0) {
                previous->next = node->next;
            } else {
                field_0x2C = node->next;
            }
            fn_8042A5D0()->Free(node);
            --field_0x30;
            return atomic;
        }
        previous = node;
        node = node->next;
    }
    return 0;
}

void EffectManagerRW3::fn_804324A0(RpAtomic *atomic, int count) {
    fn_804483E8(atomic);
}

void EffectManagerRW3::fn_804323F8() {
    UnkEffectManagerRW3Data2C *node = field_0x2C;
    UnkEffectManagerRW3Data2C *next;
    UnkEffectManagerRW3Data2C *previous = 0;
    for (; node != 0; node = next) {
        next = node->next;
        if (node->field_0x4 == 0) {
            if (node->atomic != 0) {
                fn_804483E8(node->atomic);
            }
            if (previous != 0) {
                previous->next = next;
            } else {
                field_0x2C = next;
            }
            --field_0x30;
            fn_8042A5D0()->Free(node);
        } else {
            --node->field_0x4;
            previous = node;
        }
    }
}

void EffectManagerRW3::fn_8043237C() {
    UnkEffectManagerRW3Data2C *node = field_0x2C;
    while (node != 0) {
        UnkEffectManagerRW3Data2C *next = node->next;
        if (node->atomic != 0) {
            fn_804483E8(node->atomic);
        }
        --field_0x30;
        fn_8042A5D0()->Free(node);
        node = next;
    }
    field_0x2C = 0;
}

} // namespace RW3
} // namespace Effect
