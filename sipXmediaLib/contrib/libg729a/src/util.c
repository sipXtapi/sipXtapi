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
 * Function  Set zero()                                              *
 *           ~~~~~~~~~~                                              *
 * Set vector x[] to zero                                            *
 *-------------------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"

void g729_Set_zero(
    G729_Word16 x[],       /* (o)    : vector to clear     */
    G729_Word16 L          /* (i)    : length of vector    */
)
{
    G729_Word16 i;
    
    for (i = 0; i < L; i++)
        x[i] = 0;
    
    return;
}
/*-------------------------------------------------------------------*
 * Function  g729_Copy:                                              *
 *           ~~~~~                                                   *
 * g729_Copy vector x[] to y[]                                       *
 *-------------------------------------------------------------------*/

void g729_Copy(
    G729_Word16 x[],      /* (i)   : input vector   */
    G729_Word16 y[],      /* (o)   : output vector  */
    G729_Word16 L         /* (i)   : vector length  */
)
{
    G729_Word16 i;
    
    for (i = 0; i < L; i++)
        y[i] = x[i];
    
    return;
}

/* g729_Random generator  */

G729_Word16 g729_Random()
{
    static G729_Word16 seed = 21845;
    
    /* seed = seed*31821 + 13849; */
    seed = g729_extract_l(g729_L_add(g729_L_shr(g729_L_mult(seed, 31821), 1), 13849L));
    
    return(seed);
}

