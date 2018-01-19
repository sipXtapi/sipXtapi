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

/* Functions g729_Corr_xy2() and Cor_h_x()   */

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"

/*---------------------------------------------------------------------------*
 * Function g729_Corr_xy2()                                                       *
 * ~~~~~~~~~~~~~~~~~~~                                                       *
 * Find the correlations between the target xn[], the filtered adaptive      *
 * codebook excitation y1[], and the filtered 1st codebook innovation y2[].  *
 *   g_coeff[2]:exp_g_coeff[2] = <y2,y2>                                     *
 *   g_coeff[3]:exp_g_coeff[3] = -2<xn,y2>                                   *
 *   g_coeff[4]:exp_g_coeff[4] = 2<y1,y2>                                    *
 *---------------------------------------------------------------------------*/

void g729_Corr_xy2(
      G729_Word16 xn[],           /* (i) Q0  :Target vector.                  */
      G729_Word16 y1[],           /* (i) Q0  :Adaptive codebook.              */
      G729_Word16 y2[],           /* (i) Q12 :Filtered innovative vector.     */
      G729_Word16 g_coeff[],      /* (o) Q[exp]:Correlations between xn,y1,y2 */
      G729_Word16 exp_g_coeff[]   /* (o)       :Q-format of g_coeff[]         */
)
{
      G729_Word16   i,exp;
      G729_Word16   exp_y2y2,exp_xny2,exp_y1y2;
      G729_Word16   y2y2,    xny2,    y1y2;
      G729_Word32   L_acc;
      G729_Word16   scaled_y2[L_SUBFR];       /* Q9 */

      /*------------------------------------------------------------------*
       * Scale down y2[] from Q12 to Q9 to avoid overflow                 *
       *------------------------------------------------------------------*/
      for(i=0; i<L_SUBFR; i++) {
         scaled_y2[i] = g729_shr(y2[i], 3);        }

      /* Compute scalar product <y2[],y2[]> */
      L_acc = 1;                       /* Avoid case of all zeros */
      for(i=0; i<L_SUBFR; i++)
         L_acc = g729_L_mac(L_acc, scaled_y2[i], scaled_y2[i]);    /* L_acc:Q19 */

      exp      = g729_norm_l(L_acc);
      y2y2     = g729_round( g729_L_shl(L_acc, exp) );
      exp_y2y2 = g729_add(exp, 19-16);                          /* Q[19+exp-16] */

      g_coeff[2]     = y2y2;
      exp_g_coeff[2] = exp_y2y2;

      /* Compute scalar product <xn[],y2[]> */
      L_acc = 1;                       /* Avoid case of all zeros */
      for(i=0; i<L_SUBFR; i++)
         L_acc = g729_L_mac(L_acc, xn[i], scaled_y2[i]);           /* L_acc:Q10 */

      exp      = g729_norm_l(L_acc);
      xny2     = g729_round( g729_L_shl(L_acc, exp) );
      exp_xny2 = g729_add(exp, 10-16);                          /* Q[10+exp-16] */

      g_coeff[3]     = g729_negate(xny2);
      exp_g_coeff[3] = g729_sub(exp_xny2,1);                   /* -2<xn,y2> */

      /* Compute scalar product <y1[],y2[]> */
      L_acc = 1;                       /* Avoid case of all zeros */
      for(i=0; i<L_SUBFR; i++)
         L_acc = g729_L_mac(L_acc, y1[i], scaled_y2[i]);           /* L_acc:Q10 */

      exp      = g729_norm_l(L_acc);
      y1y2     = g729_round( g729_L_shl(L_acc, exp) );
      exp_y1y2 = g729_add(exp, 10-16);                          /* Q[10+exp-16] */

      g_coeff[4]     = y1y2;
      exp_g_coeff[4] = g729_sub(exp_y1y2,1);    ;                /* 2<y1,y2> */

      return;
}


/*--------------------------------------------------------------------------*
 *  Function  g729_Cor_h_X()                                                     *
 *  ~~~~~~~~~~~~~~~~~~~                                                     *
 * Compute correlations of input response h[] with the target vector X[].   *
 *--------------------------------------------------------------------------*/

void g729_Cor_h_X(
     G729_Word16 h[],        /* (i) Q12 :Impulse response of filters      */
     G729_Word16 X[],        /* (i)     :Target vector                    */
     G729_Word16 D[]         /* (o)     :Correlations between h[] and D[] */
                        /*          Normalized to 13 bits            */
)
{
   G729_Word16 i, j;
   G729_Word32 s, max, L_temp;
   G729_Word32 y32[L_SUBFR];

   /* first keep the result on 32 bits and find absolute maximum */

   max = 0;

   for (i = 0; i < L_SUBFR; i++)
   {
     s = 0;
     for (j = i; j <  L_SUBFR; j++)
       s = g729_L_mac(s, X[j], h[j-i]);

     y32[i] = s;

     s = g729_L_abs(s);
     L_temp =g729_L_sub(s,max);
     if(L_temp>0L) {
        max = s;
     }
   }

   /* Find the number of right shifts to do on y32[]  */
   /* so that maximum is on 13 bits                   */

   j = g729_norm_l(max);
   if( g729_sub(j,16) > 0) {
    j = 16;
   }

   j = g729_sub(18, j);

   for(i=0; i<L_SUBFR; i++) {
     D[i] = g729_extract_l( g729_L_shr(y32[i], j) );
   }

   return;

}

