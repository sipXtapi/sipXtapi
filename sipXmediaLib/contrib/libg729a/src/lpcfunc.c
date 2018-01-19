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

/*-------------------------------------------------------------*
 *  Procedure g729_Lsp_Az:                                     *
 *            ~~~~~~                                           *
 *   Compute the LPC coefficients from lsp (order=10)          *
 *-------------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "oper_32b.h"

#include "ld8a.h"
#include "tab_ld8a.h"

/* local function */

static void Get_lsp_pol(G729_Word16 *lsp, G729_Word32 *f);

void g729_Lsp_Az(
    G729_Word16 lsp[],    /* (i) Q15 : line spectral frequencies            */
    G729_Word16 a[]       /* (o) Q12 : predictor coefficients (order = 10)  */
)
{
    G729_Word16 i, j;
    G729_Word32 f1[6], f2[6];
    G729_Word32 t0;
    
    Get_lsp_pol(&lsp[0],f1);
    Get_lsp_pol(&lsp[1],f2);
    
    for (i = 5; i > 0; i--)
    {
        f1[i] = g729_L_add(f1[i], f1[i-1]);        /* f1[i] += f1[i-1]; */
        f2[i] = g729_L_sub(f2[i], f2[i-1]);        /* f2[i] -= f2[i-1]; */
    }
    
    a[0] = 4096;
    for (i = 1, j = 10; i <= 5; i++, j--)
    {
        t0   = g729_L_add(f1[i], f2[i]);                      /* f1[i] + f2[i]             */
        a[i] = g729_extract_l( g729_L_shr_r(t0, 13) );        /* from Q24 to Q12 and * 0.5 */
        
        t0   = g729_L_sub(f1[i], f2[i]);                      /* f1[i] - f2[i]             */
        a[j] = g729_extract_l( g729_L_shr_r(t0, 13) );        /* from Q24 to Q12 and * 0.5 */
        
    }
    
    return;
}

/*-----------------------------------------------------------*
 * procedure Get_lsp_pol:                                    *
 *           ~~~~~~~~~~~                                     *
 *   Find the polynomial F1(z) or F2(z) from the LSPs        *
 *-----------------------------------------------------------*
 *                                                           *
 * Parameters:                                               *
 *  lsp[]   : line spectral freq. (cosine domain)    in Q15  *
 *  f[]     : the coefficients of F1 or F2           in Q24  *
 *-----------------------------------------------------------*/

static void Get_lsp_pol(G729_Word16 *lsp, G729_Word32 *f)
{
    G729_Word16 i,j, hi, lo;
    G729_Word32 t0;
    
    /* All computation in Q24 */
    
    *f = g729_L_mult(4096, 2048);             /* f[0] = 1.0;             in Q24  */
    f++;
    *f = g729_L_msu((G729_Word32)0, *lsp, 512);    /* f[1] =  -2.0 * lsp[0];  in Q24  */
    
    f++;
    lsp += 2;                            /* Advance lsp pointer             */
    
    for(i=2; i<=5; i++)
    {
        *f = f[-2];
        
        for(j=1; j<i; j++, f--)
        {
            g729_L_Extract(f[-1] ,&hi, &lo);
            t0 = g729_Mpy_32_16(hi, lo, *lsp);         /* t0 = f[-1] * lsp    */
            t0 = g729_L_shl(t0, 1);
            *f = g729_L_add(*f, f[-2]);                /* *f += f[-2]         */
            *f = g729_L_sub(*f, t0);                   /* *f -= t0            */
        }
        *f   = g729_L_msu(*f, *lsp, 512);              /* *f -= lsp<<9        */
        f   += i;                                      /* Advance f pointer   */
        lsp += 2;                                      /* Advance lsp pointer */
    }
    
    return;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Functions : g729_Lsp_lsf and g729_Lsf_lsp                               |
 |                                                                           |
 |      g729_Lsp_lsf   Transformation lsp to lsf                             |
 |      g729_Lsf_lsp   Transformation lsf to lsp                             |
 |---------------------------------------------------------------------------|
 |  Algorithm:                                                               |
 |                                                                           |
 |   The transformation from lsp[i] to lsf[i] and lsf[i] to lsp[i] are       |
 |   approximated by a look-up g729_table and interpolation.                 |
 |___________________________________________________________________________|
 */


void g729_Lsf_lsp(
    G729_Word16 lsf[],    /* (i) Q15 : lsf[m] normalized (range: 0.0<=val<=0.5) */
    G729_Word16 lsp[],    /* (o) Q15 : lsp[m] (range: -1<=val<1)                */
    G729_Word16 m         /* (i)     : LPC order                                */
)
{
    G729_Word16 i, ind, offset;
    G729_Word32 L_tmp;
    
    for(i=0; i<m; i++)
    {
        ind    = g729_shr(lsf[i], 8);               /* ind    = b8-b15 of lsf[i] */
        offset = lsf[i] & (G729_Word16)0x00ff;      /* offset = b0-b7  of lsf[i] */
        
        /* lsp[i] = g729_table[ind]+ ((g729_table[ind+1]-g729_table[ind])*offset) / 256 */
        
        L_tmp   = g729_L_mult(g729_sub(g729_table[ind+1], g729_table[ind]), offset);
        lsp[i] = g729_add(g729_table[ind], g729_extract_l(g729_L_shr(L_tmp, 9)));
    }
    return;
}


void g729_Lsp_lsf(
    G729_Word16 lsp[],    /* (i) Q15 : lsp[m] (range: -1<=val<1)                */
    G729_Word16 lsf[],    /* (o) Q15 : lsf[m] normalized (range: 0.0<=val<=0.5) */
    G729_Word16 m         /* (i)     : LPC order                                */
)
{
    G729_Word16 i, ind, tmp;
    G729_Word32 L_tmp;
    
    ind = 63;    /* begin at end of g729_table -1 */
    
    for(i= m-(G729_Word16)1; i >= 0; i--)
    {
        /* find value in g729_table that is just greater than lsp[i] */
        while( g729_sub(g729_table[ind], lsp[i]) < 0 )
        {
            ind = g729_sub(ind,1);
        }
        
        /* acos(lsp[i])= ind*256 + ( ( lsp[i]-g729_table[ind] ) * g729_slope[ind] )/4096 */
        
        L_tmp  = g729_L_mult( g729_sub(lsp[i], g729_table[ind]) , g729_slope[ind] );
        tmp = g729_round(g729_L_shl(L_tmp, 3));     /*(lsp[i]-g729_table[ind])*g729_slope[ind])>>12*/
        lsf[i] = g729_add(tmp, g729_shl(ind, 8));
    }
    return;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Functions : g729_Lsp_lsf and g729_Lsf_lsp                               |
 |                                                                           |
 |      g729_Lsp_lsf   Transformation lsp to lsf                             |
 |      g729_Lsf_lsp   Transformation lsf to lsp                             |
 |---------------------------------------------------------------------------|
 |  Algorithm:                                                               |
 |                                                                           |
 |   The transformation from lsp[i] to lsf[i] and lsf[i] to lsp[i] are       |
 |   approximated by a look-up g729_table and interpolation.                 |
 |___________________________________________________________________________|
 */

void g729_Lsf_lsp2(
    G729_Word16 lsf[],    /* (i) Q13 : lsf[m] (range: 0.0<=val<PI) */
    G729_Word16 lsp[],    /* (o) Q15 : lsp[m] (range: -1<=val<1)   */
    G729_Word16 m         /* (i)     : LPC order                   */
)
{
    G729_Word16 i, ind;
    G729_Word16 offset;   /* in Q8 */
    G729_Word16 freq;     /* normalized frequency in Q15 */
    G729_Word32 L_tmp;
    
    for(i=0; i<m; i++)
    {
        /*    freq = g729_abs_s(freq);*/
        freq = g729_mult(lsf[i], 20861);          /* 20861: 1.0/(2.0*PI) in Q17 */
        ind    = g729_shr(freq, 8);               /* ind    = b8-b15 of freq */
        offset = freq & (G729_Word16)0x00ff;      /* offset = b0-b7  of freq */
        
        if ( g729_sub(ind, 63)>0 ){
            ind = 63;                 /* 0 <= ind <= 63 */
        }
        
        /* lsp[i] = g729_table2[ind]+ (g729_slope_cos[ind]*offset >> 12) */
        
        L_tmp   = g729_L_mult(g729_slope_cos[ind], offset);   /* L_tmp in Q28 */
        lsp[i] = g729_add(g729_table2[ind], g729_extract_l(g729_L_shr(L_tmp, 13)));
        
    }
    return;
}

void g729_Lsp_lsf2(
    G729_Word16 lsp[],    /* (i) Q15 : lsp[m] (range: -1<=val<1)   */
    G729_Word16 lsf[],    /* (o) Q13 : lsf[m] (range: 0.0<=val<PI) */
    G729_Word16 m         /* (i)     : LPC order                   */
)
{
    G729_Word16 i, ind;
    G729_Word16 offset;   /* in Q15 */
    G729_Word16 freq;     /* normalized frequency in Q16 */
    G729_Word32 L_tmp;
    
    ind = 63;           /* begin at end of g729_table2 -1 */
    
    for(i= m-(G729_Word16)1; i >= 0; i--)
    {
        /* find value in g729_table2 that is just greater than lsp[i] */
        while( g729_sub(g729_table2[ind], lsp[i]) < 0 )
        {
            ind = g729_sub(ind,1);
            if ( ind <= 0 )
                break;
        }
        
        offset = g729_sub(lsp[i], g729_table2[ind]);
        
        /* acos(lsp[i])= ind*512 + (g729_slope_acos[ind]*offset >> 11) */
        
        L_tmp  = g729_L_mult( g729_slope_acos[ind], offset );   /* L_tmp in Q28 */
        freq = g729_add(g729_shl(ind, 9), g729_extract_l(g729_L_shr(L_tmp, 12)));
        lsf[i] = g729_mult(freq, 25736);           /* 25736: 2.0*PI in Q12 */
        
    }
    return;
}

/*-------------------------------------------------------------*
 *  procedure g729_Weight_Az                                   *
 *            ~~~~~~~~~                                        *
 * Weighting of LPC coefficients.                              *
 *   ap[i]  =  a[i] * (gamma ** i)                             *
 *                                                             *
 *-------------------------------------------------------------*/


void g729_Weight_Az(
    G729_Word16 a[],      /* (i) Q12 : a[m+1]  LPC coefficients             */
    G729_Word16 gamma,    /* (i) Q15 : Spectral expansion factor.           */
    G729_Word16 m,        /* (i)     : LPC order.                           */
    G729_Word16 ap[]      /* (o) Q12 : Spectral expanded LPC coefficients   */
)
{
    G729_Word16 i, fac;
    
    ap[0] = a[0];
    fac   = gamma;
    for(i=1; i<m; i++)
    {
        ap[i] = g729_round( g729_L_mult(a[i], fac) );
        fac   = g729_round( g729_L_mult(fac, gamma) );
    }
    ap[m] = g729_round( g729_L_mult(a[m], fac) );
    
    return;
}

/*----------------------------------------------------------------------*
 * Function g729_Int_qlpc()                                             *
 * ~~~~~~~~~~~~~~~~~~~                                                  *
 * Interpolation of the LPC parameters.                                 *
 *----------------------------------------------------------------------*/

/* Interpolation of the quantized LSP's */

void g729_Int_qlpc(
    G729_Word16 lsp_old[], /* input : LSP vector of past frame              */
    G729_Word16 lsp_new[], /* input : LSP vector of present frame           */
    G729_Word16 Az[]       /* output: interpolated Az() for the 2 subframes */
)
{
    G729_Word16 i;
    G729_Word16 lsp[M];
    
    /*  lsp[i] = lsp_new[i] * 0.5 + lsp_old[i] * 0.5 */
    
    for (i = 0; i < M; i++) {
        lsp[i] = g729_add(g729_shr(lsp_new[i], 1), g729_shr(lsp_old[i], 1));
    }
    
    g729_Lsp_Az(lsp, Az);              /* Subframe 1 */
    
    g729_Lsp_Az(lsp_new, &Az[MP1]);    /* Subframe 2 */
    
    return;
}


