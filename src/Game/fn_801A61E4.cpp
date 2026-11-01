/* EnemyParamCommon and its parameter tables; original filename unknown. */
#include "Game/EnemyParamCommon.h"
#include "Game/Enemy.h"
#include "Game/UnkResource_80012700.h"
#include "Game/fn_801A04C0.h"

const int lbl_804CC260[15] = {
    0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1,
};

const short lbl_804CC29C[15] = {
    40, 1000, 3201, 10, 10, 10, 3151, 4101, 3007, 2000, 5000, 10, 10, 10, 10,
};

const float lbl_804CC2BC[15][6] = {
    {7.0f, 22.0f, 0.0f, 10.0f, 5.0f, 4.0f},    {15.0f, 22.0f, -8.0f, 0.0f, 9.0f, 8.0f},
    {20.0f, 40.0f, 0.0f, 0.0f, 20.0f, 18.0f},  {18.0f, 30.0f, 0.0f, 0.0f, 15.0f, 12.0f},
    {15.0f, 50.0f, 0.0f, 20.0f, 13.0f, 12.0f}, {10.0f, 30.0f, 0.0f, 20.0f, 7.0f, 6.0f},
    {15.0f, 20.0f, -4.0f, 0.0f, 5.0f, 7.0f},   {10.0f, 15.0f, -4.0f, 0.0f, 3.0f, 4.0f},
    {10.0f, 40.0f, 0.0f, 0.0f, 10.0f, 10.0f},  {8.0f, 10.0f, 0.0f, 0.0f, 8.0f, 8.0f},
    {8.0f, 10.0f, 0.0f, 0.0f, 12.0f, 12.0f},   {8.0f, 40.0f, 0.0f, 15.0f, 8.0f, 7.0f},
    {12.0f, 24.0f, 0.0f, 12.0f, 8.0f, 8.0f},   {12.0f, 24.0f, 0.0f, 12.0f, 8.0f, 8.0f},
    {7.0f, 20.0f, 0.0f, 5.0f, 3.0f, 3.0f},
};

const float lbl_804CC424[15][7] = {
    {10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 6.0f, 120.0f},
    {50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 3.0f, 0.0f},
    {10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1.0f, 120.0f},
    {50.0f, 50.0f, 50.0f, 0.0f, 0.0f, 3.0f, 120.0f},
    {10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 3.0f, 120.0f},
    {10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 6.0f, 120.0f},
    {50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 6.0f, 0.0f},
    {50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 6.0f, 0.0f},
    {10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 6.0f, 120.0f},
    {10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 6.0f, 120.0f},
    {10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 8.0f, 0.0f},
    {50.0f, 50.0f, 50.0f, 0.0f, 0.0f, 6.0f, 120.0f},
    {50.0f, 50.0f, 50.0f, 0.0f, 0.0f, 3.0f, 120.0f},
    {50.0f, 50.0f, 50.0f, 0.0f, 0.0f, 3.0f, 120.0f},
    {50.0f, 50.0f, 50.0f, 0.0f, 0.0f, 3.0f, 120.0f},
};

const float lbl_804CC5C8[15] = {
    2.0f, 2.0f, 18.0f, 8.0f, 24.0f, 4.0f, 6.0f, 2.0f, 12.0f, 1.0f, 1.0f, 16.0f, 4.0f, 10.0f, 8.0f,
};

const float lbl_804CC604[15][3] = {
    {50.0f, 1.3f, 1.0f}, {2.0f, 2.0f, 1.0f},  {0.0f, 0.0f, 0.0f},  {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},  {50.0f, 1.3f, 1.0f}, {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.0f},  {50.0f, 1.2f, 1.0f}, {0.0f, 0.0f, 0.0f},  {50.0f, 1.3f, 1.0f},
    {40.0f, 1.2f, 1.0f}, {0.0f, 0.0f, 0.0f},  {50.0f, 1.3f, 1.0f},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC6B8[] = {
    {40, 0xc80},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC6C8[] = {
    {10, 0xc80},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC6D8[] = {
    {10, 0xc80},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC6E8[] = {
    {1000, 0xc80},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC6F8[] = {
    {4101, 0xc80},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC708[] = {
    {3151, 0xc80},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC718[] = {
    {2000, 0xc80},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC728[] = {
    {30, 0x1400},
    {10, 0x800},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC740[] = {
    {9000, 0x1400},
    {3201, 0x800},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC758[] = {
    {3007, 0xc80},
    {3007, 0x1000},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC770[] = {
    {30, 0x1400},
    {10, 0x800},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC788[] = {
    {5000, 0xc80},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC798[] = {
    {30, 0x1400},
    {10, 0x800},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC7B0[] = {
    {10, 0x480},
    {-1, 0x0},
};

const UnkEnemyParaEntry_801A6230 lbl_804CC7C0[] = {
    {10, 0xc80},
    {-1, 0x0},
};

const unsigned short lbl_804CC7D0[15] = {
    48, 3, 0, 12, 0, 22, 6, 5, 8, 0, 0, 5, 22, 8, 1,
};

const UnkEnemyParaEntry_801A6230 *lbl_8053AA40[15] = {
    lbl_804CC6B8, lbl_804CC6E8, lbl_804CC740, lbl_804CC728, lbl_804CC770,
    lbl_804CC6C8, lbl_804CC708, lbl_804CC6F8, lbl_804CC758, lbl_804CC718,
    lbl_804CC788, lbl_804CC798, lbl_804CC6D8, lbl_804CC7B0, lbl_804CC7C0,
};

const UnkEnemyParaEntry_800C9178 lbl_805F5800 = {1, -1, -1, 0, 0};
const UnkEnemyParaGroup_800C961C lbl_805F5808 = {&lbl_805F5800, 1, 0};
const UnkEnemyParaData_8017B8E4 lbl_805F5810 = {&lbl_805F5808, 1};

EnemyParamCommon::EnemyParamCommon() : field_0x8(&lbl_805F5810) {}

EnemyParamCommon::~EnemyParamCommon() {}

int EnemyParamCommon::fn_8019B464() const {
    return lbl_804CC260[m_enemy->field_0x20];
}

const char *EnemyParamCommon::fn_8019B46C() const {
    int kind = fn_801A642C(m_enemy);
    return lbl_804CBF2C[kind][0];
}

const char *EnemyParamCommon::fn_8019B474() const {
    int kind = fn_801A642C(m_enemy);
    return lbl_804CBF2C[kind][1];
}

const char *EnemyParamCommon::fn_8019B47C() const {
    int kind = fn_801A642C(m_enemy);
    return lbl_804CBF2C[kind][2];
}

int fn_801A642C(const Enemy *enemy) {
    return enemy->field_0x20;
}

const char *EnemyParamCommon::fn_8019B484() const {
    int kind = fn_801A642C(m_enemy);
    return lbl_804CBF2C[kind][3];
}

int EnemyParamCommon::fn_8019B4B4() const {
    return fn_801A04C0(m_enemy->field_0x20);
}

int EnemyParamCommon::fn_80192B58() const {
    return lbl_804CC29C[m_enemy->field_0x20];
}

const float *EnemyParamCommon::fn_8019B48C() const {
    return lbl_804CC2BC[m_enemy->field_0x20];
}

float EnemyParamCommon::fn_80181F04() const {
    return lbl_804CC424[m_enemy->field_0x20][0];
}

float EnemyParamCommon::fn_8019B494() const {
    return lbl_804CC424[m_enemy->field_0x20][1];
}

float EnemyParamCommon::fn_8019B49C() const {
    return lbl_804CC424[m_enemy->field_0x20][2];
}

float EnemyParamCommon::fn_8019B4A4() const {
    return lbl_804CC424[m_enemy->field_0x20][3];
}

float EnemyParamCommon::fn_8019B4AC() const {
    return lbl_804CC424[m_enemy->field_0x20][0];
}

float EnemyParamCommon::fn_8019AD04() const {
    return lbl_804CC424[m_enemy->field_0x20][5];
}

float EnemyParamCommon::fn_8019B4E0() const {
    return lbl_804CC424[m_enemy->field_0x20][6];
}

float EnemyParamCommon::fn_8019B4E8() const {
    return lbl_804CC5C8[m_enemy->field_0x20];
}

const float *EnemyParamCommon::fn_8019B4F0() const {
    return lbl_804CC604[m_enemy->field_0x20];
}

const UnkEnemyParaEntry_801A6230 *EnemyParamCommon::fn_8019B4F8() const {
    return lbl_8053AA40[m_enemy->field_0x20];
}

unsigned short fn_801A61E4(int kind) {
    if (kind == 14) {
        return fn_80012608(fn_800122D4(), 21, 7);
    }
    return lbl_804CC7D0[kind];
}
