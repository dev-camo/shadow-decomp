/* CRI ADX decoder output switch (adx_dcd3.c): only the mono flag survives in this executable, the
 * function that sets it is not linked. The flag is read by ADX_DecodeSte4 (adx_dcd5.c): nonzero mixes
 * both channels of a stereo stream down to mono.
 *
 * Name and place follow the RE4 GameCube decompilation, where the flag is defined by the file of
 * that name. */
#include "CRI/adx_dcd.h"

Sint32 adx_decode_output_mono_flag;
