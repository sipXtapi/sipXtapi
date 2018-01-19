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

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/

#ifndef __G729_BASIC_OP_H__
#define __G729_BASIC_OP_H__

#define G729A_MAX_32 (G729_Word32)0x7fffffffL
#define G729A_MIN_32 (G729_Word32)0x80000000L

#define G729A_MAX_16 (G729_Word16)0x7fff
#define G729A_MIN_16 (G729_Word16)0x8000

#if defined(USE_GLOBAL_OVERFLOW_FLAG) && (USE_GLOBAL_OVERFLOW_FLAG == 1)
extern G729_Flag G729A_Overflow_Flag;
#endif

/*___________________________________________________________________________
 |                                                                           |
 |   Operators prototypes                                                    |
 |___________________________________________________________________________|
*/
#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------*
 * Never overflow                               *
 *----------------------------------------------*/
    
G729_Word16 g729_abs_s(G729_Word16 var1);              /* Short abs,           1 */
G729_Word16 g729_negate(G729_Word16 var1);             /* Short negate,        1 */
G729_Word16 g729_extract_h(G729_Word32 L_var1);        /* Extract high,        1 */
G729_Word16 g729_extract_l(G729_Word32 L_var1);        /* Extract low,         1 */
G729_Word32 g729_L_abs(G729_Word32 L_var1);            /* Long abs,            3 */
G729_Word32 g729_L_negate(G729_Word32 L_var1);         /* Long negate,         2 */
G729_Word32 g729_L_deposit_h(G729_Word16 var1);        /* 16 bit var1 -> MSB,  2 */
G729_Word32 g729_L_deposit_l(G729_Word16 var1);        /* 16 bit var1 -> LSB   2 */
G729_Word16 g729_norm_s(G729_Word16 var1);             /* Short norm,         15 */
G729_Word16 g729_norm_l(G729_Word32 L_var1);           /* Long norm,          30 */

/*----------------------------------------------*
 * Maybe overflow                               *
 *----------------------------------------------*/

G729_Word16 g729_add(G729_Word16 var1, G729_Word16 var2);     /* Short add,             1 */
G729_Word16 g729_sub(G729_Word16 var1, G729_Word16 var2);     /* Short sub,             1 */
G729_Word16 g729_shl(G729_Word16 var1, G729_Word16 var2);     /* Short shift left,      1 */
G729_Word16 g729_shr(G729_Word16 var1, G729_Word16 var2);     /* Short shift right,     1 */
G729_Word16 g729_mult(G729_Word16 var1, G729_Word16 var2);    /* Short mult,            1 */
G729_Word32 g729_L_add(G729_Word32 L_var1, G729_Word32 L_var2);   /* Long add,          2 */
G729_Word32 g729_L_sub(G729_Word32 L_var1, G729_Word32 L_var2);   /* Long sub,          2 */
G729_Word32 g729_L_shl(G729_Word32 L_var1, G729_Word16 var2);     /* Long shift left,   2 */
G729_Word32 g729_L_shr(G729_Word32 L_var1, G729_Word16 var2);     /* Long shift right,  2 */
G729_Word32 g729_L_mult(G729_Word16 var1, G729_Word16 var2);      /* Long mult,         1 */
G729_Word16 g729_round(G729_Word32 L_var1);                       /* Round,             1 */
G729_Word32 g729_L_mac(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2);   /* Mac,    1 */
G729_Word32 g729_L_msu(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2);   /* Msu,    1 */
G729_Word16 g729_mult_r(G729_Word16 var1, G729_Word16 var2);  /* Mult with round,        2 */
G729_Word16 g729_shr_r(G729_Word16 var1, G729_Word16 var2);   /* Shift right with round, 2 */
G729_Word16 g729_mac_r(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2);/* Mac with rounding, 2*/
G729_Word16 g729_msu_r(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2);/* Msu with rounding, 2*/
G729_Word32 g729_L_shr_r(G729_Word32 L_var1, G729_Word16 var2);  /* Long shift right with round,  3*/
G729_Word16 g729_div_s(G729_Word16 var1, G729_Word16 var2);      /* Short division,      18 */

/*----------------------------------------------*
 * No use                                       *
 *----------------------------------------------*/
    
#if 0
G729_Word32 g729_L_macNs(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2); /* Mac without sat, 1*/
G729_Word32 g729_L_msuNs(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2); /* Msu without sat, 1*/
G729_Word32 g729_L_sat(G729_Word32 L_var1);                         /* Long saturation,  4 */
G729_Word32 g729_L_add_c(G729_Word32 L_var1, G729_Word32 L_var2);   /*Long add with c,   2 */
G729_Word32 g729_L_sub_c(G729_Word32 L_var1, G729_Word32 L_var2);   /*Long sub with c,   2 */
#endif
    
#ifdef __cplusplus
}
#endif

#endif
