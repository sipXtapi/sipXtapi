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

/**
 *  Portions of this file are derived from the following GPL notice:
 *
 *  g729a codec for iPhone and iPod Touch
 *  Copyright (C) 2009 Samuel <samuelv0304@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*---------------------------------------------------------------------------*
 * Pitch related functions                                                   *
 * ~~~~~~~~~~~~~~~~~~~~~~~                                                   *
 *---------------------------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "ld8a.h"
#include "tab_ld8a.h"

/*---------------------------------------------------------------------------*
 * Function  g729_Pitch_ol_fast                                              *
 * ~~~~~~~~~~~~~~~~~~~~~~~                                                   *
 * Compute the open loop pitch lag. (fast version)                           *
 *                                                                           *
 *---------------------------------------------------------------------------*/


G729_Word16 g729_Pitch_ol_fast(  /* output: open loop pitch lag                        */
    G729_Word16 signal[],        /* input : signal used to compute the open loop pitch */
                                 /*     signal[-pit_max] to signal[-1] should be known */
    G729_Word16  pit_max,        /* input : maximum pitch lag                          */
    G729_Word16  L_frame         /* input : length of frame to compute pitch           */
)
{
    G729_Word16  i, j;
    G729_Word16  max1, max2, max3;
    G729_Word16  max_h, max_l, ener_h, ener_l;
    G729_Word16  T1, T2, T3;
    G729_Word16  *p, *p1;
    G729_Word32  max, sum, L_temp;
    
    /* Scaled signal */
    
    G729_Word16 scaled_signal[L_FRAME+PIT_MAX];
    G729_Word16 *scal_sig;
    
    /* add '#if' to avoid warning(-Wunused-variable) */
#if !defined(USE_GLOBAL_OVERFLOW_FLAG) || (USE_GLOBAL_OVERFLOW_FLAG != 1)
    G729_Flag overflow;
#endif
    
    scal_sig = &scaled_signal[pit_max];
    
    /*--------------------------------------------------------*
     *  Verification for risk of overflow.                    *
     *--------------------------------------------------------*/
    
    sum = 0;
    
#if defined(USE_GLOBAL_OVERFLOW_FLAG) && (USE_GLOBAL_OVERFLOW_FLAG == 1)
    
    G729A_Overflow_Flag = 0;
    for(i= -pit_max; i< L_frame; i+=2)
        sum = g729_L_mac(sum, signal[i], signal[i]);
    
#else
    
    overflow = 0;
    for ( i = -pit_max; i < L_frame; i += 2 )
    {
        sum += (((G729_Word32)signal[i] * (G729_Word32)signal[i]) << 1);
        if ( sum < 0 )
        {
            overflow = 1;
            sum = G729A_MAX_32;
            break;
        }
    }
#endif
    
    /* Anyway, sum MUST be positive */
    
    /*--------------------------------------------------------*
     * Scaling of input signal.                               *
     *                                                        *
     *   if Overflow        -> scal_sig[i] = signal[i]>>3     *
     *   else if sum < 1^20 -> scal_sig[i] = signal[i]<<3     *
     *   else               -> scal_sig[i] = signal[i]        *
     *--------------------------------------------------------*/
    
#if defined(USE_GLOBAL_OVERFLOW_FLAG) && (USE_GLOBAL_OVERFLOW_FLAG == 1)
    if(G729A_Overflow_Flag == 1)
#else
    if(overflow == 1)
#endif
    {
        for(i=-pit_max; i<L_frame; i++)
        {
            scal_sig[i] = g729_shr(signal[i], 3);
        }
    }
    else
    {
        L_temp = g729_L_sub(sum, (G729_Word32)1048576L);
        if ( L_temp < (G729_Word32)0 )  /* if (sum < 2^20) */
        {
            for(i=-pit_max; i<L_frame; i++)
            {
                scal_sig[i] = g729_shl(signal[i], 3);
            }
        }
        else
        {
            for(i=-pit_max; i<L_frame; i++)
            {
                scal_sig[i] = signal[i];
            }
        }
    }

    /*--------------------------------------------------------------------*
     *  The pitch lag search is divided in three sections.                *
     *  Each section cannot have a pitch multiple.                        *
     *  We find a maximum for each section.                               *
     *  We compare the maxima of each section by favoring small lag.      *
     *                                                                    *
     *  First section:  lag delay = 20 to 39                              *
     *  Second section: lag delay = 40 to 79                              *
     *  Third section:  lag delay = 80 to 143                             *
     *--------------------------------------------------------------------*/
    
    /* First section */
    
    max = G729A_MIN_32;
    T1  = 20;    /* Only to remove warning from some compilers */
    for (i = 20; i < 40; i++) {
        p  = scal_sig;
        p1 = &scal_sig[-i];
        sum = 0;
        for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
            sum = g729_L_mac(sum, *p, *p1);
        L_temp = g729_L_sub(sum, max);
        if (L_temp > 0) { max = sum; T1 = i;   }
    }
    
    /* compute energy of maximum */
    
    sum = 1;                   /* to avoid division by zero */
    p = &scal_sig[-T1];
    for(i=0; i<L_frame; i+=2, p+=2)
        sum = g729_L_mac(sum, *p, *p);
    
    /* max1 = max/sqrt(energy)                  */
    /* This result will always be on 16 bits !! */
    
    sum = g729_Inv_sqrt(sum);            /* 1/sqrt(energy),    result in Q30 */
    g729_L_Extract(max, &max_h, &max_l);
    g729_L_Extract(sum, &ener_h, &ener_l);
    sum  = g729_Mpy_32(max_h, max_l, ener_h, ener_l);
    max1 = g729_extract_l(sum);
    
    /* Second section */
    
    max = G729A_MIN_32;
    T2  = 40;    /* Only to remove warning from some compilers */
    for (i = 40; i < 80; i++) {
        p  = scal_sig;
        p1 = &scal_sig[-i];
        sum = 0;
        for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
            sum = g729_L_mac(sum, *p, *p1);
        L_temp = g729_L_sub(sum, max);
        if (L_temp > 0) { max = sum; T2 = i;   }
    }
    
    /* compute energy of maximum */
    
    sum = 1;                   /* to avoid division by zero */
    p = &scal_sig[-T2];
    for(i=0; i<L_frame; i+=2, p+=2)
        sum = g729_L_mac(sum, *p, *p);
    
    /* max2 = max/sqrt(energy)                  */
    /* This result will always be on 16 bits !! */
    
    sum = g729_Inv_sqrt(sum);            /* 1/sqrt(energy),    result in Q30 */
    g729_L_Extract(max, &max_h, &max_l);
    g729_L_Extract(sum, &ener_h, &ener_l);
    sum  = g729_Mpy_32(max_h, max_l, ener_h, ener_l);
    max2 = g729_extract_l(sum);
    
    /* Third section */
    
    max = G729A_MIN_32;
    T3  = 80;    /* Only to remove warning from some compilers */
    for (i = 80; i < 143; i+=2) {
        p  = scal_sig;
        p1 = &scal_sig[-i];
        sum = 0;
        for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
            sum = g729_L_mac(sum, *p, *p1);
        L_temp = g729_L_sub(sum, max);
        if (L_temp > 0) { max = sum; T3 = i;   }
    }
    
    /* Test around max3 */
    
    i = T3;
    p  = scal_sig;
    p1 = &scal_sig[-(i+1)];
    sum = 0;
    for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
        sum = g729_L_mac(sum, *p, *p1);
    L_temp = g729_L_sub(sum, max);
    if (L_temp > 0) { max = sum; T3 = i+(G729_Word16)1;   }
    
    p  = scal_sig;
    p1 = &scal_sig[-(i-1)];
    sum = 0;
    for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
        sum = g729_L_mac(sum, *p, *p1);
    L_temp = g729_L_sub(sum, max);
    if (L_temp > 0) { max = sum; T3 = i-(G729_Word16)1;   }
    
    /* compute energy of maximum */
    
    sum = 1;                   /* to avoid division by zero */
    p = &scal_sig[-T3];
    for(i=0; i<L_frame; i+=2, p+=2)
        sum = g729_L_mac(sum, *p, *p);
    
    /* max1 = max/sqrt(energy)                  */
    /* This result will always be on 16 bits !! */
    
    sum = g729_Inv_sqrt(sum);            /* 1/sqrt(energy),    result in Q30 */
    g729_L_Extract(max, &max_h, &max_l);
    g729_L_Extract(sum, &ener_h, &ener_l);
    sum  = g729_Mpy_32(max_h, max_l, ener_h, ener_l);
    max3 = g729_extract_l(sum);
    
    /*-----------------------*
     * Test for multiple.    *
     *-----------------------*/
    
    /* if( abs(T2*2 - T3) < 5)  */
    /*    max2 += max3 * 0.25;  */
    
    i = g729_sub(g729_shl(T2,1), T3);
    j = g729_sub(g729_abs_s(i), 5);
    if(j < 0)
        max2 = g729_add(max2, g729_shr(max3, 2));
    
    /* if( abs(T2*3 - T3) < 7)  */
    /*    max2 += max3 * 0.25;  */
    
    i = g729_add(i, T2);
    j = g729_sub(g729_abs_s(i), 7);
    if(j < 0)
        max2 = g729_add(max2, g729_shr(max3, 2));
    
    /* if( abs(T1*2 - T2) < 5)  */
    /*    max1 += max2 * 0.20;  */
    
    i = g729_sub(g729_shl(T1,1), T2);
    j = g729_sub(g729_abs_s(i), 5);
    if(j < 0)
        max1 = g729_add(max1, g729_mult(max2, 6554));
    
    /* if( abs(T1*3 - T2) < 7)  */
    /*    max1 += max2 * 0.20;  */
    
    i = g729_add(i, T1);
    j = g729_sub(g729_abs_s(i), 7);
    if(j < 0)
        max1 = g729_add(max1, g729_mult(max2, 6554));
    
    /*--------------------------------------------------------------------*
     * Compare the 3 sections maxima.                                     *
     *--------------------------------------------------------------------*/
    
    if( g729_sub(max1, max2) < 0 ) {max1 = max2; T1 = T2;  }
    if( g729_sub(max1, max3) <0 )  {T1 = T3; }
    
    return T1;
}




/*--------------------------------------------------------------------------*
 *  Function  Dot_Product()                                                 *
 *  ~~~~~~~~~~~~~~~~~~~~~~                                                  *
 *--------------------------------------------------------------------------*/

G729_Word32 Dot_Product(      /* (o)   :Result of scalar product. */
                        G729_Word16   x[],     /* (i)   :First vector.             */
                        G729_Word16   y[],     /* (i)   :Second vector.            */
                        G729_Word16   lg       /* (i)   :Number of point.          */
)
{
    G729_Word16 i;
    G729_Word32 sum;
    
    sum = 0;
    for(i=0; i<lg; i++)
        sum = g729_L_mac(sum, x[i], y[i]);
    
    return sum;
}

/*--------------------------------------------------------------------------*
 *  Function  g729_Pitch_fr3_fast()                                         *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~                                              *
 * Fast version of the pitch close loop.                                    *
 *--------------------------------------------------------------------------*/

G729_Word16 g729_Pitch_fr3_fast(    /* (o)     : pitch period.                          */
    G729_Word16 exc[],              /* (i)     : excitation buffer                      */
    G729_Word16 xn[],               /* (i)     : target vector                          */
    G729_Word16 h[],                /* (i) Q12 : impulse response of filters.           */
    G729_Word16 L_subfr,            /* (i)     : Length of subframe                     */
    G729_Word16 t0_min,             /* (i)     : minimum value in the searched range.   */
    G729_Word16 t0_max,             /* (i)     : maximum value in the searched range.   */
    G729_Word16 i_subfr,            /* (i)     : indicator for first subframe.          */
    G729_Word16 *pit_frac           /* (o)     : chosen fraction.                       */
)
{
    G729_Word16 t, t0;
    G729_Word16 Dn[L_SUBFR];
    G729_Word16 exc_tmp[L_SUBFR];
    G729_Word32 max, corr, L_temp;
    
    /*-----------------------------------------------------------------*
     * Compute correlation of target vector with impulse response.     *
     *-----------------------------------------------------------------*/
    
    g729_Cor_h_X(h, xn, Dn);
    
    /*-----------------------------------------------------------------*
     * Find maximum integer delay.                                     *
     *-----------------------------------------------------------------*/
    
    max = G729A_MIN_32;
    t0 = t0_min; /* Only to remove warning from some compilers */
    
    for(t=t0_min; t<=t0_max; t++)
    {
        corr = Dot_Product(Dn, &exc[-t], L_subfr);
        L_temp = g729_L_sub(corr, max);
        if(L_temp > 0) {max = corr; t0 = t;  }
    }
    
    /*-----------------------------------------------------------------*
     * Test fractions.                                                 *
     *-----------------------------------------------------------------*/
    
    /* Fraction 0 */
    
    g729_Pred_lt_3(exc, t0, 0, L_subfr);
    max = Dot_Product(Dn, exc, L_subfr);
    *pit_frac = 0;
    
    /* If first subframe and lag > 84 do not search fractional pitch */
    
    if( (i_subfr == 0) && (g729_sub(t0, 84) > 0) )
        return t0;
    
    g729_Copy(exc, exc_tmp, L_subfr);
    
    /* Fraction -1/3 */
    
    g729_Pred_lt_3(exc, t0, -1, L_subfr);
    corr = Dot_Product(Dn, exc, L_subfr);
    L_temp = g729_L_sub(corr, max);
    if(L_temp > 0) {
        max = corr;
        *pit_frac = -1;
        g729_Copy(exc, exc_tmp, L_subfr);
    }
    
    /* Fraction +1/3 */
    
    g729_Pred_lt_3(exc, t0, 1, L_subfr);
    corr = Dot_Product(Dn, exc, L_subfr);
    L_temp = g729_L_sub(corr, max);
    if(L_temp > 0) {
        max = corr;
        *pit_frac =  1;
    }
    else
        g729_Copy(exc_tmp, exc, L_subfr);
    
    return t0;
}


/*---------------------------------------------------------------------*
 * Function  g729_G_pitch:                                             *
 *           ~~~~~~~~                                                  *
 *---------------------------------------------------------------------*
 * Compute correlations <xn,y1> and <y1,y1> to use in gains quantizer. *
 * Also compute the gain of pitch. Result in Q14                       *
 *  if (gain < 0)  gain =0                                             *
 *  if (gain >1.2) gain =1.2                                           *
 *---------------------------------------------------------------------*/


G729_Word16 g729_G_pitch(   /* (o) Q14 : Gain of pitch lag saturated to 1.2       */
    G729_Word16 xn[],       /* (i)     : Pitch target.                            */
    G729_Word16 y1[],       /* (i)     : Filtered adaptive codebook.              */
    G729_Word16 g_coeff[],  /* (i)     : Correlations need for gain quantization. */
    G729_Word16 L_subfr     /* (i)     : Length of subframe.                      */
)
{
    G729_Word16 i;
    G729_Word16 xy, yy, exp_xy, exp_yy, gain;
    G729_Word32 s;
    
    G729_Word16 scaled_y1[L_SUBFR];
    
    /* add '#if' to avoid warning(-Wunused-variable) */
#if !defined(USE_GLOBAL_OVERFLOW_FLAG) || (USE_GLOBAL_OVERFLOW_FLAG != 1)
    G729_Word32 s1, L_temp;
    G729_Flag overflow;
#endif
    
    /* divide "y1[]" by 4 to avoid overflow */
    
    for(i=0; i<L_subfr; i++)
        scaled_y1[i] = g729_shr(y1[i], 2);
    
    /* Compute scalar product <y1[],y1[]> */
    
    s = 1;                    /* Avoid case of all zeros */

#if defined(USE_GLOBAL_OVERFLOW_FLAG) && (USE_GLOBAL_OVERFLOW_FLAG == 1)

    G729A_Overflow_Flag = 0;
    for(i=0; i<L_subfr; i++)
        s = g729_L_mac(s, y1[i], y1[i]);

#else
    
    overflow = 0;
    for ( i = 0; i < L_subfr; ++i )
    {
        s += (((G729_Word32)y1[i] * (G729_Word32)y1[i]) << 1);
        if ( s < 0 )
        {
            overflow = 1;
            break;
        }
    }
#endif
    
#if defined(USE_GLOBAL_OVERFLOW_FLAG) && (USE_GLOBAL_OVERFLOW_FLAG == 1)
    if (G729A_Overflow_Flag == 0)
#else
    if (overflow == 0)
#endif
    {
        exp_yy = g729_norm_l(s);
        yy     = g729_round( g729_L_shl(s, exp_yy) );
    }
    else
    {
        s = 1;                  /* Avoid case of all zeros */
        for(i=0; i<L_subfr; i++)
            s = g729_L_mac(s, scaled_y1[i], scaled_y1[i]);
        exp_yy = g729_norm_l(s);
        yy     = g729_round( g729_L_shl(s, exp_yy) );
        exp_yy = g729_sub(exp_yy, 4);
    }
    
    /* Compute scalar product <xn[],y1[]> */
    
    s = 0;
    
#if defined(USE_GLOBAL_OVERFLOW_FLAG) && (USE_GLOBAL_OVERFLOW_FLAG == 1)
    
    G729A_Overflow_Flag = 0;
    for(i=0; i<L_subfr; i++)
        s = g729_L_mac(s, xn[i], y1[i]);

#else
    
    overflow = 0;
    for ( i = 0; i < L_subfr; ++i )
    {
        L_temp = xn[i] * y1[i];
        if ( (G729_Word32)0x40000000 == L_temp )
        {
            overflow = 1;
            break;
        }
        
        s1 = s;
        s = (L_temp << 1) + s1;
        
        if (((s1 ^ L_temp) > 0) && ((s ^ s1) < 0))
        {
            overflow = 1;
            break;
        }
    }
#endif
    
#if defined(USE_GLOBAL_OVERFLOW_FLAG) && (USE_GLOBAL_OVERFLOW_FLAG == 1)
    if (G729A_Overflow_Flag == 0)
#else
    if (overflow == 0)
#endif
    {
        exp_xy = g729_norm_l(s);
        xy     = g729_round( g729_L_shl(s, exp_xy) );
    }
    else
    {
        s = 0;
        for(i=0; i<L_subfr; i++)
            s = g729_L_mac(s, xn[i], scaled_y1[i]);
        exp_xy = g729_norm_l(s);
        xy     = g729_round( g729_L_shl(s, exp_xy) );
        exp_xy = g729_sub(exp_xy, 2);
    }
    
    g_coeff[0] = yy;
    g_coeff[1] = g729_sub(15, exp_yy);
    g_coeff[2] = xy;
    g_coeff[3] = g729_sub(15, exp_xy);
    
    /* If (xy <= 0) gain = 0 */
    
    if (xy <= 0)
    {
        g_coeff[3] = -15;   /* Force exp_xy to -15 = (15-30) */
        return( (G729_Word16) 0);
    }
    
    /* compute gain = xy/yy */
    
    xy = g729_shr(xy, 1);             /* Be sure xy < yy */
    gain = g729_div_s( xy, yy);
    
    i = g729_sub(exp_xy, exp_yy);
    gain = g729_shr(gain, i);         /* saturation if > 1.99 in Q14 */
    
    /* if(gain >1.2) gain = 1.2  in Q14 */
    
    if( g729_sub(gain, 19661) > 0)
    {
        gain = 19661;
    }
    
    
    return(gain);
}



/*----------------------------------------------------------------------*
 *    Function g729_Enc_lag3                                            *
 *             ~~~~~~~~                                                 *
 *   Encoding of fractional pitch lag with 1/3 resolution.              *
 *----------------------------------------------------------------------*
 * The pitch range for the first subframe is divided as follows:        *
 *   19 1/3  to   84 2/3   resolution 1/3                               *
 *   85      to   143      resolution 1                                 *
 *                                                                      *
 * The period in the first subframe is encoded with 8 bits.             *
 * For the range with fractions:                                        *
 *   index = (T-19)*3 + frac - 1;   where T=[19..85] and frac=[-1,0,1]  *
 * and for the integer only range                                       *
 *   index = (T - 85) + 197;        where T=[86..143]                   *
 *----------------------------------------------------------------------*
 * For the second subframe a resolution of 1/3 is always used, and the  *
 * search range is relative to the lag in the first subframe.           *
 * If t0 is the lag in the first subframe then                          *
 *  t_min=t0-5   and  t_max=t0+4   and  the range is given by           *
 *       t_min - 2/3   to  t_max + 2/3                                  *
 *                                                                      *
 * The period in the 2nd subframe is encoded with 5 bits:               *
 *   index = (T-(t_min-1))*3 + frac - 1;    where T[t_min-1 .. t_max+1] *
 *----------------------------------------------------------------------*/


G729_Word16 g729_Enc_lag3(  /* output: Return index of encoding */
    G729_Word16 T0,         /* input : Pitch delay              */
    G729_Word16 T0_frac,    /* input : Fractional pitch delay   */
    G729_Word16 *T0_min,    /* in/out: Minimum search delay     */
    G729_Word16 *T0_max,    /* in/out: Maximum search delay     */
    G729_Word16 pit_min,    /* input : Minimum pitch delay      */
    G729_Word16 pit_max,    /* input : Maximum pitch delay      */
    G729_Word16 pit_flag    /* input : Flag for 1st subframe    */
)
{
    G729_Word16 index, i;
    
    if (pit_flag == 0)   /* if 1st subframe */
    {
        /* encode pitch delay (with fraction) */
        
        if (g729_sub(T0, 85) <= 0)
        {
            /* index = t0*3 - 58 + t0_frac   */
            i = g729_add(g729_add(T0, T0), T0);
            index = g729_add(g729_sub(i, 58), T0_frac);
        }
        else {
            index = g729_add(T0, 112);
        }
        
        /* find T0_min and T0_max for second subframe */
        
        *T0_min = g729_sub(T0, 5);
        if (g729_sub(*T0_min, pit_min) < 0)
        {
            *T0_min = pit_min;
        }
        
        *T0_max = g729_add(*T0_min, 9);
        if (g729_sub(*T0_max, pit_max) > 0)
        {
            *T0_max = pit_max;
            *T0_min = g729_sub(*T0_max, 9);
        }
    }
    else      /* if second subframe */
    {
        
        /* i = t0 - t0_min;               */
        /* index = i*3 + 2 + t0_frac;     */
        i = g729_sub(T0, *T0_min);
        i = g729_add(g729_add(i, i), i);
        index = g729_add(g729_add(i, 2), T0_frac);
    }
    
    
    return index;
}

