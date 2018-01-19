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

/*---------------------------------------------------------------------------*
 *  g729_Gain_predict()        : make gcode0(exp_gcode0)                     *
 *  g729_Gain_update()         : update table of past quantized energies.    *
 *  g729_Gain_update_erasure() : update table of past quantized energies.    *
 *                                                        (frame erasure)    *
 *    This function is used both Coder and Decoder.                          *
 *---------------------------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"
#include "tab_ld8a.h"
#include "oper_32b.h"

/*---------------------------------------------------------------------------*
 * Function  g729_Gain_predict                                               *
 * ~~~~~~~~~~~~~~~~~~~~~~                                                    *
 * MA prediction is performed on the innovation energy (in dB with mean      *
 * removed).                                                                 *
 *---------------------------------------------------------------------------*/
void g729_Gain_predict(
                       G729_Word16 past_qua_en[], /* (i) Q10 :Past quantized energies        */
                       G729_Word16 code[],        /* (i) Q13 :Innovative vector.             */
                       G729_Word16 L_subfr,       /* (i)     :Subframe length.               */
                       G729_Word16 *gcode0,       /* (o) Qxx :Predicted codebook gain        */
                       G729_Word16 *exp_gcode0    /* (o)     :Q-Format(gcode0)               */
)
{
    G729_Word16  i, exp, frac;
    G729_Word32  L_tmp;
    
    
    
    /*-------------------------------*
     * Energy coming from code       *
     *-------------------------------*/
    
    L_tmp = 0;
    for(i=0; i<L_subfr; i++)
        L_tmp = g729_L_mac(L_tmp, code[i], code[i]);
    
    /*-----------------------------------------------------------------*
     *  Compute: means_ener - 10log10(ener_code/ L_sufr)               *
     *  Note: mean_ener change from 36 dB to 30 dB because input/2     *
     *                                                                 *
     * = 30.0 - 10 log10( ener_code / lcode)  + 10log10(2^27)          *
     *                                          !!ener_code in Q27!!   *
     * = 30.0 - 3.0103 * g729_Log2(ener_code) + 10log10(40) + 10log10(2^27) *
     * = 30.0 - 3.0103 * g729_Log2(ener_code) + 16.02  + 81.278             *
     * = 127.298 - 3.0103 * g729_Log2(ener_code)                            *
     *-----------------------------------------------------------------*/
    
    g729_Log2(L_tmp, &exp, &frac);               /* Q27->Q0 ^Q0 ^Q15       */
    L_tmp = g729_Mpy_32_16(exp, frac, -24660);   /* Q0 Q15 Q13 -> ^Q14     */
    /* hi:Q0+Q13+1            */
    /* lo:Q15+Q13-15+1        */
    /* -24660[Q13]=-3.0103    */
    L_tmp = g729_L_mac(L_tmp, 32588, 32);        /* 32588*32[Q14]=127.298  */
    
    /*-----------------------------------------------------------------*
     * Compute gcode0.                                                 *
     *  = Sum(i=0,3) g729_pred[i]*past_qua_en[i] - ener_code + mean_ener    *
     *-----------------------------------------------------------------*/
    
    L_tmp = g729_L_shl(L_tmp, 10);                      /* From Q14 to Q24 */
    for(i=0; i<4; i++)
        L_tmp = g729_L_mac(L_tmp, g729_pred[i], past_qua_en[i]); /* Q13*Q10 ->Q24 */
    
    *gcode0 = g729_extract_h(L_tmp);                    /* From Q24 to Q8  */
    
    /*-----------------------------------------------------------------*
     * gcode0 = pow(10.0, gcode0/20)                                   *
     *        = pow(2, 3.3219*gcode0/20)                               *
     *        = pow(2, 0.166*gcode0)                                   *
     *-----------------------------------------------------------------*/
    
    L_tmp = g729_L_mult(*gcode0, 5439);       /* *0.166 in Q15, result in Q24*/
    L_tmp = g729_L_shr(L_tmp, 8);             /* From Q24 to Q16             */
    g729_L_Extract(L_tmp, &exp, &frac);       /* Extract exponent of gcode0  */
    
    *gcode0 = g729_extract_l(g729_Pow2(14, frac)); /* Put 14 as exponent so that  */
    /* output of g729_Pow2() will be:   */
    /* 16768 < g729_Pow2() <= 32767     */
    *exp_gcode0 = g729_sub(14,exp);
}


/*---------------------------------------------------------------------------*
 * Function  g729_Gain_update                                                *
 * ~~~~~~~~~~~~~~~~~~~~~~                                                    *
 * update table of past quantized energies                                   *
 *---------------------------------------------------------------------------*/
void g729_Gain_update(
                      G729_Word16 past_qua_en[],   /* (io) Q10 :Past quantized energies           */
                      G729_Word32  L_gbk12         /* (i) Q13 : g729_gbk1[indice1][1]+g729_gbk2[indice2][1] */
)
{
    G729_Word16  i, tmp;
    G729_Word16  exp, frac;
    G729_Word32  L_acc;
    
    
    
    for(i=3; i>0; i--){
        past_qua_en[i] = past_qua_en[i-1];         /* Q10 */
    }
    /*----------------------------------------------------------------------*
     * -- past_qua_en[0] = 20*log10(g729_gbk1[index1][1]+g729_gbk2[index2][1]); --    *
     *    2 * 10 log10( g729_gbk1[index1][1]+g729_gbk2[index2][1] )                   *
     *  = 2 * 3.0103 g729_Log2( g729_gbk1[index1][1]+g729_gbk2[index2][1] )                *
     *  = 2 * 3.0103 g729_Log2( g729_gbk1[index1][1]+g729_gbk2[index2][1] )                *
     *                                                 24660:Q12(6.0205)    *
     *----------------------------------------------------------------------*/
    
    g729_Log2( L_gbk12, &exp, &frac );               /* L_gbk12:Q13       */
    L_acc = g729_L_Comp( g729_sub(exp, 13), frac);         /* L_acc:Q16           */
    tmp = g729_extract_h( g729_L_shl( L_acc, 13 ) );       /* tmp:Q13           */
    past_qua_en[0] = g729_mult( tmp, 24660 );        /* past_qua_en[]:Q10 */
}


/*---------------------------------------------------------------------------*
 * Function  g729_Gain_update_erasure                                        *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                             *
 * update table of past quantized energies (frame erasure)                   *
 *---------------------------------------------------------------------------*
 *     av_pred_en = 0.0;                                                     *
 *     for (i = 0; i < 4; i++)                                               *
 *        av_pred_en += past_qua_en[i];                                      *
 *     av_pred_en = av_pred_en*0.25 - 4.0;                                   *
 *     if (av_pred_en < -14.0) av_pred_en = -14.0;                           *
 *---------------------------------------------------------------------------*/
void g729_Gain_update_erasure(
                              G729_Word16 past_qua_en[]     /* (i) Q10 :Past quantized energies        */
)
{
    G729_Word16  i, av_pred_en;
    G729_Word32  L_tmp;
    
    
    
    L_tmp = 0;                                                     /* Q10 */
    for(i=0; i<4; i++)
        L_tmp = g729_L_add( L_tmp, g729_L_deposit_l( past_qua_en[i] ) );
    av_pred_en = g729_extract_l( g729_L_shr( L_tmp, 2 ) );
    av_pred_en = g729_sub( av_pred_en, 4096 );                          /* Q10 */
    
    if( g729_sub(av_pred_en, -14336) < 0 ){
        av_pred_en = -14336;                              /* 14336:14[Q10] */
    }
    
    for(i=3; i>0; i--){
        past_qua_en[i] = past_qua_en[i-1];
    }
    past_qua_en[0] = av_pred_en;
}

