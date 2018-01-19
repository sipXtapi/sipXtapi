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

/*-----------------------------------------------------------------*
 *   Functions g729_Coder_ld8a and g729_Init_Coder_ld8a            *
 *             ~~~~~~~~~~     ~~~~~~~~~~~~~~~                      *
 *                                                                 *
 *  g729_Init_Coder_ld8a(void);                                    *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *                                                                 *
 *                                                                 *
 *  g729_Coder_ld8a(G729_Word16 ana[]);                            *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"

#include "g729a_errors.h"
#include "g729a_encoder.h"

/*-----------------------------------------------------------*
 *    Coder constant parameters (defined in "ld8a.h")        *
 *-----------------------------------------------------------*
 *   L_WINDOW    : LPC analysis window size.                 *
 *   L_NEXT      : Samples of next frame needed for autocor. *
 *   L_FRAME     : Frame size.                               *
 *   L_SUBFR     : Sub-frame size.                           *
 *   M           : LPC order.                                *
 *   MP1         : LPC order+1                               *
 *   L_TOTAL     : Total size of speech buffer.              *
 *   PIT_MIN     : Minimum pitch lag.                        *
 *   PIT_MAX     : Maximum pitch lag.                        *
 *   L_INTERPOL  : Length of filter for interpolation        *
 *-----------------------------------------------------------*/

/*--------------------------------------------------------*
 *         Static memory allocation.                      *
 *--------------------------------------------------------*/

static G729_Word16 g729_lsp_old[M]={30000, 26000, 21000, 15000, 8000, 0, -8000,-15000,-21000,-26000};

/*-----------------------------------------------------------------*
 *   Function  g729_Init_Coder_ld8a                                     *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *  g729_Init_Coder_ld8a(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *       - initialize pointers to speech buffer                    *
 *       - initialize static  pointers                             *
 *       - set static vectors to zero                              *
 *                                                                 *
 *-----------------------------------------------------------------*/

void g729_Init_Coder_ld8a(g729a_encoder_state * state)
{
    int i;
    
    state->error = G729A_NO_ERROR;
    
    /*----------------------------------------------------------------------*
     *      Initialize pointers to speech vector.                            *
     *                                                                       *
     *                                                                       *
     *   |--------------------|-------------|-------------|------------|     *
     *     previous speech           sf1           sf2         L_NEXT        *
     *                                                                       *
     *   <----------------  Total speech vector (L_TOTAL)   ----------->     *
     *   <----------------  LPC analysis window (L_WINDOW)  ----------->     *
     *   |                   <-- present frame (L_FRAME) -->                 *
     * old_speech            |              <-- new speech (L_FRAME) -->     *
     * p_window              |              |                                *
     *                     speech           |                                *
     *                             new_speech                                *
     *-----------------------------------------------------------------------*/
    
    state->new_speech = state->old_speech + L_TOTAL - L_FRAME;         /* New speech     */
    state->speech     = state->new_speech - L_NEXT;                    /* Present frame  */
    state->p_window   = state->old_speech + L_TOTAL - L_WINDOW;        /* For LPC window */
    
    /* Initialize static pointers */
    
    state->wsp    = state->old_wsp + PIT_MAX;
    state->exc    = state->old_exc + PIT_MAX + L_INTERPOL;
    
    /* Static vectors to zero */
    
    g729_Set_zero(state->old_speech, L_TOTAL);
    g729_Set_zero(state->old_exc, PIT_MAX+L_INTERPOL);
    g729_Set_zero(state->old_wsp, PIT_MAX);
    g729_Set_zero(state->mem_w,   M);
    g729_Set_zero(state->mem_w0,  M);
    g729_Set_zero(state->mem_zero, M);
    state->sharp = SHARPMIN;
    
    /* Initialize lsp_old[] & lsp_old_q[] */
    g729_Copy(g729_lsp_old, state->lsp_old, M);
    g729_Copy(state->lsp_old, state->lsp_old_q, M);
    
    for ( i = 0; i < 4; ++i ) state->past_qua_en[i] = -14336;
    
    g729_Lsp_encw_reset(&(state->lspenc_state));
    g729_Init_exc_err(&(state->taming_state));
    
    return;
}

/*-----------------------------------------------------------------*
 *   Functions g729_Coder_ld8a                                     *
 *            ~~~~~~~~~~                                           *
 *  g729_Coder_ld8a(G729_Word16 ana[]);                            *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/

void g729_Coder_ld8a(
    g729a_encoder_state * state,
    G729_Word16 ana[]       /* output  : Analysis parameters */
)
{
    
    /* LPC analysis */
    
    G729_Word16 Aq_t[(MP1)*2];         /* A(z)   quantized for the 2 subframes */
    G729_Word16 Ap_t[(MP1)*2];         /* A(z/gamma)       for the 2 subframes */
    G729_Word16 *Aq, *Ap;              /* Pointer on Aq_t and Ap_t             */
    
    /* Other vectors */
    
    G729_Word16 h1[L_SUBFR];            /* Impulse response h1[]              */
    G729_Word16 xn[L_SUBFR];            /* Target vector for pitch search     */
    G729_Word16 xn2[L_SUBFR];           /* Target vector for codebook search  */
    G729_Word16 code[L_SUBFR];          /* Fixed codebook excitation          */
    G729_Word16 y1[L_SUBFR];            /* Filtered adaptive excitation       */
    G729_Word16 y2[L_SUBFR];            /* Filtered fixed codebook excitation */
    G729_Word16 g_coeff[4];             /* Correlations between xn & y1       */
    
    G729_Word16 g_coeff_cs[5];
    G729_Word16 exp_g_coeff_cs[5];      /* Correlations between xn, y1, & y2
                                                 <y1,y1>, -2<xn,y1>,
                                                 <y2,y2>, -2<xn,y2>, 2<y1,y2> */
    
    /* Scalars */
    
    G729_Word16 i, j, k, i_subfr;
    G729_Word16 T_op, T0, T0_min, T0_max, T0_frac;
    G729_Word16 gain_pit, gain_code, index;
    G729_Word16 temp, taming;
    G729_Word32 L_temp;
    
    /*------------------------------------------------------------------------*
     *  - Perform LPC analysis:                                               *
     *       * autocorrelation + lag windowing                                *
     *       * g729_Levinson-durbin algorithm to find a[]                          *
     *       * convert a[] to lsp[]                                           *
     *       * quantize and code the LSPs                                     *
     *       * find the interpolated LSPs and convert to a[] for the 2        *
     *         subframes (both quantized and unquantized)                     *
     *------------------------------------------------------------------------*/
    {
        /* Temporary vectors */
        G729_Word16 r_l[MP1], r_h[MP1];       /* Autocorrelations low and hi          */
        G729_Word16 rc[M];                    /* Reflection coefficients.             */
        G729_Word16 lsp_new[M], lsp_new_q[M]; /* LSPs at 2th subframe                 */
        
        /* LP analysis */
        
        g729_Autocorr(state->p_window, M, r_h, r_l);       /* Autocorrelations */
        g729_Lag_window(M, r_h, r_l);                      /* Lag windowing    */
        g729_Levinson(r_h, r_l, Ap_t, rc);                 /* g729_Levinson Durbin  */
        g729_Az_lsp(Ap_t, lsp_new, state->lsp_old);        /* From A(z) to lsp */
        
        /* LSP quantization */
        
        g729_Qua_lsp(&(state->lspenc_state), lsp_new, lsp_new_q, ana);
        ana += 2;                         /* Advance analysis parameters pointer */
        
        /*--------------------------------------------------------------------*
         * Find interpolated LPC parameters in all subframes                  *
         * The interpolated parameters are in array Aq_t[].                   *
         *--------------------------------------------------------------------*/
        
        g729_Int_qlpc(state->lsp_old_q, lsp_new_q, Aq_t);
        
        /* Compute A(z/gamma) */
        
        g729_Weight_Az(&Aq_t[0],   GAMMA1, M, &Ap_t[0]);
        g729_Weight_Az(&Aq_t[MP1], GAMMA1, M, &Ap_t[MP1]);
        
        /* update the LSPs for the next frame */
        
        g729_Copy(lsp_new,   state->lsp_old,   M);
        g729_Copy(lsp_new_q, state->lsp_old_q, M);
    }
    
    /*----------------------------------------------------------------------*
     * - Find the weighted input speech w_sp[] for the whole speech frame   *
     * - Find the open-loop pitch delay                                     *
     *----------------------------------------------------------------------*/
    
    g729_Residu(&Aq_t[0], &(state->speech[0]), &(state->exc[0]), L_SUBFR);
    g729_Residu(&Aq_t[MP1], &(state->speech[L_SUBFR]), &(state->exc[L_SUBFR]), L_SUBFR);
    
    {
        G729_Word16 Ap1[MP1];
        
        Ap = Ap_t;
        Ap1[0] = 4096;
        for(i=1; i<=M; i++)    /* Ap1[i] = Ap[i] - 0.7 * Ap[i-1]; */
            Ap1[i] = g729_sub(Ap[i], g729_mult(Ap[i-1], 22938));
        g729_Syn_filt(Ap1, &(state->exc[0]), &(state->wsp[0]), L_SUBFR, state->mem_w, 1);
        
        Ap += MP1;
        for(i=1; i<=M; i++)    /* Ap1[i] = Ap[i] - 0.7 * Ap[i-1]; */
            Ap1[i] = g729_sub(Ap[i], g729_mult(Ap[i-1], 22938));
        g729_Syn_filt(Ap1, &(state->exc[L_SUBFR]), &(state->wsp[L_SUBFR]), L_SUBFR, state->mem_w, 1);
    }
    
    /* Find open loop pitch lag */
    
    T_op = g729_Pitch_ol_fast(state->wsp, PIT_MAX, L_FRAME);
    
    /* Range for closed loop pitch search in 1st subframe */
    
    T0_min = g729_sub(T_op, 3);
    if (g729_sub(T0_min,PIT_MIN)<0) {
        T0_min = PIT_MIN;
    }
    
    T0_max = g729_add(T0_min, 6);
    if (g729_sub(T0_max ,PIT_MAX)>0)
    {
        T0_max = PIT_MAX;
        T0_min = g729_sub(T0_max, 6);
    }
    
    
    /*------------------------------------------------------------------------*
     *          Loop for every subframe in the analysis frame                 *
     *------------------------------------------------------------------------*
     *  To find the pitch and innovation parameters. The subframe size is     *
     *  L_SUBFR and the loop is repeated 2 times.                             *
     *     - find the weighted LPC coefficients                               *
     *     - find the LPC residual signal res[]                               *
     *     - compute the target signal for pitch search                       *
     *     - compute impulse response of weighted synthesis filter (h1[])     *
     *     - find the closed-loop pitch parameters                            *
     *     - encode the pitch delay                                           *
     *     - find target vector for codebook search                           *
     *     - codebook search                                                  *
     *     - VQ of pitch and codebook gains                                   *
     *     - update states of weighting filter                                *
     *------------------------------------------------------------------------*/
    
    Aq = Aq_t;    /* pointer to interpolated quantized LPC parameters */
    Ap = Ap_t;    /* pointer to weighted LPC coefficients             */
    
    for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR)
    {
        
        /*---------------------------------------------------------------*
         * Compute impulse response, h1[], of weighted synthesis filter  *
         *---------------------------------------------------------------*/
        
        h1[0] = 4096;
        g729_Set_zero(&h1[1], L_SUBFR-1);
        g729_Syn_filt(Ap, h1, h1, L_SUBFR, &h1[1], 0);
        
        /*----------------------------------------------------------------------*
         *  Find the target vector for pitch search:                            *
         *----------------------------------------------------------------------*/
        
        g729_Syn_filt(Ap, &(state->exc[i_subfr]), xn, L_SUBFR, state->mem_w0, 0);
        
        /*---------------------------------------------------------------------*
         *                 Closed-loop fractional pitch search                 *
         *---------------------------------------------------------------------*/
        
        T0 = g729_Pitch_fr3_fast(&(state->exc[i_subfr]), xn, h1, L_SUBFR, T0_min, T0_max,
                                 i_subfr, &T0_frac);
        
        index = g729_Enc_lag3(T0, T0_frac, &T0_min, &T0_max,PIT_MIN,PIT_MAX,i_subfr);
        
        *ana++ = index;
        
        if (i_subfr == 0) {
            *ana++ = g729_Parity_Pitch(index);
        }
        
        /*-----------------------------------------------------------------*
         *   - find filtered pitch exc                                     *
         *   - compute pitch gain and limit between 0 and 1.2              *
         *   - update target vector for codebook search                    *
         *-----------------------------------------------------------------*/
        
        g729_Syn_filt(Ap, &(state->exc[i_subfr]), y1, L_SUBFR, state->mem_zero, 0);
        
        gain_pit = g729_G_pitch(xn, y1, g_coeff, L_SUBFR);
        
        /* clip pitch gain if taming is necessary */
        
        taming = g729_test_err(&(state->taming_state), T0, T0_frac);
        
        if( taming == 1){
            if (g729_sub(gain_pit, GPCLIP) > 0) {
                gain_pit = GPCLIP;
            }
        }
        
        /* xn2[i]   = xn[i] - y1[i] * gain_pit  */
        
        for (i = 0; i < L_SUBFR; i++)
        {
            L_temp = g729_L_mult(y1[i], gain_pit);
            L_temp = g729_L_shl(L_temp, 1);               /* gain_pit in Q14 */
            xn2[i] = g729_sub(xn[i], g729_extract_h(L_temp));
        }
        
        
        /*-----------------------------------------------------*
         * - Innovative codebook search.                       *
         *-----------------------------------------------------*/
        
        index = g729_ACELP_Code_A(xn2, h1, T0, state->sharp, code, y2, &i);
        
        *ana++ = index;        /* Positions index */
        *ana++ = i;            /* Signs index     */
        
        
        /*-----------------------------------------------------*
         * - Quantization of gains.                            *
         *-----------------------------------------------------*/
        
        g_coeff_cs[0]     = g_coeff[0];            /* <y1,y1> */
        exp_g_coeff_cs[0] = g729_negate(g_coeff[1]);    /* Q-Format:XXX -> JPN */
        g_coeff_cs[1]     = g729_negate(g_coeff[2]);    /* (xn,y1) -> -2<xn,y1> */
        exp_g_coeff_cs[1] = g729_negate(g729_add(g_coeff[3], 1)); /* Q-Format:XXX -> JPN */
        
        g729_Corr_xy2( xn, y1, y2, g_coeff_cs, exp_g_coeff_cs );  /* Q0 Q0 Q12 ^Qx ^Q0 */
        /* g_coeff_cs[3]:exp_g_coeff_cs[3] = <y2,y2>   */
        /* g_coeff_cs[4]:exp_g_coeff_cs[4] = -2<xn,y2> */
        /* g_coeff_cs[5]:exp_g_coeff_cs[5] = 2<y1,y2>  */
        
        *ana++ = g729_Qua_gain(state, code, g_coeff_cs, exp_g_coeff_cs,
                               L_SUBFR, &gain_pit, &gain_code, taming);
        
        
        /*------------------------------------------------------------*
         * - Update pitch sharpening "sharp" with quantized gain_pit  *
         *------------------------------------------------------------*/
        
        state->sharp = gain_pit;
        if (g729_sub(state->sharp, SHARPMAX) > 0) { state->sharp = SHARPMAX;         }
        if (g729_sub(state->sharp, SHARPMIN) < 0) { state->sharp = SHARPMIN;         }
        
        /*------------------------------------------------------*
         * - Find the total excitation                          *
         * - update filters memories for finding the target     *
         *   vector in the next subframe                        *
         *------------------------------------------------------*/
        
        for (i = 0; i < L_SUBFR;  i++)
        {
            /* exc[i] = gain_pit*exc[i] + gain_code*code[i]; */
            /* exc[i]  in Q0   gain_pit in Q14               */
            /* code[i] in Q13  gain_cod in Q1                */
            
            L_temp = g729_L_mult(state->exc[i+i_subfr], gain_pit);
            L_temp = g729_L_mac(L_temp, code[i], gain_code);
            L_temp = g729_L_shl(L_temp, 1);
            state->exc[i+i_subfr] = g729_round(L_temp);
        }
        
        g729_update_exc_err(&(state->taming_state), gain_pit, T0);
        
        for (i = L_SUBFR-M, j = 0; i < L_SUBFR; i++, j++)
        {
            temp       = g729_extract_h(g729_L_shl( g729_L_mult(y1[i], gain_pit),  1) );
            k          = g729_extract_h(g729_L_shl( g729_L_mult(y2[i], gain_code), 2) );
            state->mem_w0[j]  = g729_sub(xn[i], g729_add(temp, k));
        }
        
        Aq += MP1;           /* interpolated LPC parameters for next subframe */
        Ap += MP1;
        
    }
    
    /*--------------------------------------------------*
     * Update signal for next frame.                    *
     * -> shift to the left by L_FRAME:                 *
     *     speech[], wsp[] and  exc[]                   *
     *--------------------------------------------------*/
    
    g729_Copy(&(state->old_speech[L_FRAME]), &(state->old_speech[0]), L_TOTAL-L_FRAME);
    g729_Copy(&(state->old_wsp[L_FRAME]), &(state->old_wsp[0]), PIT_MAX);
    g729_Copy(&(state->old_exc[L_FRAME]), &(state->old_exc[0]), PIT_MAX+L_INTERPOL);
    
    return;
}

