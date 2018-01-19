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

#include "g729a_decoder.h"

/*---------------------------------------------------------------------------*
 * Function  g729_Dec_gain                                                   *
 * ~~~~~~~~~~~~~~~~~~                                                        *
 * Decode the pitch and codebook gains                                       *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * input arguments:                                                          *
 *                                                                           *
 *   index      :Quantization index                                          *
 *   code[]     :Innovative code vector                                      *
 *   L_subfr    :Subframe size                                               *
 *   bfi        :Bad frame indicator                                         *
 *                                                                           *
 * output arguments:                                                         *
 *                                                                           *
 *   gain_pit   :Quantized pitch gain                                        *
 *   gain_cod   :Quantized codebook gain                                     *
 *                                                                           *
 *---------------------------------------------------------------------------*/
void g729_Dec_gain(
    g729a_decoder_state * state,
    G729_Word16 index,        /* (i)     :Index of quantization.         */
    G729_Word16 code[],       /* (i) Q13 :Innovative vector.             */
    G729_Word16 L_subfr,      /* (i)     :Subframe length.               */
    G729_Word16 bfi,          /* (i)     :Bad frame indicator            */
    G729_Word16 *gain_pit,    /* (o) Q14 :Pitch gain.                    */
    G729_Word16 *gain_cod     /* (o) Q1  :Code gain.                     */
)
{
    G729_Word16  index1, index2, tmp;
    G729_Word16  gcode0, exp_gcode0;
    G729_Word32  L_gbk12, L_acc, L_accb;
    
    /* Gain predictor, Past quantized energies = -14.0 in Q10 */
    
    /* static G729_Word16 past_qua_en[4] = { -14336, -14336, -14336, -14336 }; */
    
    
    /*-------------- Case of erasure. ---------------*/
    
    if(bfi != 0){
        *gain_pit = g729_mult( *gain_pit, 29491 );      /* *0.9 in Q15 */
        if (g729_sub( *gain_pit, 29491) > 0) *gain_pit = 29491;
        *gain_cod = g729_mult( *gain_cod, 32111 );      /* *0.98 in Q15 */
        
        /*----------------------------------------------*
         * update table of past quantized energies      *
         *                              (frame erasure) *
         *----------------------------------------------*/
        g729_Gain_update_erasure(state->past_qua_en);
        
        return;
    }
    
    /*-------------- Decode pitch gain ---------------*/
    
    index1 = g729_imap1[ g729_shr(index,NCODE2_B) ] ;
    index2 = g729_imap2[ index & (NCODE2-1) ] ;
    *gain_pit = g729_add( g729_gbk1[index1][0], g729_gbk2[index2][0] );
    
    /*-------------- Decode codebook gain ---------------*/
    
    /*---------------------------------------------------*
     *-  energy due to innovation                       -*
     *-  predicted energy                               -*
     *-  predicted codebook gain => gcode0[exp_gcode0]  -*
     *---------------------------------------------------*/
    
    g729_Gain_predict( state->past_qua_en, code, L_subfr, &gcode0, &exp_gcode0 );
    
    /*-----------------------------------------------------------------*
     * *gain_code = (g729_gbk1[indice1][1]+g729_gbk2[indice2][1]) * gcode0;      *
     *-----------------------------------------------------------------*/
    
    L_acc = g729_L_deposit_l( g729_gbk1[index1][1] );
    L_accb = g729_L_deposit_l( g729_gbk2[index2][1] );
    L_gbk12 = g729_L_add( L_acc, L_accb );                       /* Q13 */
    tmp = g729_extract_l( g729_L_shr( L_gbk12,1 ) );                  /* Q12 */
    L_acc = g729_L_mult(tmp, gcode0);             /* Q[exp_gcode0+12+1] */
    
    L_acc = g729_L_shl(L_acc, g729_add( g729_negate(exp_gcode0),(-12-1+1+16) ));
    *gain_cod = g729_extract_h( L_acc );                          /* Q1 */
    
    /*----------------------------------------------*
     * update table of past quantized energies      *
     *----------------------------------------------*/
    g729_Gain_update( state->past_qua_en, L_gbk12 );
    
    return;
    
}

