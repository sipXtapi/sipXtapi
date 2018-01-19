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

/*-------------------------------------------------------------------*
 * Function  g729_Pred_lt_3()                                        *
 *           ~~~~~~~~~~~                                             *
 *-------------------------------------------------------------------*
 * Compute the result of long term prediction with fractional        *
 * interpolation of resolution 1/3.                                  *
 *                                                                   *
 * On return exc[0..L_subfr-1] contains the interpolated signal      *
 *   (adaptive codebook excitation)                                  *
 *-------------------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"
#include "tab_ld8a.h"

void g729_Pred_lt_3(
    G729_Word16   exc[],       /* in/out: excitation buffer */
    G729_Word16   T0,          /* input : integer pitch lag */
    G729_Word16   frac,        /* input : fraction of lag   */
    G729_Word16   L_subfr      /* input : subframe size     */
)
{
    G729_Word16  i, j, k;
    G729_Word16  *x0, *x1, *x2, *c1, *c2;
    G729_Word32  s;
    
    x0 = &exc[-T0];
    
    frac = g729_negate(frac);
    if (frac < 0)
    {
        frac = g729_add(frac, UP_SAMP);
        x0--;
    }
    
    for (j=0; j<L_subfr; j++)
    {
        x1 = x0++;
        x2 = x0;
        c1 = &g729_inter_3l[frac];
        c2 = &g729_inter_3l[g729_sub(UP_SAMP,frac)];
        
        s = 0;
        for(i=0, k=0; i< L_INTER10; i++, k+=UP_SAMP)
        {
            s = g729_L_mac(s, x1[-i], c1[k]);
            s = g729_L_mac(s, x2[i],  c2[k]);
        }
        
        exc[j] = g729_round(s);
    }
    
    return;
}

