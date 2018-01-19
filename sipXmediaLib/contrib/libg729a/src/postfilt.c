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

/*------------------------------------------------------------------------*
 *                         POSTFILTER.C                                   *
 *------------------------------------------------------------------------*
 * Performs adaptive postfiltering on the synthesis speech                *
 * This file contains all functions related to the post filter.           *
 *------------------------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"
#include "oper_32b.h"

#include "g729a_decoder.h"

static void g729_pit_pst_filt(
    G729_Word16 *signal,      /* (i)     : input signal                        */
    G729_Word16 *scal_sig,    /* (i)     : input signal (scaled, divided by 4) */
    G729_Word16 t0_min,       /* (i)     : minimum value in the searched range */
    G729_Word16 t0_max,       /* (i)     : maximum value in the searched range */
    G729_Word16 L_subfr,      /* (i)     : size of filtering                   */
    G729_Word16 *signal_pst   /* (o)     : harmonically postfiltered signal    */
);

static void g729_preemphasis(
    g729a_post_filter_state * state,
    G729_Word16 *signal,  /* (i/o)   : input signal overwritten by the output */
    G729_Word16 g,        /* (i) Q15 : g729_preemphasis coefficient           */
    G729_Word16 L         /* (i)     : size of filtering                      */
);

static void g729_agc(
    g729a_post_filter_state * state,
    G729_Word16 *sig_in,   /* (i)     : postfilter input signal  */
    G729_Word16 *sig_out,  /* (i/o)   : postfilter output signal */
    G729_Word16 l_trm      /* (i)     : subframe size            */
);

/*---------------------------------------------------------------*
 *    Postfilter constant parameters (defined in "ld8a.h")       *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   L_SUBFR     : Sub-frame size.                               *
 *   M           : LPC order.                                    *
 *   MP1         : LPC order+1                                   *
 *   PIT_MAX     : Maximum pitch lag.                            *
 *   GAMMA2_PST  : Formant postfiltering factor (numerator)      *
 *   GAMMA1_PST  : Formant postfiltering factor (denominator)    *
 *   GAMMAP      : Harmonic postfiltering factor                 *
 *   MU          : Factor for tilt compensation filter           *
 *   AGC_FAC     : Factor for automatic gain control             *
 *---------------------------------------------------------------*/

/*---------------------------------------------------------------*
 * Procedure    g729_Init_Post_Filter:                           *
 *              ~~~~~~~~~~~~~~~~                                 *
 *  Initializes the postfilter parameters:                       *
 *---------------------------------------------------------------*/

void g729_Init_Post_Filter(g729a_post_filter_state * state)
{
    state->res2  = state->res2_buf + PIT_MAX;
    state->scal_res2  = state->scal_res2_buf + PIT_MAX;
    
    g729_Set_zero(state->mem_syn_pst, M);
    g729_Set_zero(state->res2_buf, PIT_MAX+L_SUBFR);
    g729_Set_zero(state->scal_res2_buf, PIT_MAX+L_SUBFR);
    
    state->mem_pre = 0;
    state->past_gain = 4096;
    
    return;
}

/*------------------------------------------------------------------------*
 *  Procedure     g729_Post_Filter:                                       *
 *                ~~~~~~~~~~~                                             *
 *------------------------------------------------------------------------*
 *  The postfiltering process is described as follows:                    *
 *                                                                        *
 *  - inverse filtering of syn[] through A(z/GAMMA2_PST) to get res2[]    *
 *  - use res2[] to compute pitch parameters                              *
 *  - perform pitch postfiltering                                         *
 *  - tilt compensation filtering; 1 - MU*k*z^-1                          *
 *  - synthesis filtering through 1/A(z/GAMMA1_PST)                       *
 *  - adaptive gain control                                               *
 *------------------------------------------------------------------------*/



void g729_Post_Filter(
    g729a_post_filter_state * state,
    G729_Word16 *syn,       /* in/out: synthesis speech (postfiltered is output)    */
    G729_Word16 *Az_4,      /* input : interpolated LPC parameters in all subframes */
    G729_Word16 *T          /* input : decoded pitch lags in all subframes          */
)
{
    /*-------------------------------------------------------------------*
     *           Declaration of parameters                               *
     *-------------------------------------------------------------------*/
    
    G729_Word16 res2_pst[L_SUBFR];  /* res2[] after pitch postfiltering */
    G729_Word16 syn_pst[L_FRAME];   /* post filtered synthesis speech   */
    
    G729_Word16 Ap3[MP1], Ap4[MP1];  /* bandwidth expanded LP parameters */
    
    G729_Word16 *Az;                 /* pointer to Az_4:                 */
    /*  LPC parameters in each subframe */
    G729_Word16   t0_max, t0_min;    /* closed-loop pitch search range   */
    G729_Word16   i_subfr;           /* index for beginning of subframe  */
    
    G729_Word16 h[L_H];
    
    G729_Word16  i, j;
    G729_Word16  temp1, temp2;
    G729_Word32  L_tmp;
    
    /*-----------------------------------------------------*
     * Post filtering                                      *
     *-----------------------------------------------------*/
    
    Az = Az_4;
    
    for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
    {
        /* Find pitch range t0_min - t0_max */
        
        t0_min = g729_sub(*T++, 3);
        t0_max = g729_add(t0_min, 6);
        if (g729_sub(t0_max, PIT_MAX) > 0) {
            t0_max = PIT_MAX;
            t0_min = g729_sub(t0_max, 6);
        }
        
        /* Find weighted filter coefficients Ap3[] and ap[4] */
        
        g729_Weight_Az(Az, GAMMA2_PST, M, Ap3);
        g729_Weight_Az(Az, GAMMA1_PST, M, Ap4);
        
        /* filtering of synthesis speech by A(z/GAMMA2_PST) to find res2[] */
        
        g729_Residu(Ap3, &syn[i_subfr], state->res2, L_SUBFR);
        
        /* scaling of "res2[]" to avoid energy overflow */
        
        for (j=0; j<L_SUBFR; j++)
        {
            state->scal_res2[j] = g729_shr(state->res2[j], 2);
        }
        
        /* pitch postfiltering */
        
        g729_pit_pst_filt(state->res2, state->scal_res2, t0_min, t0_max, L_SUBFR, res2_pst);
        
        /* tilt compensation filter */
        
        /* impulse response of A(z/GAMMA2_PST)/A(z/GAMMA1_PST) */
        
        g729_Copy(Ap3, h, M+1);
        g729_Set_zero(&h[M+1], L_H-M-1);
        g729_Syn_filt(Ap4, h, h, L_H, &h[M+1], 0);
        
        /* 1st correlation of h[] */
        
        L_tmp = g729_L_mult(h[0], h[0]);
        for (i=1; i<L_H; i++) L_tmp = g729_L_mac(L_tmp, h[i], h[i]);
        temp1 = g729_extract_h(L_tmp);
        
        L_tmp = g729_L_mult(h[0], h[1]);
        for (i=1; i<L_H-1; i++) L_tmp = g729_L_mac(L_tmp, h[i], h[i+1]);
        temp2 = g729_extract_h(L_tmp);
        
        if(temp2 <= 0) {
            temp2 = 0;
        }
        else {
            temp2 = g729_mult(temp2, MU);
            temp2 = g729_div_s(temp2, temp1);
        }
        
        g729_preemphasis(state, res2_pst, temp2, L_SUBFR);
        
        /* filtering through  1/A(z/GAMMA1_PST) */
        
        g729_Syn_filt(Ap4, res2_pst, &syn_pst[i_subfr], L_SUBFR, state->mem_syn_pst, 1);
        
        /* scale output to input */
        
        g729_agc(state, &syn[i_subfr], &syn_pst[i_subfr], L_SUBFR);
        
        /* update res2[] buffer;  shift by L_SUBFR */
        
        g729_Copy(&(state->res2[L_SUBFR-PIT_MAX]), &(state->res2[-PIT_MAX]), PIT_MAX);
        g729_Copy(&(state->scal_res2[L_SUBFR-PIT_MAX]), &(state->scal_res2[-PIT_MAX]), PIT_MAX);
        
        Az += MP1;
    }
    
    /* update syn[] buffer */
    
    g729_Copy(&syn[L_FRAME-M], &syn[-M], M);
    
    /* overwrite synthesis speech by postfiltered synthesis speech */
    
    g729_Copy(syn_pst, syn, L_FRAME);
    
    return;
}

/*---------------------------------------------------------------------------*
 * procedure pitch_pst_filt                                                  *
 * ~~~~~~~~~~~~~~~~~~~~~~~~                                                  *
 * Find the pitch period  around the transmitted pitch and perform           *
 * harmonic postfiltering.                                                   *
 * Filtering through   (1 + g z^-T) / (1+g) ;   g = min(pit_gain*gammap, 1)  *
 *--------------------------------------------------------------------------*/

static void g729_pit_pst_filt(
    G729_Word16 *signal,      /* (i)     : input signal                        */
    G729_Word16 *scal_sig,    /* (i)     : input signal (scaled, divided by 4) */
    G729_Word16 t0_min,       /* (i)     : minimum value in the searched range */
    G729_Word16 t0_max,       /* (i)     : maximum value in the searched range */
    G729_Word16 L_subfr,      /* (i)     : size of filtering                   */
    G729_Word16 *signal_pst   /* (o)     : harmonically postfiltered signal    */
)
{
    G729_Word16 i, j, t0;
    G729_Word16 g0, gain, cmax, en, en0;
    G729_Word16 *p, *p1, *deb_sig;
    G729_Word32 corr, cor_max, ener, ener0, temp;
    G729_Word32 L_temp;
    
    /*---------------------------------------------------------------------------*
     * Compute the correlations for all delays                                   *
     * and select the delay which maximizes the correlation                      *
     *---------------------------------------------------------------------------*/
    
    deb_sig = &scal_sig[-t0_min];
    cor_max = G729A_MIN_32;
    t0 = t0_min;             /* Only to remove warning from some compilers */
    for (i=t0_min; i<=t0_max; i++)
    {
        corr = 0;
        p    = scal_sig;
        p1   = deb_sig;
        for (j=0; j<L_subfr; j++)
            corr = g729_L_mac(corr, *p++, *p1++);
        
        L_temp = g729_L_sub(corr, cor_max);
        if (L_temp > (G729_Word32)0)
        {
            cor_max = corr;
            t0 = i;
        }
        deb_sig--;
    }
    
    /* Compute the energy of the signal delayed by t0 */
    
    ener = 1;
    p = scal_sig - t0;
    for ( i=0; i<L_subfr ;i++, p++)
        ener = g729_L_mac(ener, *p, *p);
    
    /* Compute the signal energy in the present subframe */
    
    ener0 = 1;
    p = scal_sig;
    for ( i=0; i<L_subfr; i++, p++)
        ener0 = g729_L_mac(ener0, *p, *p);
    
    if (cor_max < 0)
    {
        cor_max = 0;
    }
    
    /* scale "cor_max", "ener" and "ener0" on 16 bits */
    
    temp = cor_max;
    if (ener > temp)
    {
        temp = ener;
    }
    if (ener0 > temp)
    {
        temp = ener0;
    }
    j = g729_norm_l(temp);
    cmax = g729_round(g729_L_shl(cor_max, j));
    en = g729_round(g729_L_shl(ener, j));
    en0 = g729_round(g729_L_shl(ener0, j));
    
    /* prediction gain (dB)= -10 log(1-cor_max*cor_max/(ener*ener0)) */
    
    /* temp = (cor_max * cor_max) - (0.5 * ener * ener0)  */
    temp = g729_L_mult(cmax, cmax);
    temp = g729_L_sub(temp, g729_L_shr(g729_L_mult(en, en0), 1));
    
    if (temp < (G729_Word32)0)           /* if prediction gain < 3 dB   */
    {                               /* switch off pitch postfilter */
        for (i = 0; i < L_subfr; i++)
            signal_pst[i] = signal[i];
        return;
    }
    
    if (g729_sub(cmax, en) > 0)      /* if pitch gain > 1 */
    {
        g0 = INV_GAMMAP;
        gain = GAMMAP_2;
    }
    else {
        cmax = g729_shr(g729_mult(cmax, GAMMAP), 1);  /* cmax(Q14) = cmax(Q15) * GAMMAP */
        en = g729_shr(en, 1);          /* Q14 */
        i = g729_add(cmax, en);
        if(i > 0)
        {
            gain = g729_div_s(cmax, i);    /* gain(Q15) = cor_max/(cor_max+ener)  */
            g0 = g729_sub(32767, gain);    /* g0(Q15) = 1 - gain */
        }
        else
        {
            g0 =  32767;
            gain = 0;
        }
    }
    
    
    for (i = 0; i < L_subfr; i++)
    {
        /* signal_pst[i] = g0*signal[i] + gain*signal[i-t0]; */
        
        signal_pst[i] = g729_add(g729_mult(g0, signal[i]), g729_mult(gain, signal[i-t0]));
        
    }
    
    return;
}

/*---------------------------------------------------------------------*
 * routine g729_preemphasis()                                          *
 * ~~~~~~~~~~~~~~~~~~~~~                                               *
 * Preemphasis: filtering through 1 - g z^-1                           *
 *---------------------------------------------------------------------*/

static void g729_preemphasis(
    g729a_post_filter_state * state,
    G729_Word16 *signal,  /* (i/o)   : input signal overwritten by the output */
    G729_Word16 g,        /* (i) Q15 : g729_preemphasis coefficient           */
    G729_Word16 L         /* (i)     : size of filtering                      */
)
{
    /*
     * moved to g729a_post_filter_state struct.
     */
    /* static G729_Word16 mem_pre = 0; */
    G729_Word16 *p1, *p2, temp, i;
    
    p1 = signal + L - 1;
    p2 = p1 - 1;
    temp = *p1;
    
    for (i = 0; i <= L-2; ++i, --p1)
    {
        *p1 = g729_sub(*p1, g729_mult(g, *p2--));
    }
    
    *p1 = g729_sub(*p1, g729_mult(g, state->mem_pre));
    
    state->mem_pre = temp;
    
    return;
}

/*----------------------------------------------------------------------*
 *   routine g729_agc()                                                 *
 *   ~~~~~~~~~~~~~                                                      *
 * Scale the postfilter output on a subframe basis by automatic control *
 * of the subframe gain.                                                *
 *  gain[n] = AGC_FAC * gain[n-1] + (1 - AGC_FAC) g_in/g_out            *
 *----------------------------------------------------------------------*/

static void g729_agc(
    g729a_post_filter_state * state,
    G729_Word16 *sig_in,   /* (i)     : postfilter input signal  */
    G729_Word16 *sig_out,  /* (i/o)   : postfilter output signal */
    G729_Word16 l_trm      /* (i)     : subframe size            */
)
{
    /*
     * moved to g729a_post_filter_state struct.
     */
    /* static G729_Word16 past_gain=4096; */         /* past_gain = 1.0 (Q12) */
    G729_Word16 i, exp;
    G729_Word16 gain_in, gain_out, g0, gain;                     /* Q12 */
    G729_Word32 s;
    
    G729_Word16 signal[L_SUBFR];
    
    /* calculate gain_out with exponent */
    
    for(i=0; i<l_trm; i++)
        signal[i] = g729_shr(sig_out[i], 2);
    
    s = 0;
    for(i=0; i<l_trm; i++)
        s = g729_L_mac(s, signal[i], signal[i]);
    
    if (s == 0) {
        state->past_gain = 0;
        return;
    }
    exp = g729_sub(g729_norm_l(s), 1);
    gain_out = g729_round(g729_L_shl(s, exp));
    
    /* calculate gain_in with exponent */
    
    for(i=0; i<l_trm; i++)
        signal[i] = g729_shr(sig_in[i], 2);
    
    s = 0;
    for(i=0; i<l_trm; i++)
        s = g729_L_mac(s, signal[i], signal[i]);
    
    if (s == 0) {
        g0 = 0;
    }
    else {
        i = g729_norm_l(s);
        gain_in = g729_round(g729_L_shl(s, i));
        exp = g729_sub(exp, i);
        
        /*---------------------------------------------------*
         *  g0(Q12) = (1-AGC_FAC) * sqrt(gain_in/gain_out);  *
         *---------------------------------------------------*/
        
        s = g729_L_deposit_l(g729_div_s(gain_out,gain_in));   /* Q15 */
        s = g729_L_shl(s, 7);           /* s(Q22) = gain_out / gain_in */
        s = g729_L_shr(s, exp);         /* Q22, g729_add exponent */
        
        /* i(Q12) = s(Q19) = 1 / sqrt(s(Q22)) */
        s = g729_Inv_sqrt(s);           /* Q19 */
        i = g729_round(g729_L_shl(s,9));     /* Q12 */
        
        /* g0(Q12) = i(Q12) * (1-AGC_FAC)(Q15) */
        g0 = g729_mult(i, AGC_FAC1);       /* Q12 */
    }
    
    /* compute gain(n) = AGC_FAC gain(n-1) + (1-AGC_FAC)gain_in/gain_out */
    /* sig_out(n) = gain(n) sig_out(n)                                   */
    
    gain = state->past_gain;
    for(i=0; i<l_trm; i++) {
        gain = g729_mult(gain, AGC_FAC);
        gain = g729_add(gain, g0);
        sig_out[i] = g729_extract_h(g729_L_shl(g729_L_mult(sig_out[i], gain), 3));
    }
    state->past_gain = gain;
    
    return;
}

