/* Command-stream traversal adjacent to Effect::System RTTI data. The original file and
 * record type names are unknown; lengths are encoded record sizes in bytes. */
#include "Game/fn_804220CC.h"
#include "Game/EffectCommandExec.h"

using namespace Effect::System;

unsigned int fn_804220CC(const UnkCommandData *command) {
    unsigned int count = 0;

    while (command != 0) {
        unsigned int size = 0;

        switch (command->type) {
        case COMMAND_TYPE_00:
        case COMMAND_GLARE_SWITCH:
            break;
        case COMMAND_PTN_ANIM:
            size = sizeof(UnkPtnAnimData);
            break;
        case COMMAND_3D_POSITION:
            size = sizeof(UnkVector3Data);
            break;
        case COMMAND_3D_ROTATION:
            size = sizeof(UnkVector3Data);
            break;
        case COMMAND_2D_ROTATION:
            size = sizeof(UnkSingleData);
            break;
        case COMMAND_3D_SCALE:
            size = sizeof(UnkVector3Data);
            break;
        case COMMAND_2D_SCALE:
            size = sizeof(UnkVector2Data);
            break;
        case COMMAND_3D_VELOCITY:
            size = sizeof(UnkVector3Data);
            break;
        case COMMAND_GRAVITY:
            size = sizeof(UnkSingleData);
            break;
        case COMMAND_FRICTION:
            size = sizeof(UnkSingleData);
            break;
        case COMMAND_MAT_COLOR:
            size = COMMAND_MAT_COLOR_STRIDE;
            break;
        case COMMAND_MAT_ALPHA:
            size = COMMAND_MAT_ALPHA_STRIDE;
            break;
        case COMMAND_WAIT:
            size = sizeof(UnkWaitData);
            break;
        case COMMAND_LOOP_START:
            size = sizeof(UnkLoopStartData);
            break;
        case COMMAND_LOOP_END:
            size = sizeof(UnkCommandData);
            break;
        case COMMAND_CALLBACK:
            size = COMMAND_CALLBACK_STRIDE;
            break;
        case COMMAND_LAUNCH:
            size = sizeof(UnkLaunchData);
            break;
        case COMMAND_TIMED_LAUNCH:
            size = sizeof(UnkTimedLaunchData);
            break;
        case COMMAND_REQUEST_STOP:
            size = sizeof(UnkCommandData);
            break;
        case COMMAND_GEN_QUANTITY:
            size = sizeof(UnkSingleData);
            break;
        case COMMAND_LOCUS_POINT:
            size = COMMAND_LOCUS_POINT_STRIDE;
            break;
        case COMMAND_TOP_COLOR:
            size = COMMAND_MAT_COLOR_STRIDE;
            break;
        case COMMAND_TOP_ALPHA:
            size = COMMAND_MAT_ALPHA_STRIDE;
            break;
        case COMMAND_LAST_COLOR:
            size = COMMAND_MAT_COLOR_STRIDE;
            break;
        case COMMAND_LAST_ALPHA:
            size = COMMAND_MAT_ALPHA_STRIDE;
            break;
        case COMMAND_LENGTH:
            size = sizeof(UnkSingleData);
            break;
        case COMMAND_WIDTH:
            size = sizeof(UnkSingleData);
            break;
        case COMMAND_2D_AMPLITUDE:
            size = sizeof(UnkVector2Data);
            break;
        case COMMAND_2D_CYCLE:
            size = sizeof(UnkVector2Data);
            break;
        case COMMAND_2D_WAVE_LENGTH:
            size = sizeof(UnkVector2Data);
            break;
        case COMMAND_AMPLITUDE:
            size = sizeof(UnkSingleData);
            break;
        case COMMAND_CYCLE:
            size = sizeof(UnkSingleData);
            break;
        case COMMAND_WAVE_LENGTH:
            size = sizeof(UnkSingleData);
            break;
        }

        if (command->hasNext != 0 && size != 0) {
            command = reinterpret_cast<const UnkCommandData *>(
                reinterpret_cast<const unsigned char *>(command) + size);
        } else {
            command = 0;
        }
        count++;
    }
    return count;
}
