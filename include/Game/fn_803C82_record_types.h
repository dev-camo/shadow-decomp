#ifndef GAME_FN_803C82_RECORD_TYPES_H
#define GAME_FN_803C82_RECORD_TYPES_H

/* View of a CRI LSC scheduler object (LSC_OBJ, 0x238 bytes per LSC_Destroy).
 * Chosen accessed-field view; original field names and types unknown. */
typedef struct Fn803C82RecordView {
    /* 0x00 */ unsigned char byte_0;
    /* 0x01 */ signed char byte_1;
    /* 0x02 */ unsigned char byte_2;
    /* 0x03 */ unsigned char unknown_3[0x11];
    /* 0x14 */ unsigned int word_14;
    /* 0x18 */ unsigned int word_18;
    /* 0x1C */ unsigned int word_1C;
    /* 0x20 */ unsigned int word_20;
    /* 0x24 */ unsigned int word_24;
    /* 0x28 */ void *pointer_28;
    /* 0x2C */ unsigned int word_2C;
    /* 0x30 */ unsigned char unknown_30[4];
    /* 0x34 */ unsigned int word_34;
} Fn803C82RecordView;

#ifdef __cplusplus
extern "C" {
#endif

/* Chosen consumed-pointer/side-effect projections; original full interfaces unknown. */
void LSC_SetFlowLimit(Fn803C82RecordView *object, int value);
void LSC_Stop(Fn803C82RecordView *object);
void fn_803C8464(Fn803C82RecordView *object);
void LSC_Destroy(Fn803C82RecordView *object);

#ifdef __cplusplus
}
#endif

#endif
