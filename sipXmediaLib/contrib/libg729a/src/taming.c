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

/**************************************************************************
 * Taming functions.                                                      *
 **************************************************************************/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "ld8a.h"
#include "tab_ld8a.h"

#include "g729a_encoder.h"

void g729_Init_exc_err(g729a_taming_state * state)
{
    G729_Word16 i;
    for(i=0; i<4; i++) state->L_exc_err[i] = 0x00004000L;   /* Q14 */
}

/**************************************************************************
 * routine g729_test_err - computes the accumulated potential error       *
 * in the adaptive codebook contribution                                  *
 **************************************************************************/

G729_Word16 g729_test_err(  /* (o) flag set to 1 if taming is necessary  */
    g729a_taming_state * state,
    G729_Word16 T0,       /* (i) integer part of pitch delay           */
    G729_Word16 T0_frac   /* (i) fractional part of pitch delay        */
)
{
    G729_Word16 i, t1, zone1, zone2, flag;
    G729_Word32 L_maxloc, L_acc;
    
    if(T0_frac > 0) {
        t1 = g729_add(T0, 1);
    }
    else {
        t1 = T0;
    }
    
    i = g729_sub(t1, (L_SUBFR+L_INTER10));
    if(i < 0) {
        i = 0;
    }
    zone1 = g729_tab_zone[i];
    
    i = g729_add(t1, (L_INTER10 - 2));
    zone2 = g729_tab_zone[i];
    
    L_maxloc = -1L;
    flag = 0 ;
    for(i=zone2; i>=zone1; i--) {
        L_acc = g729_L_sub(state->L_exc_err[i], L_maxloc);
        if(L_acc > 0L) {
            L_maxloc = state->L_exc_err[i];
        }
    }
    L_acc = g729_L_sub(L_maxloc, L_THRESH_ERR);
    if(L_acc > 0L) {
        flag = 1;
    }
    
    return(flag);
}

/**************************************************************************
 * routine g729_update_exc_err - maintains the memory used to compute     *
 * the error function due to an adaptive codebook mismatch between        *
 * encoder and decoder                                                    *
 **************************************************************************/

void g729_update_exc_err(
    g729a_taming_state * state,
    G729_Word16 gain_pit,      /* (i) pitch gain */
    G729_Word16 T0             /* (i) integer part of pitch delay */
)
{
    
    G729_Word16 i, zone1, zone2, n;
    G729_Word32 L_worst, L_temp, L_acc;
    G729_Word16 hi, lo;
    
    L_worst = -1L;
    n = g729_sub(T0, L_SUBFR);
    
    if(n < 0) {
        g729_L_Extract(state->L_exc_err[0], &hi, &lo);
        L_temp = g729_Mpy_32_16(hi, lo, gain_pit);
        L_temp = g729_L_shl(L_temp, 1);
        L_temp = g729_L_add(0x00004000L, L_temp);
        L_acc = g729_L_sub(L_temp, L_worst);
        if(L_acc > 0L) {
            L_worst = L_temp;
        }
        g729_L_Extract(L_temp, &hi, &lo);
        L_temp = g729_Mpy_32_16(hi, lo, gain_pit);
        L_temp = g729_L_shl(L_temp, 1);
        L_temp = g729_L_add(0x00004000L, L_temp);
        L_acc = g729_L_sub(L_temp, L_worst);
        if(L_acc > 0L) {
            L_worst = L_temp;
        }
    }
    
    else {
        
        zone1 = g729_tab_zone[n];
        
        i = g729_sub(T0, 1);
        zone2 = g729_tab_zone[i];
        
        for(i = zone1; i <= zone2; i++) {
            g729_L_Extract(state->L_exc_err[i], &hi, &lo);
            L_temp = g729_Mpy_32_16(hi, lo, gain_pit);
            L_temp = g729_L_shl(L_temp, 1);
            L_temp = g729_L_add(0x00004000L, L_temp);
            L_acc = g729_L_sub(L_temp, L_worst);
            if(L_acc > 0L) L_worst = L_temp;
        }
    }
    
    for(i=3; i>=1; i--) {
        state->L_exc_err[i] = state->L_exc_err[i-1];
    }
    state->L_exc_err[0] = L_worst;
    
    return;
}


