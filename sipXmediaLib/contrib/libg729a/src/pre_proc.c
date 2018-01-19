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

/*------------------------------------------------------------------------*
 * Function g729_Pre_Process()                                            *
 *                                                                        *
 * Preprocessing of input speech.                                         *
 *   - 2nd order high pass filter with cut off frequency at 140 Hz.       *
 *   - Divide input by two.                                               *
 *-----------------------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "oper_32b.h"

#include "ld8a.h"
#include "tab_ld8a.h"

#include "g729a_encoder.h"

/*------------------------------------------------------------------------*
 * 2nd order high pass filter with cut off frequency at 140 Hz.           *
 * Designed with SPPACK efi command -40 dB att, 0.25 ri.                  *
 *                                                                        *
 * Algorithm:                                                             *
 *                                                                        *
 *  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b[2]*x[i-2]/2                    *
 *                     + a[1]*y[i-1]   + a[2]*y[i-2];                     *
 *                                                                        *
 *     b[3] = {0.92727435E+00, -0.18544941E+01, 0.92727435E+00};          *
 *     a[3] = {0.10000000E+01, 0.19059465E+01, -0.91140240E+00};          *
 *                                                                        *
 *  Input are divided by two in the filtering process.                    *
 *-----------------------------------------------------------------------*/

/* filter coefficients (fc = 140 Hz, coeff. b[] is divided by 2) */

static G729_Word16 g729_b140[3] = {1899, -3798, 1899};      /* 1/2 in Q12 */
static G729_Word16 g729_a140[3] = {4096, 7807, -3733};      /* Q12 */

/* Static values to be preserved between calls */
/* y[] values is keep in double precision      */

/* Initialization of static values */

void g729_Init_Pre_Process(g729a_pre_process_state * state)
{
    state->y2_hi = 0;
    state->y2_lo = 0;
    state->y1_hi = 0;
    state->y1_lo = 0;
    state->x0    = 0;
    state->x1    = 0;
}


void g729_Pre_Process(
    g729a_pre_process_state * state,
    G729_Word16 singal_in[],     /* input signal */
    G729_Word16 signal_out[],    /* output signal */
    G729_Word16 lg)              /* length of signal    */
{
    G729_Word16 i, x2;
    G729_Word32 L_tmp;
    
    for(i=0; i<lg; i++)
    {
        x2 = state->x1;
        state->x1 = state->x0;
        state->x0 = singal_in[i];
        
        /*  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + g729_b140[2]*x[i-2]/2  */
        /*                     + a[1]*y[i-1] + a[2] * y[i-2];      */
        
        L_tmp     = g729_Mpy_32_16(state->y1_hi, state->y1_lo, g729_a140[1]);
        L_tmp     = g729_L_add(L_tmp, g729_Mpy_32_16(state->y2_hi, state->y2_lo, g729_a140[2]));
        L_tmp     = g729_L_mac(L_tmp, state->x0, g729_b140[0]);
        L_tmp     = g729_L_mac(L_tmp, state->x1, g729_b140[1]);
        L_tmp     = g729_L_mac(L_tmp, x2, g729_b140[2]);
        L_tmp     = g729_L_shl(L_tmp, 3);      /* Q28 --> Q31 (Q12 --> Q15) */
        
        signal_out[i] = g729_round(L_tmp);
        
        state->y2_hi = state->y1_hi;
        state->y2_lo = state->y1_lo;
        g729_L_Extract(L_tmp, &(state->y1_hi), &(state->y1_lo));
    }
    return;
}

