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
#include "oper_32b.h"

/*___________________________________________________________________________
 |                                                                           |
 |  This file contains operations in double precision.                       |
 |  These operations are not standard double precision operations.           |
 |  They are used where single precision is not enough but the full 32 bits  |
 |  precision is not necessary. For example, the function g729_Div_32()      |
 |  has a 24 bits precision which is enough for our purposes.                |
 |                                                                           |
 |  The double precision numbers use a special representation:               |
 |                                                                           |
 |     L_32 = hi<<16 + lo<<1                                                 |
 |                                                                           |
 |  L_32 is a 32 bit integer.                                                |
 |  hi and lo are 16 bit signed integers.                                    |
 |  As the low part also contains the sign, this allows fast multiplication. |
 |                                                                           |
 |      0x8000 0000 <= L_32 <= 0x7fff fffe.                                  |
 |                                                                           |
 |  We will use DPF (Double Precision Format )in this file to specify        |
 |  this special format.                                                     |
 |___________________________________________________________________________|
 */

/*___________________________________________________________________________
 |                                                                           |
 |  Function g729_L_Extract()                                                |
 |                                                                           |
 |  Extract from a 32 bit integer two 16 bit DPF.                            |
 |                                                                           |
 |  Arguments:                                                               |
 |                                                                           |
 |   L_32      : 32 bit integer.                                             |
 |               0x8000 0000 <= L_32 <= 0x7fff ffff.                         |
 |   hi        : b16 to b31 of L_32                                          |
 |   lo        : (L_32 - hi<<16)>>1                                          |
 |___________________________________________________________________________|
 */

void g729_L_Extract(G729_Word32 L_32, G729_Word16 *hi, G729_Word16 *lo)
{
    *hi  = g729_extract_h(L_32);
    *lo  = g729_extract_l(g729_L_msu(g729_L_shr(L_32, 1) , *hi, 16384));  /* lo = L_32>>1   */
    return;
}

/*___________________________________________________________________________
 |                                                                           |
 |  Function g729_L_Comp()                                                   |
 |                                                                           |
 |  Compose from two 16 bit DPF a 32 bit integer.                            |
 |                                                                           |
 |     L_32 = hi<<16 + lo<<1                                                 |
 |                                                                           |
 |  Arguments:                                                               |
 |                                                                           |
 |   hi        msb                                                           |
 |   lo        lsf (with sign)                                               |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |        32 bit long signed integer (G729_Word32) whose value falls in the  |
 |        range : 0x8000 0000 <= L_32 <= 0x7fff fff0.                        |
 |                                                                           |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_Comp(G729_Word16 hi, G729_Word16 lo)
{
    G729_Word32 L_32;
    
    L_32 = g729_L_deposit_h(hi);
    return( g729_L_mac(L_32, lo, 1));          /* = hi<<16 + lo<<1 */
}

/*___________________________________________________________________________
 | Function g729_Mpy_32()                                                    |
 |                                                                           |
 |   Multiply two 32 bit integers (DPF). The result is divided by 2**31      |
 |                                                                           |
 |   L_32 = (hi1*hi2)<<1 + ( (hi1*lo2)>>15 + (lo1*hi2)>>15 )<<1              |
 |                                                                           |
 |   This operation can also be viewed as the multiplication of two Q31      |
 |   number and the result is also in Q31.                                   |
 |                                                                           |
 | Arguments:                                                                |
 |                                                                           |
 |  hi1         hi part of first number                                      |
 |  lo1         lo part of first number                                      |
 |  hi2         hi part of second number                                     |
 |  lo2         lo part of second number                                     |
 |                                                                           |
 |___________________________________________________________________________|
 */

G729_Word32 g729_Mpy_32(G729_Word16 hi1, G729_Word16 lo1, G729_Word16 hi2, G729_Word16 lo2)
{
    G729_Word32 L_32;
    
    L_32 = g729_L_mult(hi1, hi2);
    L_32 = g729_L_mac(L_32, g729_mult(hi1, lo2) , 1);
    L_32 = g729_L_mac(L_32, g729_mult(lo1, hi2) , 1);
    
    return( L_32 );
}

/*___________________________________________________________________________
 | Function g729_Mpy_32_16()                                                 |
 |                                                                           |
 |   Multiply a 16 bit integer by a 32 bit (DPF). The result is divided      |
 |   by 2**15                                                                |
 |                                                                           |
 |   This operation can also be viewed as the multiplication of a Q31        |
 |   number by a Q15 number, the result is in Q31.                           |
 |                                                                           |
 |   L_32 = (hi1*lo2)<<1 + ((lo1*lo2)>>15)<<1                                |
 |                                                                           |
 | Arguments:                                                                |
 |                                                                           |
 |  hi          hi part of 32 bit number.                                    |
 |  lo          lo part of 32 bit number.                                    |
 |  n           16 bit number.                                               |
 |                                                                           |
 |___________________________________________________________________________|
 */

G729_Word32 g729_Mpy_32_16(G729_Word16 hi, G729_Word16 lo, G729_Word16 n)
{
    G729_Word32 L_32;
    
    L_32 = g729_L_mult(hi, n);
    L_32 = g729_L_mac(L_32, g729_mult(lo, n) , 1);
    
    return( L_32 );
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_Div_32                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |             Fractional integer division of two 32 bit numbers.            |
 |             L_num / L_denom.                                              |
 |             L_num and L_denom must be positive and L_num < L_denom.       |
 |             L_denom = denom_hi<<16 + denom_lo<<1                          |
 |             denom_hi is a normalize number.                               |
 |             The result is in Q30.                                         |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_num                                                                  |
 |        32 bit long signed integer (G729_Word32) whose value falls in the  |
 |        range : 0x0000 0000 < L_num < L_denom                              |
 |                                                                           |
 |    L_denom = denom_hi<<16 + denom_lo<<1      (DPF)                        |
 |                                                                           |
 |       denom_hi                                                            |
 |             16 bit positive normalized integer whose value falls in the   |
 |             range : 0x4000 < hi < 0x7fff                                  |
 |       denom_lo                                                            |
 |             16 bit positive integer whose value falls in the              |
 |             range : 0 < lo < 0x7fff                                       |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_div                                                                  |
 |        32 bit long signed integer (G729_Word32) whose value falls in the  |
 |        range : 0x0000 0000 <= L_div <= 0x7fff ffff.                       |
 |        It's a Q31 value                                                   |
 |                                                                           |
 |  Algorithm:                                                               |
 |                                                                           |
 |  - find = 1/L_denom.                                                      |
 |      First approximation: approx = 1 / denom_hi                           |
 |      1/L_denom = approx * (2.0 - L_denom * approx )                       |
 |                                                                           |
 |  -  result = L_num * (1/L_denom)                                          |
 |___________________________________________________________________________|
 */

G729_Word32 g729_Div_32(G729_Word32 L_num, G729_Word16 denom_hi, G729_Word16 denom_lo)
{
    G729_Word16 approx, hi, lo, n_hi, n_lo;
    G729_Word32 L_32;
    
    
    /* First approximation: 1 / L_denom = 1/denom_hi */
    
    approx = g729_div_s( (G729_Word16)0x3fff, denom_hi);    /* result in Q14 */
    /* Note: 3fff = 0.5 in Q15 */
    
    /* 1/L_denom = approx * (2.0 - L_denom * approx) */
    
    L_32 = g729_Mpy_32_16(denom_hi, denom_lo, approx); /* result in Q30 */
    
    
    L_32 = g729_L_sub( (G729_Word32)0x7fffffffL, L_32);      /* result in Q30 */
    
    g729_L_Extract(L_32, &hi, &lo);
    
    L_32 = g729_Mpy_32_16(hi, lo, approx);             /* = 1/L_denom in Q29 */
    
    /* L_num * (1/L_denom) */
    
    g729_L_Extract(L_32, &hi, &lo);
    g729_L_Extract(L_num, &n_hi, &n_lo);
    L_32 = g729_Mpy_32(n_hi, n_lo, hi, lo);            /* result in Q29   */
    L_32 = g729_L_shl(L_32, 2);                        /* From Q29 to Q31 */
    
    return( L_32 );
}
/* end of file */
