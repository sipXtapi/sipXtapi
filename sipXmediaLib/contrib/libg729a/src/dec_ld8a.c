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

/*-----------------------------------------------------------------*
 *   Functions g729_Init_Decod_ld8a  and g729_Decod_ld8a                     *
 *-----------------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"

#include "g729a_errors.h"
#include "g729a_decoder.h"

/*---------------------------------------------------------------*
 *   Decoder constant parameters (defined in "ld8a.h")           *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   L_SUBFR     : Sub-frame size.                               *
 *   M           : LPC order.                                    *
 *   MP1         : LPC order+1                                   *
 *   PIT_MIN     : Minimum pitch lag.                            *
 *   PIT_MAX     : Maximum pitch lag.                            *
 *   L_INTERPOL  : Length of filter for interpolation            *
 *   PRM_SIZE    : Size of vector containing analysis parameters *
 *---------------------------------------------------------------*/

/*--------------------------------------------------------*
 *         Static memory allocation.                      *
 *--------------------------------------------------------*/

static G729_Word16 g729_lsp_old[M]={30000, 26000, 21000, 15000, 8000, 0, -8000,-15000,-21000,-26000};

/*-----------------------------------------------------------------*
 *   Function g729_Init_Decod_ld8a                                      *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *   ->Initialization of variables for the decoder section.        *
 *                                                                 *
 *-----------------------------------------------------------------*/

void g729_Init_Decod_ld8a(g729a_decoder_state * state)
{
    int i;
    
    state->error = G729A_NO_ERROR;
    
    /* Initialize static pointer */
    state->exc = state->old_exc + PIT_MAX + L_INTERPOL;
    
    /* Static vectors to zero */
    g729_Set_zero(state->old_exc, PIT_MAX+L_INTERPOL);
    g729_Set_zero(state->mem_syn, M);
    
    /* Initialize lsp_old[] */
    g729_Copy(g729_lsp_old, state->lsp_old, M);
    
    state->sharp  = SHARPMIN;
    state->old_T0 = 60;
    state->gain_code = 0;
    state->gain_pitch = 0;
    
    for ( i = 0; i < 4; ++i ) state->past_qua_en[i] = -14336;
    
    g729_Lsp_decw_reset(&(state->lspdec_state));
    return;
}

/*-----------------------------------------------------------------*
 *   Function g729_Decod_ld8a                                           *
 *           ~~~~~~~~~~                                            *
 *   ->Main decoder routine.                                       *
 *                                                                 *
 *-----------------------------------------------------------------*/

void g729_Decod_ld8a(
    g729a_decoder_state * state,
    G729_Word16  parm[],      /* (i)   : vector of synthesis parameters
                                         parm[0] = bad frame indicator (bfi)  */
    G729_Word16  synth[],     /* (o)   : synthesis speech                     */
    G729_Word16  A_t[],       /* (o)   : decoded LP filter in 2 subframes     */
    G729_Word16  *T2,         /* (o)   : decoded pitch lag in 2 subframes     */
    G729_Word16 bad_lsf       /* (i)   : bad LSF indicator   */
)
{
    G729_Word16  *Az;                  /* Pointer on A_t   */
    G729_Word16  lsp_new[M];           /* LSPs             */
    G729_Word16  code[L_SUBFR];        /* ACELP codevector */
    
    /* Scalars */
    
    G729_Word16  i, j, i_subfr;
    G729_Word16  T0, T0_frac, index;
    G729_Word16  bfi;
    G729_Word32  L_temp;
    
    G729_Word16 bad_pitch;             /* bad pitch indicator */
    
    /* Test bad frame indicator (bfi) */
    
    bfi = *parm++;
    
    /* Decode the LSPs */
    
    g729_D_lsp(&(state->lspdec_state), parm, lsp_new, g729_add(bfi, bad_lsf));
    parm += 2;
    
    /*
     Note: "bad_lsf" is introduce in case the standard is used with
     channel protection.
     */
    
    /* Interpolation of LPC for the 2 subframes */
    
    g729_Int_qlpc(state->lsp_old, lsp_new, A_t);
    
    /* update the LSFs for the next frame */
    
    g729_Copy(lsp_new, state->lsp_old, M);
    
    /*------------------------------------------------------------------------*
     *          Loop for every subframe in the analysis frame                 *
     *------------------------------------------------------------------------*
     * The subframe size is L_SUBFR and the loop is repeated L_FRAME/L_SUBFR  *
     *  times                                                                 *
     *     - decode the pitch delay                                           *
     *     - decode algebraic code                                            *
     *     - decode pitch and codebook gains                                  *
     *     - find the excitation and compute synthesis speech                 *
     *------------------------------------------------------------------------*/
    
    Az = A_t;            /* pointer to interpolated LPC parameters */
    
    for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
    {
        
        index = *parm++;            /* pitch index */
        
        if(i_subfr == 0)
        {
            i = *parm++;              /* get parity check result */
            bad_pitch = g729_add(bfi, i);
            if( bad_pitch == 0)
            {
                g729_Dec_lag3(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac);
                state->old_T0 = T0;
            }
            else        /* Bad frame, or parity error */
            {
                T0  =  state->old_T0;
                T0_frac = 0;
                state->old_T0 = g729_add( state->old_T0, 1);
                if( g729_sub(state->old_T0, PIT_MAX) > 0) {
                    state->old_T0 = PIT_MAX;
                }
            }
        }
        else                  /* second subframe */
        {
            if( bfi == 0)
            {
                g729_Dec_lag3(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac);
                state->old_T0 = T0;
            }
            else
            {
                T0  =  state->old_T0;
                T0_frac = 0;
                state->old_T0 = g729_add( state->old_T0, 1);
                if( g729_sub(state->old_T0, PIT_MAX) > 0) {
                    state->old_T0 = PIT_MAX;
                }
            }
        }
        *T2++ = T0;
        
        /*-------------------------------------------------*
         * - Find the adaptive codebook vector.            *
         *-------------------------------------------------*/
        
        g729_Pred_lt_3(&(state->exc[i_subfr]), T0, T0_frac, L_SUBFR);
        
        /*-------------------------------------------------------*
         * - Decode innovative codebook.                         *
         * - Add the fixed-gain pitch contribution to code[].    *
         *-------------------------------------------------------*/
        
        if(bfi != 0)        /* Bad frame */
        {
            
            parm[0] = g729_Random() & (G729_Word16)0x1fff;     /* 13 bits random */
            parm[1] = g729_Random() & (G729_Word16)0x000f;     /*  4 bits random */
        }
        g729_Decod_ACELP(parm[1], parm[0], code);
        parm +=2;
        
        j = g729_shl(state->sharp, 1);          /* From Q14 to Q15 */
        if(g729_sub(T0, L_SUBFR) <0 ) {
            for (i = T0; i < L_SUBFR; i++) {
                code[i] = g729_add(code[i], g729_mult(code[i-T0], j));
            }
        }
        
        /*-------------------------------------------------*
         * - Decode pitch and codebook gains.              *
         *-------------------------------------------------*/
        
        index = *parm++;      /* index of energy VQ */
        
        g729_Dec_gain(state, index, code, L_SUBFR, bfi, &(state->gain_pitch), &(state->gain_code));
        
        /*-------------------------------------------------------------*
         * - Update pitch sharpening "sharp" with quantized gain_pitch *
         *-------------------------------------------------------------*/
        
        state->sharp = state->gain_pitch;
        if (g729_sub(state->sharp, SHARPMAX) > 0) { state->sharp = SHARPMAX;  }
        if (g729_sub(state->sharp, SHARPMIN) < 0) { state->sharp = SHARPMIN;  }
        
        /*-------------------------------------------------------*
         * - Find the total excitation.                          *
         * - Find synthesis speech corresponding to exc[].       *
         *-------------------------------------------------------*/
        
        for (i = 0; i < L_SUBFR;  i++)
        {
            /* exc[i] = gain_pitch*exc[i] + gain_code*code[i]; */
            /* exc[i]  in Q0   gain_pitch in Q14               */
            /* code[i] in Q13  gain_codeode in Q1              */
            
            L_temp = g729_L_mult(state->exc[i+i_subfr], state->gain_pitch);
            L_temp = g729_L_mac(L_temp, code[i], state->gain_code);
            L_temp = g729_L_shl(L_temp, 1);
            state->exc[i+i_subfr] = g729_round(L_temp);
        }
        
#if defined(USE_GLOBAL_OVERFLOW_FLAG) && (USE_GLOBAL_OVERFLOW_FLAG == 1)
        G729A_Overflow_Flag = 0;
        g729_Syn_filt(Az, &(state->exc[i_subfr]), &synth[i_subfr], L_SUBFR, state->mem_syn, 0);
        if(G729A_Overflow_Flag != 0)
#else
        if (g729_Syn_filt_Overflow(Az, &(state->exc[i_subfr]), &synth[i_subfr], L_SUBFR, state->mem_syn))
#endif
        {
            /* In case of overflow in the synthesis          */
            /* -> Scale down vector exc[] and redo synthesis */
            
            for(i=0; i<PIT_MAX+L_INTERPOL+L_FRAME; i++)
                state->old_exc[i] = g729_shr(state->old_exc[i], 2);
            
            g729_Syn_filt(Az, &(state->exc[i_subfr]), &synth[i_subfr], L_SUBFR, state->mem_syn, 1);
        }
        else
        {
            g729_Copy(&synth[i_subfr+L_SUBFR-M], state->mem_syn, M);
        }
        
        Az += MP1;    /* interpolated LPC parameters for next subframe */
    }
    
    /*--------------------------------------------------*
     * Update signal for next frame.                    *
     * -> shift to the left by L_FRAME  exc[]           *
     *--------------------------------------------------*/
    
    g729_Copy(&(state->old_exc[L_FRAME]), &(state->old_exc[0]), PIT_MAX+L_INTERPOL);
    
    return;
}

