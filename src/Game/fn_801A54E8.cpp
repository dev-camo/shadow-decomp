/* EnemyMoveCommon (RTTI name) and collision helpers; original filename unknown.
 * NonMatching: vector-return copies, collision-query code and metadata still differ. */
#include "Game/EnemyMoveCommon.h"
#include "Game/Enemy.h"
#include "Game/EnemyParaFunc.h"
#include "Game/EnemyStatFunc.h"
#include "Game/EnemyDispFunc.h"
#include "Game/UnkGlobal_8057D6A0.h"

bool lbl_805E81D0 = false;

bool fn_801A61D0(const UnkProbeFilterRecord *record);
unsigned int fn_801A609C(const Fn8040Vector3 *position, float *height, Fn8040Vector3 *normal,
                         int flags, int arg);

bool fn_801A61D0(const UnkProbeFilterRecord *record) {
    return (record->flags & 0x1AB3) != 0;
}

unsigned int fn_801A609C(const Fn8040Vector3 *position, float *height, Fn8040Vector3 *normal,
                         int flags, int arg) {
    Fn8040Vector3 end(position->x, position->y - 10000.0f, position->z);
    UnkSegment_80020A20 segment(*position, end);
    UnkProbeList_80021FE4 *hits = fn_80021FE4(fn_80020D40(), &segment, flags, arg, fn_801A61D0, 0);
    unsigned int result = 0;
    if (hits != 0 && hits->field_0x4 > 0) {
        for (UnkProbeIterator_80027374 it(hits->fn_8001EE04()); it.node != hits->fn_8001EDEC();
             fn_80027398(&it)) {
            UnkProbeHit_8001E9BC *hit = it.node->hit;
            if (hit != 0 && fn_8001E9F0(hit) != 3) {
                if (height != 0) {
                    *height = fn_8001E9BC(hit)->y;
                }
                if (normal != 0) {
                    fn_800091D0(normal, fn_8001E9C4(hit));
                }
                result = fn_80026588(it.node);
                break;
            }
        }
        delete hits;
    }
    return result;
}

void Unk801A5E50::fn_801A5EA8(const Fn8040Vector3 &position) {
    fn_8040CE60(&field_0x14);
    bool touched = false;
    const Fn8040Vector3 &displayPosition = m_enemy->m_display->fn_8019B3DC();
    Fn8040Vector3 differenceTemporary;
    fn_8040CD74(&differenceTemporary, &position, &displayPosition);
    Fn8040Vector3 difference;
    fn_800091EC(&difference, &differenceTemporary);
    Fn8040Matrix34 *matrix = m_enemy->m_display->fn_8019448C(m_enemy->m_parameter->fn_80192B58());
    if (matrix != 0) {
        const Fn8040Vector3 &translation = fn_8040FB08(matrix);
        Fn8040Vector3 center;
        fn_8040CDD0(&center, &translation, &difference);
        fn_800091D0(&m_sphere.m_center, &center);
    } else {
        fn_800091D0(&m_sphere.m_center, fn_8017C690(m_enemy));
        m_sphere.m_center.y += m_enemy->m_parameter->fn_8019B48C()[3];
    }
    UnkWorld_80020D40 *world = fn_80020D40();
    UnkProbeList_80021FE4 *hits = fn_80021CA4(world, &m_sphere, 1, 0, 0, 0, 0);
    if (hits != 0) {
        fn_8001E9F8(hits);
        for (UnkProbeIterator_80027374 it(hits->fn_8001EE04()); it.node != hits->fn_8001EDEC();
             fn_80027398(&it)) {
            UnkProbeHit_8001E9BC *hit = it.node->hit;
            if (hit != 0) {
                Fn8040Vector3 normal;
                fn_800091EC(&normal, fn_8001E9C4(hit));
                if (normal.y < 0.5f) {
                    const Fn8040Vector3 &displacementTemporary =
                        fn_8001E858(world, hit, &m_sphere.m_center, m_sphere.m_radius);
                    Fn8040Vector3 displacement;
                    fn_800091EC(&displacement, &displacementTemporary);
                    displacement.y = 0.0f;
                    fn_8040CFF8(&field_0x14, &displacement);
                    touched = true;
                }
            }
        }
        delete hits;
    }
    field_0x21 = field_0x20;
    field_0x20 = touched;
}

Unk801A5E50::Unk801A5E50() : m_enemy(0) {
    field_0x14.Set(0.0f, 0.0f, 0.0f);
    field_0x20 = field_0x21 = lbl_805E81D0;
}

void Unk801A5E50::fn_801A5DF0(Enemy *enemy) {
    m_enemy = enemy;
    fn_800091D0(&m_sphere.m_center, fn_8017C690(m_enemy));
    m_sphere.m_radius = m_enemy->m_parameter->fn_8019B48C()[0];
}

EnemyMoveCommon::EnemyMoveCommon() {
    field_0x8.fn_801A5DF0(m_enemy);
    fn_800091D0(&field_0x2C, fn_8017C690(m_enemy));
    field_0x38 = 0.0f;
    field_0x3C = -999999.0f;
    field_0x4C = 15;
    Fn8040Vector3 up(0.0f, 1.0f, 0.0f);
    fn_800091D0(&field_0x40, &up);
    field_0x5C = true;
    field_0x5D = false;
    field_0x5E = false;
    field_0x5F = false;
    field_0x60 = false;
}

void EnemyMoveCommon::fn_8019B438() {
    fn_800091D0(&field_0x2C, fn_8017C690(m_enemy));
    fn_8019B43C(0.0f);
}

const Fn8040Vector3 &Unk801A5E50::fn_801A5CB4() const {
    return field_0x14;
}

void EnemyMoveCommon::fn_8019B43C(float delta) {
    if (fn_8019B450()) {
        m_enemy->m_boundary.fn_801A687C(*fn_8017C690(m_enemy));
    }
    if (field_0x5D) {
        const Fn8040Vector3 &displacement = fn_801A54E8(delta);
        fn_8040CFF8(fn_80181F0C(m_enemy), &displacement);
    }
    const Fn8040Vector3 &displayDisplacement = m_enemy->m_display->fn_8019B3BC();
    Fn8040Vector3 movementTemporary;
    fn_8040CDD0(&movementTemporary, fn_80181F0C(m_enemy), &displayDisplacement);
    Fn8040Vector3 movement;
    fn_800091EC(&movement, &movementTemporary);
    Fn8040Vector3 candidate;
    fn_8040CDD0(&candidate, fn_8017C690(m_enemy), &movement);
    fn_800091D0(&field_0x2C, &candidate);
    if (!m_enemy->m_boundary.fn_801A6944(field_0x2C)) {
        Fn8040Vector3 fromOriginTemporary;
        fn_8040CD74(&fromOriginTemporary, &field_0x2C, &m_enemy->field_0x48);
        Fn8040Vector3 fromOrigin;
        fn_800091EC(&fromOrigin, &fromOriginTemporary);
        fromOrigin.y = 0.0f;
        fn_8040CE74(&fromOrigin);
        fn_8040CE38(&fromOrigin, fn_8018346C(m_enemy));
        field_0x2C.x = fromOrigin.x + m_enemy->field_0x48.x;
        field_0x2C.z = fromOrigin.z + m_enemy->field_0x48.z;
        movement.x = field_0x2C.x - fn_8017C690(m_enemy)->x;
        movement.z = field_0x2C.z - fn_8017C690(m_enemy)->z;
    }
    float distanceSquared = fn_8040CF2C(&movement);
    if (field_0x5C) {
        int mode;
        if (field_0x8.fn_801A5758() || !fn_8019588C() || distanceSquared > 0.5f) {
            mode = 2;
        } else if (distanceSquared > 0.1f) {
            mode = 1;
        } else {
            mode = 0;
        }
        unsigned int phase = lbl_8057D6A0.field_0x90 + m_enemy->field_0x28;
        if (mode == 2 || (mode == 1 && (phase & 1) != 0) || (mode == 0 && (phase & 15) == 0)) {
            field_0x8.fn_801A5EA8(field_0x2C);
            fn_8040CFF8(&movement, &field_0x8.fn_801A5CB4());
        }
    }
    movement.y -= delta * m_enemy->m_parameter->fn_8019B4E0();
    Fn8040Vector3 positionTemporary;
    fn_8040CDD0(&positionTemporary, fn_8017C690(m_enemy), &movement);
    Fn8040Vector3 probePosition;
    fn_800091EC(&probePosition, &positionTemporary);
    probePosition.y += m_enemy->m_parameter->fn_8019B48C()[2];
    Fn8040Vector3 position;
    fn_800091EC(&position, &probePosition);
    probePosition.y += 10.0f;
    field_0x3C = -999999.0f;
    unsigned int flags = fn_801A609C(&probePosition, &field_0x3C, &field_0x40, 3, 12);
    if (flags != 0) {
        position.y = position.y > field_0x3C ? position.y : field_0x3C;
    } else if (!fn_8019B450()) {
        position.x = fn_8017C690(m_enemy)->x;
        position.z = fn_8017C690(m_enemy)->z;
        fn_800091D0(&probePosition, &position);
        field_0x3C = -999999.0f;
        flags = fn_801A609C(&probePosition, &field_0x3C, &field_0x40, 3, 12);
        if (flags != 0) {
            position.y = position.y > field_0x3C ? position.y : field_0x3C;
        }
    }
    field_0x38 = position.y - field_0x3C;
    position.y -= m_enemy->m_parameter->fn_8019B48C()[2];
    if ((flags & 0x20) != 0) {
        field_0x60 = true;
        field_0x5F = field_0x38 < 5.0f;
    } else {
        field_0x60 = false;
        field_0x5F = false;
    }
    field_0x4C = flags != 0 ? flags >> 28 : 15;
    fn_800091D0(&field_0x2C, &position);
    if (!m_enemy->m_status->fn_8017B83C()->Test(10)) {
        fn_800091D0(fn_8017C690(m_enemy), &field_0x2C);
    }
    m_enemy->m_display->fn_8017F234(1);
}

bool EnemyMoveCommon::fn_8019588C() {
    return field_0x38 < 5.0f;
}

bool Unk801A5E50::fn_801A5758() const {
    return field_0x20;
}

bool EnemyMoveCommon::fn_8019B448() {
    return field_0x8.fn_801A5758();
}

void EnemyMoveCommon::fn_8019B460() {
    field_0x5D = true;
    field_0x5E = true;
    Fn8040Vector3 direction(0.0f, 0.0f, m_enemy->m_status->fn_8017B780() == 0 ? 1.0f : -1.0f);
    fn_800091D0(&field_0x50, &direction);
    Fn8040Vector3 rotated;
    fn_8040D0AC(&rotated, &field_0x50, fn_8017C684(m_enemy)->y);
    fn_800091D0(&field_0x50, &rotated);
    fn_8040CE38(&field_0x50, m_enemy->m_parameter->fn_8019B4F0()[0]);
    float factor = m_enemy->m_parameter->fn_8019B4F0()[1];
    field_0x50.y = m_enemy->m_parameter->fn_8019B4E0() * factor;
}

Fn8040Vector3 EnemyMoveCommon::fn_801A54E8(float delta) {
    Fn8040Vector3 scaled;
    fn_8040CDA8(&scaled, &field_0x50, delta);
    Fn8040Vector3 result;
    fn_800091D0(&result, &scaled);

    float vertical = field_0x50.y - delta * m_enemy->m_parameter->fn_8019B4E0();
    field_0x50.y = vertical;
    if (vertical < 0.0f) {
        field_0x50.y = 0.0f;
    }
    if (!field_0x5E && field_0x38 < 0.1f) {
        float x = field_0x50.x * 0.001f;
        field_0x50.x = x;
        if (x < 0.0001f) {
            field_0x50.x = 0.0f;
        }
        float z = field_0x50.z * 0.001f;
        field_0x50.z = z;
        if (z < 0.0001f) {
            field_0x50.z = 0.0f;
        }
    }
    if (fn_8000B044(fn_8040CF2C(&field_0x50))) {
        field_0x5D = false;
    }
    field_0x5E = false;
    return result;
}
