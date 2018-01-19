/**
 *  Copyright (c) 2015, Russell
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  Portions of this file are derived from the following ITU notice:
 *
 *  ITU-T G.729 Software Package Release 2 (November 2006)
 *
 *  ITU-T G.729A Speech Coder    ANSI-C Source Code
 *  Version 1.1    Last modified: September 1996
 *
 *  Copyright (c) 1996,
 *  AT&T, France Telecom, NTT, Universite de Sherbrooke
 *  All rights reserved.
 */

#ifndef __G729_TAB_LD8A_H__
#define __G729_TAB_LD8A_H__

#ifdef __cplusplus
extern "C" {
#endif
    
extern G729_Word16 g729_hamwindow[L_WINDOW];
extern G729_Word16 g729_lag_h[M];
extern G729_Word16 g729_lag_l[M];
extern G729_Word16 g729_table[65];
extern G729_Word16 g729_slope[64];
extern G729_Word16 g729_table2[64];
extern G729_Word16 g729_slope_cos[64];
extern G729_Word16 g729_slope_acos[64];
extern G729_Word16 g729_lspcb1[NC0][M];
extern G729_Word16 g729_lspcb2[NC1][M];
extern G729_Word16 g729_fg[2][MA_NP][M];
extern G729_Word16 g729_fg_sum[2][M];
extern G729_Word16 g729_fg_sum_inv[2][M];
extern G729_Word16 g729_grid[GRID_POINTS+1];
extern G729_Word16 g729_inter_3l[FIR_SIZE_SYN];
extern G729_Word16 g729_pred[4];
extern G729_Word16 g729_gbk1[NCODE1][2];
extern G729_Word16 g729_gbk2[NCODE2][2];
extern G729_Word16 g729_map1[NCODE1];
extern G729_Word16 g729_map2[NCODE2];
extern G729_Word16 g729_coef[2][2];
extern G729_Word32 g729_L_coef[2][2];
extern G729_Word16 g729_thr1[NCODE1-NCAN1];
extern G729_Word16 g729_thr2[NCODE2-NCAN2];
extern G729_Word16 g729_imap1[NCODE1];
extern G729_Word16 g729_imap2[NCODE2];
extern G729_Word16 g729_bitsno[PRM_SIZE];
extern G729_Word16 g729_tabpow[33];
extern G729_Word16 g729_tablog[33];
extern G729_Word16 g729_tabsqr[49];
extern G729_Word16 g729_tab_zone[PIT_MAX+L_INTERPOL-1];

#ifdef __cplusplus
}
#endif

#endif  /* __G729_TAB_LD8A_H__ */
/* end of file */
