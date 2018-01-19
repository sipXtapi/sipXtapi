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

/*___________________________________________________________________________
 |                                                                           |
 | Basics operators.                                                         |
 |___________________________________________________________________________|
 */

/*___________________________________________________________________________
 |                                                                           |
 |   Include-Files                                                           |
 |___________________________________________________________________________|
 */

#include <stdio.h>
#include <stdlib.h>
#include "g729a_typedef.h"
#include "basic_op.h"

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
 */

/* G729_Flag Carry =0; */

#if !defined(USE_GLOBAL_OVERFLOW_FLAG) || (USE_GLOBAL_OVERFLOW_FLAG != 1)
static
#endif
G729_Flag G729A_Overflow_Flag = 0;

/*___________________________________________________________________________
 |                                                                           |
 |   Local Functions                                                         |
 |___________________________________________________________________________|
 */

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_sature                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Limit the 32 bit input to the range of a 16 bit word.                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |        32 bit long signed integer (G729_Word32) whose value falls in the  |
 |        range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                      |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |        16 bit short signed integer (G729_Word16) whose value falls in the |
 |        range : 0xffff 8000 <= var_out <= 0x0000 7fff.                     |
 |___________________________________________________________________________|
 */

static G729_Word16 g729_sature(G729_Word32 L_var1)
{
    G729_Word16 var_out;
    
    if (L_var1 > 0X00007fffL)
    {
        G729A_Overflow_Flag = 1;
        var_out = G729A_MAX_16;
    }
    else if (L_var1 < (G729_Word32)0xffff8000L)
    {
        G729A_Overflow_Flag = 1;
        var_out = G729A_MIN_16;
    }
    else
    {
        G729A_Overflow_Flag = 0;
        var_out = g729_extract_l(L_var1);
    }
    
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Functions                                                               |
 |___________________________________________________________________________|
 */

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_abs_s                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Absolute value of var1; g729_abs_s(-32768) = 32767.                    |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0x0000 0000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_abs_s(G729_Word16 var1)
{
    G729_Word16 var_out;
    
    if (var1 == (G729_Word16)0x8000 )
    {
        var_out = G729A_MAX_16;
    }
    else
    {
        if (var1 < 0)
        {
            var_out = -var1;
        }
        else
        {
            var_out = var1;
        }
    }
    return(var_out);
}

/*_____________________________________________________________________________
 |                                                                             |
 |   Function Name : g729_negate                                               |
 |                                                                             |
 |   Purpose :                                                                 |
 |                                                                             |
 |   Negate var1 with saturation, saturate in the case where input is -32768:  |
 |                g729_negate(var1) = g729_sub(0,var1).                        |
 |                                                                             |
 |   Complexity weight : 1                                                     |
 |                                                                             |
 |   Inputs :                                                                  |
 |                                                                             |
 |    var1                                                                     |
 |             16 bit short signed integer (G729_Word16) whose value falls     |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.              |
 |                                                                             |
 |   Outputs :                                                                 |
 |                                                                             |
 |    none                                                                     |
 |                                                                             |
 |   Return Value :                                                            |
 |                                                                             |
 |    var_out                                                                  |
 |             16 bit short signed integer (G729_Word16) whose value falls     |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.           |
 |_____________________________________________________________________________|
 */

G729_Word16 g729_negate(G729_Word16 var1)
{
    G729_Word16 var_out;
    
    var_out = (var1 == G729A_MIN_16) ? G729A_MAX_16 : -var1;
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_extract_h                                          |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Return the 16 MSB of L_var1.                                            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (G729_Word32 ) whose value falls   |
 |             in the range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.          |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_extract_h(G729_Word32 L_var1)
{
    G729_Word16 var_out;
    
    var_out = (G729_Word16) (L_var1 >> 16);
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_extract_l                                          |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Return the 16 LSB of L_var1.                                            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (G729_Word32 ) whose value falls   |
 |             in the range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.          |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_extract_l(G729_Word32 L_var1)
{
    G729_Word16 var_out;
    
    var_out = (G729_Word16) L_var1;
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_abs                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Absolute value of L_var1; Saturate in case where the input is          |
 |                                                               -214783648  |
 |                                                                           |
 |   Complexity weight : 3                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= var1 <= 0x7fff ffff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x0000 0000 <= var_out <= 0x7fff ffff.         |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_abs(G729_Word32 L_var1)
{
    G729_Word32 L_var_out;
    
    if (L_var1 == G729A_MIN_32)
    {
        L_var_out = G729A_MAX_32;
    }
    else
    {
        if (L_var1 < 0)
        {
            L_var_out = -L_var1;
        }
        else
        {
            L_var_out = L_var1;
        }
    }
    
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_negate                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Negate the 32 bit variable L_var1 with saturation; saturate in the case |
 |   where input is -2147483648 (0x8000 0000).                               |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (G729_Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_negate(G729_Word32 L_var1)
{
    G729_Word32 L_var_out;
    
    L_var_out = (L_var1 == G729A_MIN_32) ? G729A_MAX_32 : -L_var1;
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_deposit_h                                        |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Deposit the 16 bit var1 into the 16 MS bits of the 32 bit output. The   |
 |   16 LS bits of the output are zeroed.                                    |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= var_out <= 0x7fff 0000.         |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_deposit_h(G729_Word16 var1)
{
    G729_Word32 L_var_out;
    
    L_var_out = (G729_Word32) var1 << 16;
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_deposit_l                                        |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Deposit the 16 bit var1 into the 16 LS bits of the 32 bit output. The   |
 |   16 MS bits of the output are sign extended.                             |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0xFFFF 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_deposit_l(G729_Word16 var1)
{
    G729_Word32 L_var_out;
    
    L_var_out = (G729_Word32) var1;
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_norm_s                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shift needed to normalize the 16 bit varia- |
 |   ble var1 for positive values on the interval with minimum of 16384 and  |
 |   maximum of 32767, and for negative values on the interval with minimum  |
 |   of -32768 and maximum of -16384; in order to normalize the result, the  |
 |   following operation must be done :                                      |
 |                    norm_var1 = g729_shl(var1,g729_norm_s(var1)).          |
 |                                                                           |
 |   Complexity weight : 15                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0x0000 0000 <= var_out <= 0x0000 000f.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_norm_s(G729_Word16 var1)
{
    G729_Word16 var_out;
    
    if (var1 == 0)
    {
        var_out = 0;
    }
    else
    {
        if (var1 == (G729_Word16) 0xffff)
        {
            var_out = 15;
        }
        else
        {
            if (var1 < 0)
            {
                var1 = ~var1;
            }
            
            for(var_out = 0; var1 < 0x4000; var_out++)
            {
                var1 <<= 1;
            }
        }
    }
    
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_norm_l                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shift needed to normalize the 32 bit varia- |
 |   ble l_var1 for positive values on the interval with minimum of          |
 |   1073741824 and maximum of 2147483647, and for negative values on the in-|
 |   terval with minimum of -2147483648 and maximum of -1073741824; in order |
 |   to normalize the result, the following operation must be done :         |
 |          norm_L_var1 = g729_L_shl(L_var1,g729_norm_l(L_var1)).            |
 |                                                                           |
 |   Complexity weight : 30                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= var1 <= 0x7fff ffff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0x0000 0000 <= var_out <= 0x0000 001f.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_norm_l(G729_Word32 L_var1)
{
    G729_Word16 var_out;
    
    if (L_var1 == 0)
    {
        var_out = 0;
    }
    else
    {
        if (L_var1 == (G729_Word32)0xffffffffL)
        {
            var_out = 31;
        }
        else
        {
            if (L_var1 < 0)
            {
                L_var1 = ~L_var1;
            }
            
            for(var_out = 0;L_var1 < (G729_Word32)0x40000000L;var_out++)
            {
                L_var1 <<= 1;
            }
        }
    }
    
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_add                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the addition (var1+var2) with overflow control and saturation;|
 |    the 16 bit result is set at +32767 when overflow occurs or at -32768   |
 |    when underflow occurs.                                                 |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_add(G729_Word16 var1, G729_Word16 var2)
{
    G729_Word16 var_out;
    G729_Word32 L_somme;
    
    L_somme = (G729_Word32) var1 + var2;
    var_out = g729_sature(L_somme);
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_sub                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the subtraction (var1+var2) with overflow control and satu-   |
 |    ration; the 16 bit result is set at +32767 when overflow occurs or at  |
 |    -32768 when underflow occurs.                                          |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_sub(G729_Word16 var1, G729_Word16 var2)
{
    G729_Word16 var_out;
    G729_Word32 L_diff;
    
    L_diff = (G729_Word32) var1 - var2;
    var_out = g729_sature(L_diff);
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_shl                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 16 bit input var1 left var2 positions.Zero fill|
 |   the var2 LSB of the result. If var2 is negative, arithmetically shift   |
 |   var1 right by -var2 with sign extension. Saturate the result in case of |
 |   underflows or overflows.                                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_shl(G729_Word16 var1, G729_Word16 var2)
{
    G729_Word16 var_out;
    G729_Word32 resultat;
    
    if (var2 < 0)
    {
        var_out = g729_shr(var1, -var2);
    }
    else
    {
        resultat = (G729_Word32) var1 * ((G729_Word32) 1 << var2);
        if ((var2 > 15 && var1 != 0) || (resultat != (G729_Word32)((G729_Word16) resultat)))
        {
            G729A_Overflow_Flag = 1;
            var_out = (var1 > 0) ? G729A_MAX_16 : G729A_MIN_16;
        }
        else
        {
            var_out = g729_extract_l(resultat);
        }
    }
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_shr                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 16 bit input var1 right var2 positions with    |
 |   sign extension. If var2 is negative, arithmetically shift var1 left by  |
 |   -var2 with sign extension. Saturate the result in case of underflows or |
 |   overflows.                                                              |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_shr(G729_Word16 var1, G729_Word16 var2)
{
    G729_Word16 var_out;
    
    if (var2 < 0)
    {
        var_out = g729_shl(var1, -var2);
    }
    else
    {
        if (var2 >= 15)
        {
            var_out = (var1 < 0) ? (G729_Word16)(-1) : (G729_Word16)0;
        }
        else
        {
            if (var1 < 0)
            {
                var_out = ~(( ~var1) >> var2 );
            }
            else
            {
                var_out = var1 >> var2;
            }
        }
    }
    
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_mult                                               |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the multiplication of var1 by var2 and gives a 16 bit result  |
 |    which is scaled i.e.:                                                  |
 |         g729_mult(var1,var2) = g729_shr((var1 times var2),15) and         |
 |         g729_mult(-32768,-32768) = 32767.                                 |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_mult(G729_Word16 var1, G729_Word16 var2)
{
    G729_Word16 var_out;
    G729_Word32 L_produit;
    
    L_produit = (G729_Word32)var1 * (G729_Word32)var2;
    
    L_produit = (L_produit & (G729_Word32) 0xffff8000L) >> 15;
    
    if (L_produit & (G729_Word32) 0x00010000L)
    {
        L_produit = L_produit | (G729_Word32) 0xffff0000L;
    }
    
    var_out = g729_sature(L_produit);
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_add                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   32 bits addition of the two 32 bits variables (L_var1+L_var2) with      |
 |   overflow control and saturation; the result is set at +214783647 when   |
 |   overflow occurs or at -214783648 when underflow occurs.                 |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |    L_var2   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.       |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_add(G729_Word32 L_var1, G729_Word32 L_var2)
{
    G729_Word32 L_var_out;
    
    L_var_out = L_var1 + L_var2;
    
    if (((L_var1 ^ L_var2) & G729A_MIN_32) == 0)
    {
        if ((L_var_out ^ L_var1) & G729A_MIN_32)
        {
            L_var_out = (L_var1 < 0) ? G729A_MIN_32 : G729A_MAX_32;
            G729A_Overflow_Flag = 1;
        }
    }
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_sub                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   32 bits subtraction of the two 32 bits variables (L_var1-L_var2) with   |
 |   overflow control and saturation; the result is set at +214783647 when   |
 |   overflow occurs or at -214783648 when underflow occurs.                 |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |    L_var2   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.       |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_sub(G729_Word32 L_var1, G729_Word32 L_var2)
{
    G729_Word32 L_var_out;
    
    L_var_out = L_var1 - L_var2;
    
    if (((L_var1 ^ L_var2) & G729A_MIN_32) != 0)
    {
        if ((L_var_out ^ L_var1) & G729A_MIN_32)
        {
            L_var_out = (L_var1 < 0L) ? G729A_MIN_32 : G729A_MAX_32;
            G729A_Overflow_Flag = 1;
        }
    }
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_shl                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 32 bit input L_var1 left var2 positions. Zero  |
 |   fill the var2 LSB of the result. If var2 is negative, L_var1 right by   |
 |   -var2 arithmetically shift with sign extension. Saturate the result in  |
 |   case of underflows or overflows.                                        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.       |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_shl(G729_Word32 L_var1, G729_Word16 var2)
{
    G729_Word32 L_var_out;
    
    /* initialization used only to suppress Microsoft Visual C++ warnings */
    L_var_out = 0;
    
    if (var2 <= 0)
    {
        L_var_out = g729_L_shr(L_var1, -var2);
    }
    else
    {
        for(;var2>0;var2--)
        {
            if (L_var1 > (G729_Word32) 0X3fffffffL)
            {
                G729A_Overflow_Flag = 1;
                L_var_out = G729A_MAX_32;
                break;
            }
            else
            {
                if (L_var1 < (G729_Word32) 0xc0000000L)
                {
                    G729A_Overflow_Flag = 1;
                    L_var_out = G729A_MIN_32;
                    break;
                }
            }
            L_var1 *= 2;
            L_var_out = L_var1;
        }
    }
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_shr                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 32 bit input L_var1 right var2 positions with  |
 |   sign extension. If var2 is negative, arithmetically shift L_var1 left   |
 |   by -var2 and zero fill the var2 LSB of the result. Saturate the result  |
 |   in case of underflows or overflows.                                     |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.       |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_shr(G729_Word32 L_var1, G729_Word16 var2)
{
    G729_Word32 L_var_out;
    
    if (var2 < 0)
    {
        L_var_out = g729_L_shl(L_var1, -var2);
    }
    else
    {
        if (var2 >= 31)
        {
            L_var_out = (L_var1 < 0L) ? -1 : 0;
        }
        else
        {
            if (L_var1<0)
            {
                L_var_out = ~((~L_var1) >> var2);
            }
            else
            {
                L_var_out = L_var1 >> var2;
            }
        }
    }
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_mult                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   g729_L_mult is the 32 bit result of the multiplication of var1          |
 |   times var2 with one shift left i.e.:                                    |
 |        g729_L_mult(var1,var2) = g729_shl((var1 times var2),1) and         |
 |        g729_L_mult(-32768,-32768) = 2147483647.                           |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |        16 bit short signed integer (G729_Word16) whose value falls in the |
 |        range : 0xffff 8000 <= var1 <= 0x0000 7fff.                        |
 |                                                                           |
 |    var2                                                                   |
 |        16 bit short signed integer (G729_Word16) whose value falls in the |
 |        range : 0xffff 8000 <= var1 <= 0x0000 7fff.                        |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |        32 bit long signed integer (G729_Word32) whose value falls in the  |
 |        range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.                   |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_mult(G729_Word16 var1, G729_Word16 var2)
{
    G729_Word32 L_var_out;
    
    L_var_out = (G729_Word32)var1 * (G729_Word32)var2;
    if (L_var_out != (G729_Word32)0x40000000L)
    {
        L_var_out *= 2;
    }
    else
    {
        G729A_Overflow_Flag = 1;
        L_var_out = G729A_MAX_32;
    }
    
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_round                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Round the lower 16 bits of the 32 bit input number into its MS 16 bits  |
 |   with saturation. Shift the resulting bits right by 16 and return the 16 |
 |   bit number:                                                             |
 |         g729_round(L_var1) = g729_extract_h(g729_L_add(L_var1,32768))     |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (G729_Word32 ) whose value falls   |
 |             in the range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.          |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_round(G729_Word32 L_var1)
{
    G729_Word16 var_out;
    G729_Word32 L_arrondi;
    
    L_arrondi = g729_L_add(L_var1, (G729_Word32)0x00008000);
    var_out = g729_extract_h(L_arrondi);
    return(var_out);
}

/*______________________________________________________________________________________
 |                                                                                      |
 |   Function Name : g729_L_mac                                                         |
 |                                                                                      |
 |   Purpose :                                                                          |
 |                                                                                      |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit               |
 |   result to L_var3 with saturation, return a 32 bit result:                          |
 |        g729_L_mac(L_var3,var1,var2) = g729_L_add(L_var3,(g729_L_mult(var1,var2)).    |
 |                                                                                      |
 |   Complexity weight : 1                                                              |
 |                                                                                      |
 |   Inputs :                                                                           |
 |                                                                                      |
 |    L_var3   32 bit long signed integer (G729_Word32) whose value falls in the        |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                            |
 |                                                                                      |
 |    var1                                                                              |
 |             16 bit short signed integer (G729_Word16) whose value falls in the       |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                              |
 |                                                                                      |
 |    var2                                                                              |
 |             16 bit short signed integer (G729_Word16) whose value falls in the       |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                              |
 |                                                                                      |
 |   Outputs :                                                                          |
 |                                                                                      |
 |    none                                                                              |
 |                                                                                      |
 |   Return Value :                                                                     |
 |                                                                                      |
 |    L_var_out                                                                         |
 |             32 bit long signed integer (G729_Word32) whose value falls in the        |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.                         |
 |______________________________________________________________________________________|
 */

G729_Word32 g729_L_mac(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2)
{
    G729_Word32 L_var_out;
    G729_Word32 L_produit;
    
    L_produit = g729_L_mult(var1, var2);
    L_var_out = g729_L_add(L_var3, L_produit);
    return(L_var_out);
}


/*______________________________________________________________________________________
 |                                                                                      |
 |   Function Name : g729_L_msu                                                         |
 |                                                                                      |
 |   Purpose :                                                                          |
 |                                                                                      |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32              |
 |   bit result to L_var3 with saturation, return a 32 bit result:                      |
 |        g729_L_msu(L_var3,var1,var2) = g729_L_sub(L_var3,(g729_L_mult(var1,var2)).    |
 |                                                                                      |
 |   Complexity weight : 1                                                              |
 |                                                                                      |
 |   Inputs :                                                                           |
 |                                                                                      |
 |    L_var3   32 bit long signed integer (G729_Word32) whose value falls in the        |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                            |
 |                                                                                      |
 |    var1                                                                              |
 |             16 bit short signed integer (G729_Word16) whose value falls in the       |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                              |
 |                                                                                      |
 |    var2                                                                              |
 |             16 bit short signed integer (G729_Word16) whose value falls in the       |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                              |
 |                                                                                      |
 |   Outputs :                                                                          |
 |                                                                                      |
 |    none                                                                              |
 |                                                                                      |
 |   Return Value :                                                                     |
 |                                                                                      |
 |    L_var_out                                                                         |
 |             32 bit long signed integer (G729_Word32) whose value falls in the        |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.                         |
 |______________________________________________________________________________________|
 */

G729_Word32 g729_L_msu(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2)
{
    G729_Word32 L_var_out;
    G729_Word32 L_produit;
    
    L_produit = g729_L_mult(var1, var2);
    L_var_out = g729_L_sub(L_var3, L_produit);
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_mult_r                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as g729_mult with rounding, i.e.:                                  |
 |     g729_mult_r(var1,var2) = g729_shr(((var1*var2) + 16384),15) and       |
 |     g729_mult_r(-32768,-32768) = 32767.                                   |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_mult_r(G729_Word16 var1, G729_Word16 var2)
{
    G729_Word16 var_out;
    G729_Word32 L_produit_arr;
    
    L_produit_arr = (G729_Word32)var1 * (G729_Word32)var2; /* product */
    L_produit_arr += (G729_Word32) 0x00004000;        /* round */
    L_produit_arr &= (G729_Word32) 0xffff8000L;
    L_produit_arr >>= 15;                        /* shift */
    
    if (L_produit_arr & (G729_Word32) 0x00010000L)   /* sign extend when necessary */
    {
        L_produit_arr |= (G729_Word32) 0xffff0000L;
    }
    
    var_out = g729_sature(L_produit_arr);
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_shr_r                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as g729_shr(var1,var2) but with rounding. Saturate the result      |
 |   in case of underflows or overflows :                                    |
 |    If var2 is greater than zero :                                         |
 |       g729_shr_r(var1,var2) = g729_shr(g729_add(var1,2**(var2-1)),var2)   |
 |    If var2 is less than zero :                                            |
 |       g729_shr_r(var1,var2) = g729_shr(var1,var2).                        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var_out <= 0x0000 7fff.         |
 |___________________________________________________________________________|
 */

G729_Word16 g729_shr_r(G729_Word16 var1, G729_Word16 var2)
{
    G729_Word16 var_out;
    
    if (var2>15)
    {
        var_out = 0;
    }
    else
    {
        var_out = g729_shr(var1, var2);
        
        if (var2 > 0)
        {
            if ((var1 & ((G729_Word16)1 << (var2-1))) != 0)
            {
                var_out++;
            }
        }
    }
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_mac_r                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
 |   result to L_var3 with saturation. Round the LS 16 bits of the result    |
 |   into the MS 16 bits with saturation and shift the result right by 16.   |
 |   Return a 16 bit result.                                                 |
 |        g729_mac_r(L_var3,var1,var2) = round(g729_L_mac(Lvar3,var1,var2))  |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0x0000 8000 <= L_var_out <= 0x0000 7fff.       |
 |___________________________________________________________________________|
 */

G729_Word16 g729_mac_r(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2)
{
    G729_Word16 var_out;
    
    L_var3 = g729_L_mac(L_var3, var1, var2);
    L_var3 = g729_L_add(L_var3, (G729_Word32)0x00008000);
    var_out = g729_extract_h(L_var3);
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_msu_r                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
 |   bit result to L_var3 with saturation. Round the LS 16 bits of the res-  |
 |   ult into the MS 16 bits with saturation and shift the result right by   |
 |   16. Return a 16 bit result.                                             |
 |        g729_msu_r(L_var3,var1,var2) = round(g729_L_msu(Lvar3,var1,var2))  |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0x0000 8000 <= L_var_out <= 0x0000 7fff.       |
 |___________________________________________________________________________|
 */

G729_Word16 g729_msu_r(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2)
{
    G729_Word16 var_out;
    
    L_var3 = g729_L_msu(L_var3, var1, var2);
    L_var3 = g729_L_add(L_var3, (G729_Word32)0x00008000);
    var_out = g729_extract_h(L_var3);
    return(var_out);
}

/*__________________________________________________________________________________
 |                                                                                  |
 |   Function Name : g729_L_shr_r                                                   |
 |                                                                                  |
 |   Purpose :                                                                      |
 |                                                                                  |
 |   Same as g729_L_shr(L_var1,var2)but with rounding. Saturate the result          |
 |   in case of underflows or overflows :                                           |
 |    If var2 is greater than zero :                                                |
 |       g729_L_shr_r(var1,var2) = g729_L_shr(g729_L_add(L_var1,2**(var2-1)),var2)  |
 |    If var2 is less than zero :                                                   |
 |       g729_L_shr_r(var1,var2) = g729_L_shr(L_var1,var2).                         |
 |                                                                                  |
 |   Complexity weight : 3                                                          |
 |                                                                                  |
 |   Inputs :                                                                       |
 |                                                                                  |
 |    L_var1                                                                        |
 |             32 bit long signed integer (G729_Word32) whose value falls           |
 |             in the range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                                  |
 |    var2                                                                          |
 |             16 bit short signed integer (G729_Word16) whose value falls          |
 |             in the range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                                  |
 |   Outputs :                                                                      |
 |                                                                                  |
 |    none                                                                          |
 |                                                                                  |
 |   Return Value :                                                                 |
 |                                                                                  |
 |    L_var_out                                                                     |
 |             32 bit long signed integer (G729_Word32) whose value falls           |
 |             in the range : 0x8000 0000 <= var_out <= 0x7fff ffff.                |
 |__________________________________________________________________________________|
 */

G729_Word32 g729_L_shr_r(G729_Word32 L_var1,G729_Word16 var2)
{
    G729_Word32 L_var_out;
    
    if (var2 > 31)
    {
        L_var_out = 0;
    }
    else
    {
        L_var_out = g729_L_shr(L_var1, var2);
        if (var2 > 0)
        {
            if ( (L_var1 & ( (G729_Word32)1 << (var2-1) )) != 0)
            {
                L_var_out++;
            }
        }
    }
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_div_s                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces a result which is the fractional  integer division of var1 by  |
 |   var2; var1 and var2 must be positive and var2 must be greater or equal  |
 |   to var1; the result is positive (leading bit equal to 0) and truncated  |
 |   to 16 bits.                                                             |
 |   If var1 = var2 then div(var1,var2) = 32767.                             |
 |                                                                           |
 |   Complexity weight : 18                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0x0000 0000 <= var1 <= var2 and var2 != 0.     |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : var1 <= var2 <= 0x0000 7fff and var2 != 0.     |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (G729_Word16) whose value falls   |
 |             in the range : 0x0000 0000 <= var_out <= 0x0000 7fff.         |
 |             It's a Q15 value (point between b15 and b14).                 |
 |___________________________________________________________________________|
 */

G729_Word16 g729_div_s(G729_Word16 var1, G729_Word16 var2)
{
    G729_Word16 var_out = 0;
    G729_Word16 iteration;
    G729_Word32 L_num;
    G729_Word32 L_denom;
    
    if ((var1 > var2) || (var1 < 0) || (var2 < 0))
    {
        printf("Division Error var1=%d  var2=%d\n",var1,var2);
        exit(0);
    }
    
    if (var2 == 0)
    {
        printf("Division by 0, Fatal error \n");
        exit(0);
    }
    
    if (var1 == 0)
    {
        var_out = 0;
    }
    else
    {
        if (var1 == var2)
        {
            var_out = G729A_MAX_16;
        }
        else
        {
            L_num = g729_L_deposit_l(var1);
            L_denom = g729_L_deposit_l(var2);
            
            for(iteration=0;iteration<15;iteration++)
            {
                var_out <<=1;
                L_num <<= 1;
                
                if (L_num >= L_denom)
                {
                    L_num = g729_L_sub(L_num, L_denom);
                    var_out = g729_add(var_out, 1);
                }
            }
        }
    }
    
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   No use functions                                                        |
 |___________________________________________________________________________|
 */

#if 0

/*__________________________________________________________________________________________
 |                                                                                          |
 |   Function Name : g729_L_macNs                                                           |
 |                                                                                          |
 |   Purpose :                                                                              |
 |                                                                                          |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit                   |
 |   result to L_var3 without saturation, return a 32 bit result. Generate                  |
 |   carry and overflow values :                                                            |
 |        g729_L_macNs(L_var3,var1,var2) = g729_L_add_c(L_var3,(g729_L_mult(var1,var2)).    |
 |                                                                                          |
 |   Complexity weight : 1                                                                  |
 |                                                                                          |
 |   Inputs :                                                                               |
 |                                                                                          |
 |    L_var3   32 bit long signed integer (G729_Word32) whose value falls in the            |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                                |
 |                                                                                          |
 |    var1                                                                                  |
 |             16 bit short signed integer (G729_Word16) whose value falls in the           |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                                  |
 |                                                                                          |
 |    var2                                                                                  |
 |             16 bit short signed integer (G729_Word16) whose value falls in the           |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                                  |
 |                                                                                          |
 |   Outputs :                                                                              |
 |                                                                                          |
 |    none                                                                                  |
 |                                                                                          |
 |   Return Value :                                                                         |
 |                                                                                          |
 |    L_var_out                                                                             |
 |             32 bit long signed integer (G729_Word32) whose value falls in the            |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.                             |
 |                                                                                          |
 |   Caution :                                                                              |
 |                                                                                          |
 |    In some cases the Carry flag has to be cleared or set before using op-                |
 |    rators which take into account its value.                                             |
 |__________________________________________________________________________________________|
 */

G729_Word32 g729_L_macNs(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2)
{
    G729_Word32 L_var_out;
    
    L_var_out = g729_L_mult(var1, var2);
    L_var_out = g729_L_add_c(L_var3,L_var_out);
    return(L_var_out);
}

/*__________________________________________________________________________________________
 |                                                                                          |
 |   Function Name : g729_L_msuNs                                                           |
 |                                                                                          |
 |   Purpose :                                                                              |
 |                                                                                          |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32                  |
 |   bit result from L_var3 without saturation, return a 32 bit result. Ge-                 |
 |   nerate carry and overflow values :                                                     |
 |        g729_L_msuNs(L_var3,var1,var2) = g729_L_sub_c(L_var3,(g729_L_mult(var1,var2)).    |
 |                                                                                          |
 |   Complexity weight : 1                                                                  |
 |                                                                                          |
 |   Inputs :                                                                               |
 |                                                                                          |
 |    L_var3   32 bit long signed integer (G729_Word32) whose value falls in the            |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                                |
 |                                                                                          |
 |    var1                                                                                  |
 |             16 bit short signed integer (G729_Word16) whose value falls in the           |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                                  |
 |                                                                                          |
 |    var2                                                                                  |
 |             16 bit short signed integer (G729_Word16) whose value falls in the           |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                                  |
 |                                                                                          |
 |   Outputs :                                                                              |
 |                                                                                          |
 |    none                                                                                  |
 |                                                                                          |
 |   Return Value :                                                                         |
 |                                                                                          |
 |    L_var_out                                                                             |
 |             32 bit long signed integer (G729_Word32) whose value falls in the            |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.                             |
 |                                                                                          |
 |   Caution :                                                                              |
 |                                                                                          |
 |    In some cases the Carry flag has to be cleared or set before using op-                |
 |    rators which take into account its value.                                             |
 |__________________________________________________________________________________________|
 */

G729_Word32 g729_L_msuNs(G729_Word32 L_var3, G729_Word16 var1, G729_Word16 var2)
{
    G729_Word32 L_var_out;
    
    L_var_out = g729_L_mult(var1,var2);
    L_var_out = g729_L_sub_c(L_var3,L_var_out);
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_sat                                              |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    32 bit L_var1 is set to 2147833647 if an overflow occurred or to       |
 |    -214783648 if an underflow occurred on the most recent g729_L_add_c,   |
 |    g729_L_sub_c, g729_L_macNs or LmsuNs operations. The carry and         |
 |    overflow values are binary values which can be tested and              |
 |    assigned values.                                                       |
 |                                                                           |
 |   Complexity weight : 4                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= var1 <= 0x7fff ffff.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= var_out <= 0x7fff ffff.         |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_sat (G729_Word32 L_var1)
{
    G729_Word32 L_var_out;
    
    L_var_out = L_var1;
    
    if (Overflow)
    {
        if (Carry)
        {
            L_var_out = G729A_MIN_32;
        }
        else
        {
            L_var_out = G729A_MAX_32;
        }
        
        Carry = 0;
        Overflow = 0;
    }
    
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_add_c                                            |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Performs 32 bits addition of the two 32 bits variables (L_var1+L_var2+C)|
 |   with carry. No saturation. Generate carry and Overflow values. The car- |
 |   ry and overflow values are binary variables which can be tested and as- |
 |   signed values.                                                          |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |    L_var2   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.       |
 |                                                                           |
 |   Caution :                                                               |
 |                                                                           |
 |    In some cases the Carry flag has to be cleared or set before using op- |
 |    rators which take into account its value.                              |
 |___________________________________________________________________________|
 */
G729_Word32 g729_L_add_c(G729_Word32 L_var1, G729_Word32 L_var2)
{
    G729_Word32 L_var_out;
    G729_Word32 L_test;
    G729_Flag carry_int = 0;
    
    L_var_out = L_var1 + L_var2 + Carry;
    
    L_test = L_var1 + L_var2;
    
    if ((L_var1>0) && (L_var2 >0) && (L_test < 0))
    {
        Overflow = 1;
        carry_int = 0;
    }
    else
    {
        if ((L_var1<0) && (L_var2 <0) && (L_test >0))
        {
            Overflow = 1;
            carry_int = 1;
        }
        else
        {
            if (((L_var1 ^ L_var2) < 0) && (L_test > 0))
            {
                Overflow = 0;
                carry_int = 1;
            }
            else
            {
                Overflow = 0;
                carry_int = 0;
            }
        }
    }
    
    if (Carry)
    {
        if (L_test == G729A_MAX_32)
        {
            Overflow = 1;
            Carry = carry_int;
        }
        else
        {
            if (L_test == (G729_Word32) 0xFFFFFFFFL)
            {
                Carry = 1;
            }
            else
            {
                Carry = carry_int;
            }
        }
    }
    else
    {
        Carry = carry_int;
    }
    
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g729_L_sub_c                                            |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Performs 32 bits subtraction of the two 32 bits variables with carry    |
 |   (borrow) : L_var1-L_var2-C. No saturation. Generate carry and Overflow  |
 |   values. The carry and overflow values are binary variables which can    |
 |   be tested and assigned values.                                          |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |    L_var2   32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.          |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (G729_Word32) whose value falls    |
 |             in the range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.       |
 |                                                                           |
 |   Caution :                                                               |
 |                                                                           |
 |    In some cases the Carry flag has to be cleared or set before using op- |
 |    rators which take into account its value.                              |
 |___________________________________________________________________________|
 */

G729_Word32 g729_L_sub_c(G729_Word32 L_var1, G729_Word32 L_var2)
{
    G729_Word32 L_var_out;
    G729_Word32 L_test;
    G729_Flag carry_int = 0;
    
    if (Carry)
    {
        Carry = 0;
        if (L_var2 != G729A_MIN_32)
        {
            L_var_out = g729_L_add_c(L_var1,-L_var2);
        }
        else
        {
            L_var_out = L_var1 - L_var2;
            if (L_var1 > 0L)
            {
                Overflow = 1;
                Carry = 0;
            }
        }
    }
    else
    {
        L_var_out = L_var1 - L_var2 - (G729_Word32)0X00000001;
        L_test = L_var1 - L_var2;
        
        if ((L_test < 0) && (L_var1 > 0) && (L_var2 < 0))
        {
            Overflow = 1;
            carry_int = 0;
        }
        else if ((L_test > 0) && (L_var1 < 0) && (L_var2 > 0))
        {
            Overflow = 1;
            carry_int = 1;
        }
        else if ((L_test > 0) && ((L_var1 ^ L_var2) > 0))
        {
            Overflow = 0;
            carry_int = 1;
        }
        
        
        if (L_test == G729A_MIN_32)
        {
            Overflow = 1;
            Carry = carry_int;
        }
        else
        {
            Carry = carry_int;
        }
    }
    
    return(L_var_out);
}
#endif  /* #if 0 */
/* end of file */
