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

/*-----------------------------------------------------------*
 *  Function  g729_Decod_ACELP()                             *
 *  ~~~~~~~~~~~~~~~~~~~~~~~                                  *
 *   Algebraic codebook decoder.                             *
 *----------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"

void g729_Decod_ACELP(
                      G729_Word16 sign,      /* (i)     : signs of 4 pulses.                       */
                      G729_Word16 index,     /* (i)     : Positions of the 4 pulses.               */
                      G729_Word16 cod[]      /* (o) Q13 : algebraic (fixed) codebook excitation    */
)
{
    G729_Word16 i, j;
    G729_Word16 pos[4];
    
    
    
    /* Decode the positions */
    
    i      = index & (G729_Word16)7;
    pos[0] = g729_add(i, g729_shl(i, 2));           /* pos0 =i*5 */
    
    index  = g729_shr(index, 3);
    i      = index & (G729_Word16)7;
    i      = g729_add(i, g729_shl(i, 2));           /* pos1 =i*5+1 */
    pos[1] = g729_add(i, 1);
    
    index  = g729_shr(index, 3);
    i      = index & (G729_Word16)7;
    i      = g729_add(i, g729_shl(i, 2));           /* pos2 =i*5+1 */
    pos[2] = g729_add(i, 2);
    
    index  = g729_shr(index, 3);
    j      = index & (G729_Word16)1;
    index  = g729_shr(index, 1);
    i      = index & (G729_Word16)7;
    i      = g729_add(i, g729_shl(i, 2));           /* pos3 =i*5+3+j */
    i      = g729_add(i, 3);
    pos[3] = g729_add(i, j);
    
    /* decode the signs  and build the codeword */
    
    for (i=0; i<L_SUBFR; i++) {
        cod[i] = 0;
    }
    
    for (j=0; j<4; j++)
    {
        
        i = sign & (G729_Word16)1;
        sign = g729_shr(sign, 1);
        
        if (i != 0) {
            cod[pos[j]] = 8191;      /* Q13 +1.0 */
        }
        else {
            cod[pos[j]] = -8192;     /* Q13 -1.0 */
        }
    }
    
    return;
}

