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

/**
 *  Portions of this file are derived from the following GPL notice:
 *
 *  g729a codec for iPhone and iPod Touch
 *  Copyright (C) 2009 Samuel <samuelv0304@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*---------------------------------------------------------------*
 * LD8A.H                                                        *
 * ~~~~~~                                                        *
 * Function prototypes and constants use for G.729A 8kb/s coder. *
 *                                                               *
 *---------------------------------------------------------------*/

#ifndef __G729_LD8A_H__
#define __G729_LD8A_H__

#include "g729a_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------*
 * Mathematic functions.         *
 *-------------------------------*/

G729_Word32 g729_Inv_sqrt(   /* (o) Q30 : output value   (range: 0<=val<1)           */
  G729_Word32 L_x       /* (i) Q0  : input value    (range: 0<=val<=7fffffff)   */
);

void g729_Log2(
  G729_Word32 L_x,       /* (i) Q0 : input value                                 */
  G729_Word16 *exponent, /* (o) Q0 : Integer part of Log2.   (range: 0<=val<=30) */
  G729_Word16 *fraction  /* (o) Q15: Fractionnal part of Log2. (range: 0<=val<1) */
);

G729_Word32 g729_Pow2(   /* (o) Q0  : result       (range: 0<=val<=0x7fffffff) */
  G729_Word16 exponent,  /* (i) Q0  : Integer part.      (range: 0<=val<=30)   */
  G729_Word16 fraction   /* (i) Q15 : Fractionnal part.  (range: 0.0<=val<1.0) */
);

/*-------------------------------*
 * LPC analysis and filtering.   *
 *-------------------------------*/

void g729_Autocorr(
  G729_Word16 x[],      /* (i)    : Input signal                      */
  G729_Word16 m,        /* (i)    : LPC order                         */
  G729_Word16 r_h[],    /* (o)    : Autocorrelations  (msb)           */
  G729_Word16 r_l[]     /* (o)    : Autocorrelations  (lsb)           */
);

void g729_Lag_window(
  G729_Word16 m,         /* (i)     : LPC order                        */
  G729_Word16 r_h[],     /* (i/o)   : Autocorrelations  (msb)          */
  G729_Word16 r_l[]      /* (i/o)   : Autocorrelations  (lsb)          */
);

void g729_Levinson(
  G729_Word16 Rh[],      /* (i)     : Rh[m+1] Vector of autocorrelations (msb) */
  G729_Word16 Rl[],      /* (i)     : Rl[m+1] Vector of autocorrelations (lsb) */
  G729_Word16 A[],       /* (o) Q12 : A[m]    LPC coefficients  (m = 10)       */
  G729_Word16 rc[]       /* (o) Q15 : rc[M]   Relection coefficients.          */
);

void g729_Az_lsp(
  G729_Word16 a[],        /* (i) Q12 : predictor coefficients              */
  G729_Word16 lsp[],      /* (o) Q15 : line spectral pairs                 */
  G729_Word16 old_lsp[]   /* (i)     : old lsp[] (in case not found 10 roots) */
);

void g729_Lsp_Az(
  G729_Word16 lsp[],    /* (i) Q15 : line spectral frequencies            */
  G729_Word16 a[]       /* (o) Q12 : predictor coefficients (order = 10)  */
);

void g729_Lsf_lsp(
  G729_Word16 lsf[],    /* (i) Q15 : lsf[m] normalized (range: 0.0<=val<=0.5) */
  G729_Word16 lsp[],    /* (o) Q15 : lsp[m] (range: -1<=val<1)                */
  G729_Word16 m         /* (i)     : LPC order                                */
);

void g729_Lsp_lsf(
  G729_Word16 lsp[],    /* (i) Q15 : lsp[m] (range: -1<=val<1)                */
  G729_Word16 lsf[],    /* (o) Q15 : lsf[m] normalized (range: 0.0<=val<=0.5) */
  G729_Word16 m         /* (i)     : LPC order                                */
);

void g729_Int_qlpc(
 G729_Word16 lsp_old[], /* input : LSP vector of past frame              */
 G729_Word16 lsp_new[], /* input : LSP vector of present frame           */
 G729_Word16 Az[]       /* output: interpolated Az() for the 2 subframes */
);

void g729_Weight_Az(
  G729_Word16 a[],      /* (i) Q12 : a[m+1]  LPC coefficients             */
  G729_Word16 gamma,    /* (i) Q15 : Spectral expansion factor.           */
  G729_Word16 m,        /* (i)     : LPC order.                           */
  G729_Word16 ap[]      /* (o) Q12 : Spectral expanded LPC coefficients   */
);

void g729_Residu(
  G729_Word16 a[],    /* (i) Q12 : prediction coefficients                     */
  G729_Word16 x[],    /* (i)     : speech (values x[-m..-1] are needed (m=10)  */
  G729_Word16 y[],    /* (o)     : residual signal                             */
  G729_Word16 lg      /* (i)     : size of filtering                           */
);
    
G729_Flag g729_Syn_filt_Overflow(   /* output: if overflow, return Non-Zero, otherwise return 0 */
    G729_Word16 a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
    G729_Word16 x[],     /* (i)     : input signal                             */
    G729_Word16 y[],     /* (o)     : output signal                            */
    G729_Word16 lg,      /* (i)     : size of filtering                        */
    G729_Word16 mem[]    /* (i/o)   : memory associated with this filtering.   */
);

void g729_Syn_filt(
  G729_Word16 a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
  G729_Word16 x[],     /* (i)     : input signal                             */
  G729_Word16 y[],     /* (o)     : output signal                            */
  G729_Word16 lg,      /* (i)     : size of filtering                        */
  G729_Word16 mem[],   /* (i/o)   : memory associated with this filtering.   */
  G729_Word16 update   /* (i)     : 0=no update, 1=update of memory.         */
);

void g729_Convolve(
  G729_Word16 x[],      /* (i)     : input vector                           */
  G729_Word16 h[],      /* (i) Q12 : impulse response                       */
  G729_Word16 y[],      /* (o)     : output vector                          */
  G729_Word16 L         /* (i)     : vector size                            */
);

/*--------------------------------------------------------------------------*
 *       LTP constant parameters                                            *
 *--------------------------------------------------------------------------*/

#define UP_SAMP         3
#define L_INTER10       10
#define FIR_SIZE_SYN    (UP_SAMP*L_INTER10+1)

/*-----------------------*
 * Pitch functions.      *
 *-----------------------*/

G729_Word16 g729_Pitch_ol_fast(  /* output: open loop pitch lag                        */
   G729_Word16 signal[],         /* input : signal used to compute the open loop pitch */
                                 /*     signal[-pit_max] to signal[-1] should be known */
   G729_Word16   pit_max,        /* input : maximum pitch lag                          */
   G729_Word16   L_frame         /* input : length of frame to compute pitch           */
);

G729_Word16 g729_Pitch_fr3_fast(/* (o)     : pitch period.                          */
  G729_Word16 exc[],       /* (i)     : excitation buffer                      */
  G729_Word16 xn[],        /* (i)     : target vector                          */
  G729_Word16 h[],         /* (i) Q12 : impulse response of filters.           */
  G729_Word16 L_subfr,     /* (i)     : Length of subframe                     */
  G729_Word16 t0_min,      /* (i)     : minimum value in the searched range.   */
  G729_Word16 t0_max,      /* (i)     : maximum value in the searched range.   */
  G729_Word16 i_subfr,     /* (i)     : indicator for first subframe.          */
  G729_Word16 *pit_frac    /* (o)     : chosen fraction.                       */
);

G729_Word16 g729_G_pitch(      /* (o) Q14 : Gain of pitch lag saturated to 1.2       */
  G729_Word16 xn[],       /* (i)     : Pitch target.                            */
  G729_Word16 y1[],       /* (i)     : Filtered adaptive codebook.              */
  G729_Word16 g_coeff[],  /* (i)     : Correlations need for gain quantization. */
  G729_Word16 L_subfr     /* (i)     : Length of subframe.                      */
);

G729_Word16 g729_Enc_lag3(     /* output: Return index of encoding */
  G729_Word16 T0,         /* input : Pitch delay              */
  G729_Word16 T0_frac,    /* input : Fractional pitch delay   */
  G729_Word16 *T0_min,    /* in/out: Minimum search delay     */
  G729_Word16 *T0_max,    /* in/out: Maximum search delay     */
  G729_Word16 pit_min,    /* input : Minimum pitch delay      */
  G729_Word16 pit_max,    /* input : Maximum pitch delay      */
  G729_Word16 pit_flag    /* input : G729_Flag for 1st subframe    */
);

void g729_Dec_lag3(        /* output: return integer pitch lag       */
  G729_Word16 index,       /* input : received pitch index           */
  G729_Word16 pit_min,     /* input : minimum pitch lag              */
  G729_Word16 pit_max,     /* input : maximum pitch lag              */
  G729_Word16 i_subfr,     /* input : subframe flag                  */
  G729_Word16 *T0,         /* output: integer part of pitch lag      */
  G729_Word16 *T0_frac     /* output: fractional part of pitch lag   */
);

G729_Word16 g729_Interpol_3(      /* (o)  : interpolated value  */
  G729_Word16 *x,            /* (i)  : input vector        */
  G729_Word16 frac           /* (i)  : fraction            */
);

void g729_Pred_lt_3(
  G729_Word16   exc[],       /* in/out: excitation buffer */
  G729_Word16   T0,          /* input : integer pitch lag */
  G729_Word16   frac,        /* input : fraction of lag   */
  G729_Word16   L_subfr      /* input : subframe size     */
);

G729_Word16 g729_Parity_Pitch(    /* output: parity bit (XOR of 6 MSB bits)    */
   G729_Word16 pitch_index   /* input : index for which parity to compute */
);

G729_Word16  g729_Check_Parity_Pitch( /* output: 0 = no error, 1= error */
  G729_Word16 pitch_index,       /* input : index of parameter     */
  G729_Word16 parity             /* input : parity bit             */
);

void g729_Cor_h_X(
     G729_Word16 h[],        /* (i) Q12 :Impulse response of filters      */
     G729_Word16 X[],        /* (i)     :Target vector                    */
     G729_Word16 D[]         /* (o)     :Correlations between h[] and D[] */
                        /*          Normalized to 13 bits            */
);

/*-----------------------*
 * Innovative codebook.  *
 *-----------------------*/

#define DIM_RR  616 /* size of correlation matrix                            */
#define NB_POS  8   /* Number of positions for each pulse                    */
#define STEP    5   /* Step betweem position of the same pulse.              */
#define MSIZE   64  /* Size of vectors for cross-correlation between 2 pulses*/

/* The following constants are Q15 fractions.
   These fractions is used to keep maximum precision on "alp" sum */

#define _1_2    (G729_Word16)(16384)
#define _1_4    (G729_Word16)( 8192)
#define _1_8    (G729_Word16)( 4096)
#define _1_16   (G729_Word16)( 2048)

G729_Word16  g729_ACELP_Code_A(    /* (o)     :index of pulses positions    */
  G729_Word16 x[],            /* (i)     :Target vector                */
  G729_Word16 h[],            /* (i) Q12 :Inpulse response of filters  */
  G729_Word16 T0,             /* (i)     :Pitch lag                    */
  G729_Word16 pitch_sharp,    /* (i) Q14 :Last quantized pitch gain    */
  G729_Word16 code[],         /* (o) Q13 :Innovative codebook          */
  G729_Word16 y[],            /* (o) Q12 :Filtered innovative codebook */
  G729_Word16 *sign           /* (o)     :Signs of 4 pulses            */
);

void g729_Decod_ACELP(
  G729_Word16 sign,      /* (i)     : signs of 4 pulses.                       */
  G729_Word16 index,     /* (i)     : Positions of the 4 pulses.               */
  G729_Word16 cod[]      /* (o) Q13 : algebraic (fixed) codebook excitation    */
);

/*-------------------------------*
 * LSP VQ functions.             *
 *-------------------------------*/

void g729_Lsf_lsp2(
  G729_Word16 lsf[],    /* (i) Q13 : lsf[m] (range: 0.0<=val<PI) */
  G729_Word16 lsp[],    /* (o) Q15 : lsp[m] (range: -1<=val<1)   */
  G729_Word16 m         /* (i)     : LPC order                   */
);

void g729_Lsp_lsf2(
  G729_Word16 lsp[],    /* (i) Q15 : lsp[m] (range: -1<=val<1)   */
  G729_Word16 lsf[],    /* (o) Q13 : lsf[m] (range: 0.0<=val<PI) */
  G729_Word16 m         /* (i)     : LPC order                   */
);

void g729_Lsp_expand_1(
  G729_Word16 buf[],          /* Q13 */
  G729_Word16 gap             /* Q13 */
);

void g729_Lsp_expand_2(
  G729_Word16 buf[],         /* Q13 */
  G729_Word16 gap            /* Q13 */
);

void g729_Lsp_expand_1_2(
  G729_Word16 buf[],         /* Q13 */
  G729_Word16 gap            /* Q13 */
);

void g729_Lsp_get_quant(
  G729_Word16 lspcb1[][M],      /* Q13 */
  G729_Word16 lspcb2[][M],      /* Q13 */
  G729_Word16 code0,
  G729_Word16 code1,
  G729_Word16 code2,
  G729_Word16 fg[][M],            /* Q15 */
  G729_Word16 freq_prev[][M],     /* Q13 */
  G729_Word16 lspq[],             /* Q13 */
  G729_Word16 fg_sum[]            /* Q15 */
);

void g729_Lsp_stability(
  G729_Word16 buf[]     /* Q13 */
);

void g729_Lsp_prev_compose(
  G729_Word16 lsp_ele[],             /* Q13 */
  G729_Word16 lsp[],                 /* Q13 */
  G729_Word16 fg[][M],               /* Q15 */
  G729_Word16 freq_prev[][M],        /* Q13 */
  G729_Word16 fg_sum[]               /* Q15 */
);

void g729_Lsp_prev_extract(
  G729_Word16 lsp[M],                 /* Q13 */
  G729_Word16 lsp_ele[M],             /* Q13 */
  G729_Word16 fg[MA_NP][M],           /* Q15 */
  G729_Word16 freq_prev[MA_NP][M],    /* Q13 */
  G729_Word16 fg_sum_inv[M]           /* Q12 */
);

void g729_Lsp_prev_update(
  G729_Word16 lsp_ele[M],             /* Q13 */
  G729_Word16 freq_prev[MA_NP][M]     /* Q13 */
);

/*-------------------------------*
 * gain VQ constants.            *
 *-------------------------------*/

#define NCODE1_B  3                /* number of Codebook-bit */
#define NCODE2_B  4                /* number of Codebook-bit */
#define NCODE1    (1<<NCODE1_B)    /* Codebook 1 size */
#define NCODE2    (1<<NCODE2_B)    /* Codebook 2 size */
#define NCAN1     4                /* Pre-selecting order for #1 */
#define NCAN2     8                /* Pre-selecting order for #2 */
#define INV_COEF  -17103           /* Q19 */

/*--------------------------------------------------------------------------*
 * gain VQ functions.                                                       *
 *--------------------------------------------------------------------------*/

void g729_Gain_predict(
  G729_Word16 past_qua_en[],/* (i) Q10 :Past quantized energies                  */
  G729_Word16 code[],    /* (i) Q13 : Innovative vector.                         */
  G729_Word16 L_subfr,   /* (i)     : Subframe length.                           */
  G729_Word16 *gcode0,   /* (o) Qxx : Predicted codebook gain                    */
  G729_Word16 *exp_gcode0 /* (o)    : Q-Format(gcode0)                           */
);

void g729_Gain_update(
  G729_Word16 past_qua_en[],/* (i) Q10 :Past quantized energies                  */
  G729_Word32 L_gbk12    /* (i) Q13 : g729_gbk1[indice1][1]+g729_gbk2[indice2][1]          */
);

void g729_Gain_update_erasure(
  G729_Word16 past_qua_en[]/* (i) Q10 :Past quantized energies                   */
);

void g729_Corr_xy2(
      G729_Word16 xn[],           /* (i) Q0  :Target vector.                  */
      G729_Word16 y1[],           /* (i) Q0  :Adaptive codebook.              */
      G729_Word16 y2[],           /* (i) Q12 :Filtered innovative vector.     */
      G729_Word16 g_coeff[],      /* (o) Q[exp]:Correlations between xn,y1,y2 */
      G729_Word16 exp_g_coeff[]   /* (o)       :Q-format of g_coeff[]         */
);

/*-----------------------*
 * Bitstream function    *
 *-----------------------*/

void  g729_prm2bits_ld8k(G729_Word16 prm[], G729_Word16 bits[]);
void  g729_bits2prm_ld8k(G729_Word16 bits[], G729_Word16 prm[]);
#define BIT_0     (short)0x007f /* definition of zero-bit in bit-stream      */
#define BIT_1     (short)0x0081 /* definition of one-bit in bit-stream       */

void  g729_prm2bits_ld8k_compressed(G729_Word16 prm[], G729_UWord8 bits[]);
void  g729_bits2prm_ld8k_compressed(G729_UWord8 bits[], G729_Word16 prm[]);

#define SYNC_WORD (short)0x6b21 /* definition of frame erasure flag          */
#define SIZE_WORD (short)80     /* number of speech bits                     */


/*-----------------------------------*
 * Post-filter functions.            *
 *-----------------------------------*/

#define L_H 22     /* size of truncated impulse response of A(z/g1)/A(z/g2) */

#define GAMMAP      16384   /* 0.5               (Q15) */
#define INV_GAMMAP  21845   /* 1/(1+GAMMAP)      (Q15) */
#define GAMMAP_2    10923   /* GAMMAP/(1+GAMMAP) (Q15) */

#define  GAMMA2_PST 18022 /* Formant postfilt factor (numerator)   0.55 Q15 */
#define  GAMMA1_PST 22938 /* Formant postfilt factor (denominator) 0.70 Q15 */

#define  MU       26214   /* Factor for tilt compensation filter   0.8  Q15 */
#define  AGC_FAC  29491   /* Factor for automatic gain control     0.9  Q15 */
#define  AGC_FAC1 (G729_Word16)(32767 - AGC_FAC)    /* 1-AGC_FAC in Q15          */

/*--------------------------------------------------------------------------*
 * Constants and prototypes for taming procedure.                           *
 *--------------------------------------------------------------------------*/

#define GPCLIP      15564      /* Maximum pitch gain if taming is needed Q14*/
#define GPCLIP2     481        /* Maximum pitch gain if taming is needed Q9 */
#define GP0999      16383      /* Maximum pitch gain if taming is needed    */
#define L_THRESH_ERR 983040000L /* Error threshold taming 16384. * 60000.   */

/*--------------------------------------------------------------------------*
 * Prototypes for auxiliary functions.                                      *
 *--------------------------------------------------------------------------*/

void g729_Copy(
  G729_Word16 x[],      /* (i)   : input vector   */
  G729_Word16 y[],      /* (o)   : output vector  */
  G729_Word16 L         /* (i)   : vector length  */
);

void g729_Set_zero(
  G729_Word16 x[],       /* (o)    : vector to clear     */
  G729_Word16 L          /* (i)    : length of vector    */
);

G729_Word16 g729_Random(void);
    
#ifdef __cplusplus
}
#endif

#endif
