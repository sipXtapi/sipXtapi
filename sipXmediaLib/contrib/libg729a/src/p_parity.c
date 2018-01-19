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

/*------------------------------------------------------*
 * Parity_pitch - compute parity bit for first 6 MSBs   *
 *------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"

G729_Word16 g729_Parity_Pitch(    /* output: parity bit (XOR of 6 MSB bits)    */
                              G729_Word16 pitch_index   /* input : index for which parity to compute */
)
{
    G729_Word16 temp, sum, i, bit;
    
    
    temp = g729_shr(pitch_index, 1);
    
    sum = 1;
    for (i = 0; i <= 5; i++) {
        temp = g729_shr(temp, 1);
        bit = temp & (G729_Word16)1;
        sum = g729_add(sum, bit);
    }
    sum = sum & (G729_Word16)1;
    
    
    return sum;
}

/*--------------------------------------------------------------------*
 * check_parity_pitch - check parity of index with transmitted parity *
 *--------------------------------------------------------------------*/

G729_Word16  g729_Check_Parity_Pitch( /* output: 0 = no error, 1= error */
                                     G729_Word16 pitch_index,       /* input : index of parameter     */
                                     G729_Word16 parity             /* input : parity bit             */
)
{
    G729_Word16 temp, sum, i, bit;
    
    
    temp = g729_shr(pitch_index, 1);
    
    sum = 1;
    for (i = 0; i <= 5; i++) {
        temp = g729_shr(temp, 1);
        bit = temp & (G729_Word16)1;
        sum = g729_add(sum, bit);
    }
    sum = g729_add(sum, parity);
    sum = sum & (G729_Word16)1;
    
    return sum;
}

