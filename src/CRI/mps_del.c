/* CRI Sofdec MPS start-code classifier (mps_del.c): tells a 00 00 01 xx MPEG system start code
 * apart for the demuxer (pack 0xBA, system header 0xBB, program end 0xB9, packets >= 0xBC).
 * Body from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/cri_xpt.h"
#include "CRI/mps.h"

/* Start-code class of the 4 bytes at p: MPS_DELIM_END program end (0xB9), MPS_DELIM_PACK pack
 * (0xBA), MPS_DELIM_SYSHD system header (0xBB), MPS_DELIM_PKET packet (0xBC..0xFF), 0 not a start
 * code. */
Uint32 MPS_CheckDelim(Uint8 *p) {
    if (p[0] == 0 && p[1] == 0 && p[2] == 1) {
        switch (p[3]) {
        case 0xB9:
            return MPS_DELIM_END;
        case 0xBA:
            return MPS_DELIM_PACK;
        case 0xBB:
            return MPS_DELIM_SYSHD;
        default:
            if (p[3] >= 0xBC) {
                return MPS_DELIM_PKET;
            }
        }
    }
    return 0;
}
