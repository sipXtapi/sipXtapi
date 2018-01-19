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

#ifndef __G729_DECODER_H__
#define __G729_DECODER_H__

#include "g729a_typedef.h"
#include "g729a_defines.h"

typedef struct _g729a_lspdec_state
{
    /*--------------------------------------------------------------------------*
     * lspdec.c
     *--------------------------------------------------------------------------*/
    
    G729_Word16 freq_prev[MA_NP][M];      /* Q13 */
    G729_Word16 prev_ma;                  /* previous MA prediction coef.*/
    G729_Word16 prev_lsp[M];              /* previous LSP vector         */
} g729a_lspdec_state;

typedef struct _g729a_post_filter_state
{
    /*--------------------------------------------------------------------------*
     * postfilt.c
     *--------------------------------------------------------------------------*/
    
    /* inverse filtered synthesis (with A(z/GAMMA2_PST))   */
    G729_Word16 res2_buf[PIT_MAX + L_SUBFR];
    G729_Word16 *res2;
    G729_Word16 scal_res2_buf[PIT_MAX + L_SUBFR];
    G729_Word16 *scal_res2;
    
    /* memory of filter 1/A(z/GAMMA1_PST) */
    G729_Word16 mem_syn_pst[M];
    
    G729_Word16 mem_pre;
    G729_Word16 past_gain;
} g729a_post_filter_state;

typedef struct _g729a_post_process_state
{
    /*--------------------------------------------------------------------------*
     * post_pro.c
     *--------------------------------------------------------------------------*/
    
    G729_Word16 y2_hi;
    G729_Word16 y2_lo;
    G729_Word16 y1_hi;
    G729_Word16 y1_lo;
    G729_Word16 x0;
    G729_Word16 x1;
} g729a_post_process_state;

typedef struct _g729a_decoder_state
{
    G729_Word32 error;  /* TODO */
    
    /*--------------------------------------------------------------------------*
     * dec_ld8a.c
     *--------------------------------------------------------------------------*/
    
    /* Excitation vector */
    G729_Word16 old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
    G729_Word16 *exc;
    
    /* Lsp (Line spectral pairs) */
    G729_Word16 lsp_old[M];
    
    /* Filter's memory */
    G729_Word16 mem_syn[M];
    
    G729_Word16 sharp;           /* pitch sharpening of previous frame */
    G729_Word16 old_T0;          /* integer delay of previous frame    */
    G729_Word16 gain_code;       /* Code gain                          */
    G729_Word16 gain_pitch;      /* Pitch gain                         */
    
    G729_Word16 synth_buf[L_FRAME + M];
    G729_Word16 *synth;
    
    /*--------------------------------------------------------------------------*
     * dec_gain.c
     *--------------------------------------------------------------------------*/
    
    /* Gain predictor, Past quantized energies = -14.0 in Q10 */
    G729_Word16 past_qua_en[4];
    
    /*--------------------------------------------------------------------------*
     *--------------------------------------------------------------------------*/
    
    g729a_lspdec_state        lspdec_state;
    g729a_post_filter_state   post_filter_state;
    g729a_post_process_state  post_process_state;
} g729a_decoder_state;

#ifdef __cplusplus
extern "C" {
#endif
    
/*--------------------------------------------------------------------------*
 * cod_ld8a.c                                                               *
 * Main decoder functions                                                   *
 *--------------------------------------------------------------------------*/

void g729_Init_Decod_ld8a(g729a_decoder_state * state);

void g729_Decod_ld8a(
    g729a_decoder_state * state,
    G729_Word16  parm[],      /* (i)   : vector of synthesis parameters
                                         parm[0] = bad frame indicator (bfi)  */
    G729_Word16  synth[],     /* (o)   : synthesis speech                     */
    G729_Word16  A_t[],       /* (o)   : decoded LP filter in 2 subframes     */
    G729_Word16  *T2,         /* (o)   : decoded pitch lag in 2 subframes     */
    G729_Word16 bad_lsf       /* (i)   : bad LSF indicator   */
);
    
/*-------------------------------*
 * Post filter                   *
 *-------------------------------*/
    
void g729_Init_Post_Filter(g729a_post_filter_state * state);

void g729_Post_Filter(
    g729a_post_filter_state * state,
    G729_Word16 *syn,        /* in/out: synthesis speech (postfiltered is output)    */
    G729_Word16 *Az_4,       /* input : interpolated LPC parameters in all subframes */
    G729_Word16 *T           /* input : decoded pitch lags in all subframes          */
);
    
/*-------------------------------*
 * Post-process                  *
 *-------------------------------*/

void g729_Init_Post_Process(g729a_post_process_state * state);

void g729_Post_Process(
    g729a_post_process_state * state,
    G729_Word16 signal_in[],    /* Input signal        */
    G729_Word16 signal_out[],   /* Output signal       */
    G729_Word16 lg              /* Length of signal    */
);
    
/*-------------------------------*
 * lspdec                        *
 *-------------------------------*/
    
void g729_Lsp_decw_reset(g729a_lspdec_state * state);
    
void g729_D_lsp(
    g729a_lspdec_state * state,
    G729_Word16 prm[],          /* (i)     : indexes of the selected LSP */
    G729_Word16 lsp_q[],        /* (o) Q15 : Quantized LSP parameters    */
    G729_Word16 erase           /* (i)     : frame erase information     */
);
    
/*--------------------------------------------------------------------------*
 * gain VQ functions.                                                       *
 *--------------------------------------------------------------------------*/

void g729_Dec_gain(
    g729a_decoder_state * state,
    G729_Word16 index,     /* (i)     : Index of quantization.                     */
    G729_Word16 code[],    /* (i) Q13 : Innovative vector.                         */
    G729_Word16 L_subfr,   /* (i)     : Subframe length.                           */
    G729_Word16 bfi,       /* (i)     : Bad frame indicator                        */
    G729_Word16 *gain_pit, /* (o) Q14 : Pitch gain.                                */
    G729_Word16 *gain_cod  /* (o) Q1  : Code gain.                                 */
);
    
#ifdef __cplusplus
}
#endif

#endif  /* __G729_DECODER_H__ */
/* end of file */
