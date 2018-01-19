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
 * Function  g729_Convolve:                                          *
 *           ~~~~~~~~~                                               *
 *-------------------------------------------------------------------*
 * Perform the convolution between two vectors x[] and h[] and       *
 * write the result in the vector y[].                               *
 * All vectors are of length N.                                      *
 *-------------------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"

void g729_Convolve(
    G729_Word16 x[],      /* (i)     : input vector                           */
    G729_Word16 h[],      /* (i) Q12 : impulse response                       */
    G729_Word16 y[],      /* (o)     : output vector                          */
    G729_Word16 L         /* (i)     : vector size                            */
)
{
    G729_Word16 i, n;
    G729_Word32 s;
    
    for (n = 0; n < L; n++)
    {
        s = 0;
        for (i = 0; i <= n; i++)
            s = g729_L_mac(s, x[i], h[n-i]);
        
        s    = g729_L_shl(s, 3);                   /* h is in Q12 and saturation */
        y[n] = g729_extract_h(s);
    }
    
    return;
}

/*-----------------------------------------------------*
 * procedure g729_Syn_filt:                            *
 *           ~~~~~~~~                                  *
 * Do the synthesis filtering 1/A(z).                  *
 *-----------------------------------------------------*/

G729_Flag g729_Syn_filt_Overflow(   /* output: if overflow, return Non-Zero, otherwise return 0 */
    G729_Word16 a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
    G729_Word16 x[],     /* (i)     : input signal                             */
    G729_Word16 y[],     /* (o)     : output signal                            */
    G729_Word16 lg,      /* (i)     : size of filtering                        */
    G729_Word16 mem[]    /* (i/o)   : memory associated with this filtering.   */
)
{
    G729_Word16 i, j;
    G729_Word32 s, t;
    G729_Word16 tmp[100];     /* This is usually done by memory allocation (lg+M) */
    G729_Word16 *yy;
    
    /* g729_Copy mem[] to yy[] */
    
    yy = tmp;
    
    for(i=0; i<M; i++)
    {
        *yy++ = mem[i];
    }
    
    /* Do the filtering. */
    
    for (i = 0; i < lg; ++i)
    {
        s = x[i] * a[0];
        for (j = 1; j <= M; ++j)
        {
            s -= a[j] * yy[-j];
        }
        
        t = s << 4;
        if (t >> 4 != s)
        {
            *yy++ = s & G729A_MIN_32 ? G729A_MIN_16 : G729A_MAX_16;
            return -1;
        }
        else
        {
            *yy++ = (G729_Word16)((t + (G729_Word32)0x8000L) >> 16);
        }
    }
    
    for(i=0; i<lg; i++)
    {
        y[i] = tmp[i+M];
    }
    
    return 0;
}

void g729_Syn_filt(
    G729_Word16 a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
    G729_Word16 x[],     /* (i)     : input signal                             */
    G729_Word16 y[],     /* (o)     : output signal                            */
    G729_Word16 lg,      /* (i)     : size of filtering                        */
    G729_Word16 mem[],   /* (i/o)   : memory associated with this filtering.   */
    G729_Word16 update   /* (i)     : 0=no update, 1=update of memory.         */
)
{
    G729_Word16 i, j;
    G729_Word32 s;
    G729_Word16 tmp[100];     /* This is usually done by memory allocation (lg+M) */
    G729_Word16 *yy;
    
    /* g729_Copy mem[] to yy[] */
    
    yy = tmp;
    
    for(i=0; i<M; i++)
    {
        *yy++ = mem[i];
    }
    
    /* Do the filtering. */
    
    for (i = 0; i < lg; i++)
    {
        s = g729_L_mult(x[i], a[0]);
        for (j = 1; j <= M; j++)
        {
            s = g729_L_msu(s, a[j], yy[-j]);
        }
        
        s = g729_L_shl(s, 3);
        *yy++ = g729_round(s);
    }
    
    for(i=0; i<lg; i++)
    {
        y[i] = tmp[i+M];
    }
    
    /* Update of memory if update==1 */
    
    if(update != 0)
    {
        for (i = 0; i < M; i++)
        {
            mem[i] = y[lg-M+i];
        }
    }
    
    return;
}

/*-----------------------------------------------------------------------*
 * procedure g729_Residu:                                                *
 *           ~~~~~~                                                      *
 * Compute the LPC residual  by filtering the input speech through A(z)  *
 *-----------------------------------------------------------------------*/

void g729_Residu(
    G729_Word16 a[],    /* (i) Q12 : prediction coefficients                     */
    G729_Word16 x[],    /* (i)     : speech (values x[-m..-1] are needed         */
    G729_Word16 y[],    /* (o)     : residual signal                             */
    G729_Word16 lg      /* (i)     : size of filtering                           */
)
{
    G729_Word16 i, j;
    G729_Word32 s;
    
    for (i = 0; i < lg; i++)
    {
        s = g729_L_mult(x[i], a[0]);
        for (j = 1; j <= M; j++)
            s = g729_L_mac(s, a[j], x[i-j]);
        
        s = g729_L_shl(s, 3);
        y[i] = g729_round(s);
    }
    return;
}

