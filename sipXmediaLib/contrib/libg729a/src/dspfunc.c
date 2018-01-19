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

#include "g729a_typedef.h"
#include "basic_op.h"

#include "ld8a.h"
#include "tab_ld8a.h"

/*_______________________________________________________________________________
 |                                                                               |
 |   Function Name : g729_Pow2()                                                 |
 |                                                                               |
 |     L_x = pow(2.0, exponent.fraction)                                         |
 |-------------------------------------------------------------------------------|
 |  Algorithm:                                                                   |
 |                                                                               |
 |   The function g729_Pow2(L_x) is approximated by a table and linear           |
 |   interpolation.                                                              |
 |                                                                               |
 |   1- i = bit10-b15 of fraction,   0 <= i <= 31                                |
 |   2- a = bit0-b9   of fraction                                                |
 |   3- L_x = g729_tabpow[i]<<16 - (g729_tabpow[i] - g729_tabpow[i+1]) * a * 2   |
 |   4- L_x = L_x >> (30-exponent)     (with rounding)                           |
 |_______________________________________________________________________________|
 */


G729_Word32 g729_Pow2(        /* (o) Q0  : result       (range: 0<=val<=0x7fffffff) */
    G729_Word16 exponent,  /* (i) Q0  : Integer part.      (range: 0<=val<=30)   */
    G729_Word16 fraction   /* (i) Q15 : Fractional part.   (range: 0.0<=val<1.0) */
)
{
    G729_Word16 exp, i, a, tmp;
    G729_Word32 L_x;
    
    L_x = g729_L_mult(fraction, 32);           /* L_x = fraction<<6           */
    i   = g729_extract_h(L_x);                 /* Extract b10-b15 of fraction */
    L_x = g729_L_shr(L_x, 1);
    a   = g729_extract_l(L_x);                 /* Extract b0-b9   of fraction */
    a   = a & (G729_Word16)0x7fff;
    
    L_x = g729_L_deposit_h(g729_tabpow[i]);              /* g729_tabpow[i] << 16              */
    tmp = g729_sub(g729_tabpow[i], g729_tabpow[i+1]);    /* g729_tabpow[i] - g729_tabpow[i+1] */
    L_x = g729_L_msu(L_x, tmp, a);                       /* L_x -= tmp*a*2                    */
    
    exp = g729_sub(30, exponent);
    L_x = g729_L_shr_r(L_x, exp);
    
    return(L_x);
}

/*___________________________________________________________________________________
 |                                                                                   |
 |   Function Name : g729_Log2()                                                     |
 |                                                                                   |
 |       Compute g729_Log2(L_x).                                                     |
 |       L_x is positive.                                                            |
 |                                                                                   |
 |       if L_x is negative or zero, result is 0.                                    |
 |-----------------------------------------------------------------------------------|
 |  Algorithm:                                                                       |
 |                                                                                   |
 |   The function g729_Log2(L_x) is approximated by a table and linear               |
 |   interpolation.                                                                  |
 |                                                                                   |
 |   1- Normalization of L_x.                                                        |
 |   2- exponent = 30-exponent                                                       |
 |   3- i = bit25-b31 of L_x,    32 <= i <= 63  ->because of normalization.          |
 |   4- a = bit10-b24                                                                |
 |   5- i -=32                                                                       |
 |   6- fraction = g729_tablog[i]<<16 - (g729_tablog[i] - g729_tablog[i+1]) * a * 2  |
 |___________________________________________________________________________________|
 */

void g729_Log2(
    G729_Word32 L_x,       /* (i) Q0 : input value                                 */
    G729_Word16 *exponent, /* (o) Q0 : Integer part of g729_Log2.   (range: 0<=val<=30) */
    G729_Word16 *fraction  /* (o) Q15: Fractional  part of g729_Log2. (range: 0<=val<1) */
)
{
    G729_Word16 exp, i, a, tmp;
    G729_Word32 L_y;
    
    if( L_x <= (G729_Word32)0 )
    {
        *exponent = 0;
        *fraction = 0;
        return;
    }
    
    exp = g729_norm_l(L_x);
    L_x = g729_L_shl(L_x, exp );               /* L_x is normalized */
    
    *exponent = g729_sub(30, exp);
    
    L_x = g729_L_shr(L_x, 9);
    i   = g729_extract_h(L_x);                 /* Extract b25-b31 */
    L_x = g729_L_shr(L_x, 1);
    a   = g729_extract_l(L_x);                 /* Extract b10-b24 of fraction */
    a   = a & (G729_Word16)0x7fff;
    
    i   = g729_sub(i, 32);
    
    L_y = g729_L_deposit_h(g729_tablog[i]);              /* g729_tablog[i] << 16              */
    tmp = g729_sub(g729_tablog[i], g729_tablog[i+1]);    /* g729_tablog[i] - g729_tablog[i+1] */
    L_y = g729_L_msu(L_y, tmp, a);                       /* L_y -= tmp*a*2                    */
    
    *fraction = g729_extract_h( L_y);
    
    return;
}

/*______________________________________________________________________________
 |                                                                              |
 |   Function Name : g729_Inv_sqrt                                              |
 |                                                                              |
 |       Compute 1/sqrt(L_x).                                                   |
 |       L_x is positive.                                                       |
 |                                                                              |
 |       if L_x is negative or zero, result is 1 (3fff ffff).                   |
 |------------------------------------------------------------------------------|
 |  Algorithm:                                                                  |
 |                                                                              |
 |   The function 1/sqrt(L_x) is approximated by a table and linear             |
 |   interpolation.                                                             |
 |                                                                              |
 |   1- Normalization of L_x.                                                   |
 |   2- If (30-exponent) is even then shift right once.                         |
 |   3- exponent = (30-exponent)/2  +1                                          |
 |   4- i = bit25-b31 of L_x,    16 <= i <= 63  ->because of normalization.     |
 |   5- a = bit10-b24                                                           |
 |   6- i -=16                                                                  |
 |   7- L_y = g729_tabsqr[i]<<16 - (g729_tabsqr[i] - g729_tabsqr[i+1]) * a * 2  |
 |   8- L_y >>= exponent                                                        |
 |______________________________________________________________________________|
 */


G729_Word32 g729_Inv_sqrt(   /* (o) Q30 : output value   (range: 0<=val<1)           */
    G729_Word32 L_x          /* (i) Q0  : input value    (range: 0<=val<=7fffffff)   */
)
{
    G729_Word16 exp, i, a, tmp;
    G729_Word32 L_y;
    
    if( L_x <= (G729_Word32)0) return ( (G729_Word32)0x3fffffffL);
    
    exp = g729_norm_l(L_x);
    L_x = g729_L_shl(L_x, exp );               /* L_x is normalize */
    
    exp = g729_sub(30, exp);
    if( (exp & 1) == 0 )                  /* If exponent even -> shift right */
        L_x = g729_L_shr(L_x, 1);
    
    exp = g729_shr(exp, 1);
    exp = g729_add(exp, 1);
    
    L_x = g729_L_shr(L_x, 9);
    i   = g729_extract_h(L_x);                 /* Extract b25-b31 */
    L_x = g729_L_shr(L_x, 1);
    a   = g729_extract_l(L_x);                 /* Extract b10-b24 */
    a   = a & (G729_Word16)0x7fff;
    
    i   = g729_sub(i, 16);
    
    L_y = g729_L_deposit_h(g729_tabsqr[i]);              /* g729_tabsqr[i] << 16                */
    tmp = g729_sub(g729_tabsqr[i], g729_tabsqr[i+1]);    /* g729_tabsqr[i] - g729_tabsqr[i+1])  */
    L_y = g729_L_msu(L_y, tmp, a);                       /* L_y -=  tmp*a*2                     */
    
    L_y = g729_L_shr(L_y, exp);                /* denormalization */
    
    return(L_y);
}


