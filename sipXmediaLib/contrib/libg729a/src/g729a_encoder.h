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

#ifndef __G729_ENCODER_H__
#define __G729_ENCODER_H__

#include "g729a_typedef.h"
#include "g729a_defines.h"

typedef struct _g729a_pre_process_state
{
    /*--------------------------------------------------------------------------*
     * pre_proc.c                                                               *
     *--------------------------------------------------------------------------*/
    
    G729_Word16 y2_hi;
    G729_Word16 y2_lo;
    G729_Word16 y1_hi;
    G729_Word16 y1_lo;
    G729_Word16 x0;
    G729_Word16 x1;
} g729a_pre_process_state;

typedef struct _g729a_lspenc_state
{
    /*--------------------------------------------------------------------------*
     * qua_lsp.c                                                                *
     *--------------------------------------------------------------------------*/
    
    G729_Word16 freq_prev[MA_NP][M];  /* Q13:previous LSP vector */
} g729a_lspenc_state;

typedef struct _g729a_taming_state
{
    /*--------------------------------------------------------------------------*
     * taming.c                                                                 *
     *--------------------------------------------------------------------------*/
    
    G729_Word32 L_exc_err[4];
} g729a_taming_state;

typedef struct _g729a_encoder_state
{
    G729_Word32 error;  /* TODO */
    
    /*--------------------------------------------------------------------------*
     * cod_ld8a.c                                                               *
     *--------------------------------------------------------------------------*/
    
    /* Speech vector */
    G729_Word16 old_speech[L_TOTAL];
    G729_Word16 *speech;
    G729_Word16 *p_window;
    G729_Word16 *new_speech;                    /* Global variable */
    
    /* Weighted speech vector */
    G729_Word16 old_wsp[L_FRAME+PIT_MAX];
    G729_Word16 *wsp;
    
    /* Excitation vector */
    G729_Word16 old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
    G729_Word16 *exc;
    
    /* Lsp (Line spectral pairs) */
    G729_Word16 lsp_old[M];
    G729_Word16 lsp_old_q[M];
    
    /* Filter's memory */
    G729_Word16 mem_w0[M];
    G729_Word16 mem_w[M];
    G729_Word16 mem_zero[M];
    G729_Word16 sharp;
    
    /*--------------------------------------------------------------------------*
     * qua_gain.c                                                               *
     *--------------------------------------------------------------------------*/
    
    /* Gain predictor, Past quantized energies = -14.0 in Q10 */
    G729_Word16 past_qua_en[4];
    
    /*--------------------------------------------------------------------------*
     *--------------------------------------------------------------------------*/
    
    g729a_pre_process_state  pre_process_state;
    g729a_lspenc_state       lspenc_state;
    g729a_taming_state       taming_state;
} g729a_encoder_state;

#ifdef __cplusplus
extern "C" {
#endif
    
/*--------------------------------------------------------------------------*
 * cod_ld8a.c                                                               *
 * Main coder functions                                                     *
 *--------------------------------------------------------------------------*/
    
void g729_Init_Coder_ld8a(g729a_encoder_state * state);

void g729_Coder_ld8a(
    g729a_encoder_state * state,
    G729_Word16 ana[]                    /* output  : Analysis parameters */
);
    
/*-------------------------------*
 * Pre-process.                  *
 *-------------------------------*/
    
void g729_Init_Pre_Process(g729a_pre_process_state * state);

void g729_Pre_Process(
    g729a_pre_process_state * state,
    G729_Word16 singal_in[],    /* Input signal */
    G729_Word16 signal_out[],   /* Output signal */
    G729_Word16 lg              /* Length of signal    */
);
    
/*-------------------------------*
 * lspenc                        *
 *-------------------------------*/
    
void g729_Lsp_encw_reset(g729a_lspenc_state * state);

void g729_Qua_lsp(
    g729a_lspenc_state * state,
    G729_Word16 lsp[],       /* (i) Q15 : Unquantized LSP            */
    G729_Word16 lsp_q[],     /* (o) Q15 : Quantized LSP              */
    G729_Word16 ana[]        /* (o)     : indexes                    */
);
    
/*-------------------------------*
 * taming                        *
 *-------------------------------*/
void   g729_Init_exc_err(g729a_taming_state * state);
void   g729_update_exc_err(g729a_taming_state * state, G729_Word16 gain_pit, G729_Word16 t0);
G729_Word16 g729_test_err(g729a_taming_state * state, G729_Word16 t0, G729_Word16 t0_frac);
    
/*--------------------------------------------------------------------------*
 * gain VQ functions.                                                       *
 *--------------------------------------------------------------------------*/
G729_Word16 g729_Qua_gain(
    g729a_encoder_state * state,
    G729_Word16 code[],        /* (i) Q13 : Innovative vector.                         */
    G729_Word16 g_coeff[],     /* (i)     : Correlations <xn y1> -2<y1 y1>             */
                               /*            <y2,y2>, -2<xn,y2>, 2<y1,y2>              */
    G729_Word16 exp_coeff[],   /* (i)    : Q-Format g_coeff[]                         */
    G729_Word16 L_subfr,       /* (i)     : Subframe length.                           */
    G729_Word16 *gain_pit,     /* (o) Q14 : Pitch gain.                                */
    G729_Word16 *gain_cod,     /* (o) Q1  : Code gain.                                 */
    G729_Word16 tameflag       /* (i)     : flag set to 1 if taming is needed          */
);
    
#ifdef __cplusplus
}
#endif

#endif  /* __G729_ENCODER_H__ */
/* end of file */
