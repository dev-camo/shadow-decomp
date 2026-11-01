/* Model wrapper and items; original class and filename unknown.
 * NonMatching: bulk setup and several wrapper/item methods retain control and copy differences. */
#include "Game/fn_801A35AC.h"
#include "Game/fn_80015A7C.h"
#include "Game/fn_80038198.h"
#include "Game/fn_8003A840.h"
#include "Game/fn_80169350.h"
#include "Game/UnkResource_80012700.h"
#include "Game/fn_80038E84.h"

Unk801A35ACItem::Unk801A35ACItem()
    : m_clump(0), m_frame(0), field_0x8(0), field_0xC(0.0f), field_0x10(6), field_0x14(6) {}

Unk801A35ACItem::~Unk801A35ACItem() {
    fn_801A4144();
}

void Unk801A35ACItem::fn_801A4298(int resourceKey, int index) {
    if (m_clump == 0) {
        RpClump *source = static_cast<RpClump *>(fn_8001277C(fn_800122D4(), resourceKey, index));
        if (source != 0) {
            m_clump = fn_80457090(source);
            if (m_clump != 0) {
                m_frame = static_cast<RwFrame *>(m_clump->object.parent);
                int selector = fn_80169350(fn_80169364(m_clump));
                if (selector == 9) {
                    selector = 10;
                }
                RpWorld *world = fn_800593D0(fn_8003A840(), selector);
                if (world != 0) {
                    field_0x10 = selector;
                    field_0x14 = selector;
                }
                RpClump *clump = m_clump;
                RpWorld *current = fn_8046256C(clump);
                if (current != world) {
                    if (current != 0) {
                        fn_804623EC(current, clump);
                    }
                    fn_804622FC(world, clump);
                }
                m_parameters.field_0x24 = 8;
                fn_80038198(m_clump, &m_parameters);
            }
        }
    }
}

void Unk801A35ACItem::fn_801A41D8(const Fn8040Matrix34 &matrix) {
    if (m_frame != 0) {
        RwMatrix transform;
        transform.right.x = matrix.values[0][0];
        transform.right.y = matrix.values[1][0];
        transform.right.z = matrix.values[2][0];
        transform.up.x = matrix.values[0][1];
        transform.up.y = matrix.values[1][1];
        transform.up.z = matrix.values[2][1];
        transform.at.x = matrix.values[0][2];
        transform.at.y = matrix.values[1][2];
        transform.at.z = matrix.values[2][2];
        transform.pos.x = matrix.values[0][3];
        transform.pos.y = matrix.values[1][3];
        transform.pos.z = matrix.values[2][3];
        transform.pad3 = 0;
        transform.pad2 = 0;
        transform.pad1 = 0;
        transform.flags = 0;
        fn_8047EBA0(&transform);
        fn_804876E8(m_frame, &transform, rwCOMBINEREPLACE);
    }
}

void Unk801A35ACItem::fn_801A4144() {
    if (field_0x8 != 0) {
        delete field_0x8;
        field_0x8 = 0;
    }
    if (m_frame != 0 && m_frame->object.parent != 0) {
        fn_80487448(m_frame);
    }
    if (m_clump != 0) {
        RpWorld *world = fn_8046256C(m_clump);
        if (world != 0) {
            fn_804623EC(world, m_clump);
        }
        fn_80457468(m_clump);
        m_clump = 0;
        m_frame = 0;
    }
}

void Unk801A35ACItem::fn_801A3FB0(float value) {
    if (m_clump != 0) {
        field_0xC = value;
        m_parameters.field_0x0.alpha = field_0xC;
        if (field_0xC < 1.0f) {
            if (field_0x14 == 6) {
                field_0x14 = 10;
                RpWorld *current = fn_8046256C(m_clump);
                if (current != 0) {
                    fn_804623EC(current, m_clump);
                }
                RpWorld *world = fn_800593D0(fn_8003A840(), field_0x14);
                RpClump *clump = m_clump;
                current = fn_8046256C(clump);
                if (current != world) {
                    if (current != 0) {
                        fn_804623EC(current, clump);
                    }
                    fn_804622FC(world, clump);
                }
                if (m_frame != 0) {
                    if (field_0x8 != 0) {
                        field_0x8->fn_800159AC();
                    } else {
                        fn_8048722C(m_frame);
                    }
                }
            }
        } else if (field_0x14 != field_0x10) {
            field_0x14 = field_0x10;
            RpWorld *current = fn_8046256C(m_clump);
            if (current != 0) {
                fn_804623EC(current, m_clump);
            }
            RpWorld *world = fn_800593D0(fn_8003A840(), field_0x14);
            RpClump *clump = m_clump;
            current = fn_8046256C(clump);
            if (current != world) {
                if (current != 0) {
                    fn_804623EC(current, clump);
                }
                fn_804622FC(world, clump);
            }
            if (m_frame != 0) {
                if (field_0x8 != 0) {
                    field_0x8->fn_800159AC();
                } else {
                    fn_8048722C(m_frame);
                }
            }
        }
    }
}

void Unk801A35ACItem::fn_801A3F90(float red, float green, float blue, float alpha) {
    if (m_clump == 0) {
        return;
    }
    m_parameters.field_0x0.alpha = alpha;
    m_parameters.field_0x0.red = red;
    m_parameters.field_0x0.green = green;
    m_parameters.field_0x0.blue = blue;
}

void Unk801A35ACItem::fn_801A3F0C(_stMtnPlayWork *control) {
    if (m_clump != 0 && control != 0) {
        if (field_0x8 != 0) {
            delete field_0x8;
            field_0x8 = 0;
        }
        field_0x8 = new Unk80015A7C(m_clump, control);
    }
}

void Unk801A35ACItem::fn_801A3EAC(const Unk801A35ACParameters &parameters) {
    m_parameters = parameters;
}

Unk801A35AC::Unk801A35AC() : m_items(), m_count(0) {}

Unk801A35AC::~Unk801A35AC() {
    fn_801A3B58();
}

void Unk801A35AC::fn_801A3CE4(int resourceKey, const Unk801A35ACDescriptor &descriptor,
                              _stMtnPlayWork *control) {
    int count = descriptor.m_count;
    m_count = count;
    m_items.reset(new Unk801A35ACItem[count]);
    const Unk801A35ACEntry *entries = descriptor.m_entries;
    for (int i = 0; i < count; ++i) {
        m_items[i].fn_801A4298(resourceKey, entries[i].m_index);
    }
    if (control != 0) {
        m_items.get()->fn_801A3F0C(control);
    }
    int attachmentCount = descriptor.m_count;
    for (int i = 1; i < attachmentCount; ++i) {
        int parentFrameId = entries[i].m_parentFrameId;
        if (parentFrameId >= 0) {
            RwFrame *frame;
            if (m_items[0].m_frame != 0) {
                frame = fn_80038E84(m_items[0].m_frame, parentFrameId);
            } else {
                frame = 0;
            }
            if (frame != 0) {
                Unk801A35ACItem *item = fn_801A366C(i);
                if (item->m_frame != 0) {
                    fn_804872E0(frame, item->m_frame);
                }
            }
        } else if (control != 0) {
            m_items[i].fn_801A3F0C(control);
        }
    }
}

void Unk801A35AC::fn_801A3CA4(int resourceKey, short index, _stMtnPlayWork *control) {
    Unk801A35ACEntry entry = {0, -1};
    entry.m_index = index;
    Unk801A35ACDescriptor descriptor;
    descriptor.m_entries = &entry;
    descriptor.m_count = 1;
    fn_801A3CE4(resourceKey, descriptor, control);
}

void Unk801A35AC::fn_801A3B58() {
    for (int index = m_count - 1; index >= 0; --index) {
        Unk801A35ACItem *item = fn_801A366C(index);
        if (item->field_0x8 != 0) {
            delete item->field_0x8;
            item->field_0x8 = 0;
        }
    }
    for (int index = m_count - 1; index >= 0; --index) {
        Unk801A35ACItem *item = fn_801A366C(index);
        if (item->m_frame != 0 && item->m_frame->object.parent != 0) {
            fn_80487448(item->m_frame);
        }
    }
    m_count = 0;
    m_items.reset();
}

void Unk801A35AC::fn_801A3AFC() {
    for (int i = 0; i < m_count; ++i) {
        Unk801A35ACItem *item = fn_801A366C(i);
        if (item->m_clump != 0) {
            fn_80038F94(item->m_clump);
        }
    }
}

void Unk801A35AC::fn_801A3AA0() {
    for (int i = 0; i < m_count; ++i) {
        Unk801A35ACItem *item = fn_801A366C(i);
        if (item->m_clump != 0) {
            fn_80038FC0(item->m_clump);
        }
    }
}

void Unk801A35AC::fn_801A3A44() {
    for (int i = 1; i < m_count; ++i) {
        Unk801A35ACItem *item = fn_801A366C(i);
        if (item->m_clump != 0) {
            fn_80038FC0(item->m_clump);
        }
    }
}

void Unk801A35AC::fn_801A39E0(float value) {
    for (int i = 0; i < m_count; ++i) {
        m_items[i].fn_801A3FB0(value);
    }
}

void Unk801A35AC::fn_801A395C(float red, float green, float blue, float alpha) {
    for (int i = 0; i < m_count; ++i) {
        m_items[i].fn_801A3F90(red, green, blue, alpha);
    }
}

void Unk801A35AC::fn_801A38E8() {
    for (int i = 0; i < m_count; ++i) {
        Unk801A35ACItem *item = fn_801A366C(i);
        if (item->m_frame != 0) {
            if (item->field_0x8 != 0) {
                item->field_0x8->fn_800159AC();
            } else {
                fn_8048722C(item->m_frame);
            }
        }
    }
}

void Unk801A35AC::fn_801A386C(const Fn8040Matrix34 *transform) {
    for (int i = 0; i < m_count; ++i) {
        Unk801A35ACItem *item = fn_801A366C(i);
        if (item->m_frame != 0) {
            if (item->field_0x8 != 0) {
                item->field_0x8->fn_80015954(transform);
            } else {
                fn_8048722C(item->m_frame);
            }
        }
    }
}

void Unk801A35AC::fn_801A3814(RwFrame *parent) {
    if (m_count > 0) {
        Unk801A35ACItem *item = fn_801A366C(0);
        if (parent != 0 && item->m_frame != 0) {
            fn_804872E0(parent, item->m_frame);
        }
    }
}

void Unk801A35AC::fn_801A37C8() {
    if (m_count > 0) {
        Unk801A35ACItem *item = fn_801A366C(0);
        if (item->m_frame != 0 && item->m_frame->object.parent != 0) {
            fn_80487448(item->m_frame);
        }
    }
}

RpClump *Unk801A35AC::fn_801A378C() {
    if (m_count > 0) {
        return fn_801A366C(0)->m_clump;
    }
    return 0;
}

RwFrame *Unk801A35AC::fn_801A3750() {
    if (m_count > 0) {
        return fn_801A366C(0)->m_frame;
    }
    return 0;
}

RwFrame *Unk801A35AC::fn_801A3704(int id) {
    if (m_count > 0) {
        if (m_items[0].m_frame != 0) {
            return fn_80038E84(m_items[0].m_frame, id);
        }
        return 0;
    }
    return 0;
}

void Unk801A35AC::fn_801A367C(const Fn8040Vector3 &position, const Fn8040Vector3 &rotation) {
    if (m_count > 0) {
        Fn8040Matrix34 matrix;
        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &position);
        fn_8040B848(&matrix, rotation.y);
        fn_8040B884(&matrix, rotation.x);
        fn_8040B80C(&matrix, rotation.z);
        m_items[0].fn_801A41D8(matrix);
    }
}

Unk801A35ACItem *Unk801A35AC::fn_801A366C(int index) {
    return &m_items[index];
}

Unk801A35ACParameters *Unk801A35AC::fn_801A3634(int index) {
    if (index < m_count) {
        return &fn_801A366C(index)->m_parameters;
    }
    return 0;
}

void Unk801A35AC::fn_801A35AC(const Unk801A35ACParameters &parameters, int index) {
    if (index == -1) {
        for (int i = 0; i < m_count; ++i) {
            m_items[i].fn_801A3EAC(parameters);
        }
    } else if (index < m_count) {
        m_items[index].fn_801A3EAC(parameters);
    }
}
