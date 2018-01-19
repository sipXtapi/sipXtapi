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

#include <stdio.h>
#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"


void g729_Lsp_get_quant(
  G729_Word16 lspcb1[][M],      /* (i) Q13 : first stage LSP codebook      */
  G729_Word16 lspcb2[][M],      /* (i) Q13 : Second stage LSP codebook     */
  G729_Word16 code0,            /* (i)     : selected code of first stage  */
  G729_Word16 code1,            /* (i)     : selected code of second stage */
  G729_Word16 code2,            /* (i)     : selected code of second stage */
  G729_Word16 fg[][M],          /* (i) Q15 : MA prediction coef.           */
  G729_Word16 freq_prev[][M],   /* (i) Q13 : previous LSP vector           */
  G729_Word16 lspq[],           /* (o) Q13 : quantized LSP parameters      */
  G729_Word16 fg_sum[]          /* (i) Q15 : present MA prediction coef.   */
)
{
  G729_Word16 j;
  G729_Word16 buf[M];           /* Q13 */


  for ( j = 0 ; j < NC ; j++ )
    buf[j] = g729_add( lspcb1[code0][j], lspcb2[code1][j] );

  for ( j = NC ; j < M ; j++ )
    buf[j] = g729_add( lspcb1[code0][j], lspcb2[code2][j] );

  g729_Lsp_expand_1_2(buf, GAP1);
  g729_Lsp_expand_1_2(buf, GAP2);

  g729_Lsp_prev_compose(buf, lspq, fg, freq_prev, fg_sum);

  g729_Lsp_prev_update(buf, freq_prev);

  g729_Lsp_stability( lspq );

  return;
}


void g729_Lsp_expand_1(
  G729_Word16 buf[],        /* (i/o) Q13 : LSP vectors */
  G729_Word16 gap           /* (i)   Q13 : gap         */
)
{
  G729_Word16 j, tmp;
  G729_Word16 diff;        /* Q13 */

  for ( j = 1 ; j < NC ; j++ ) {
    diff = g729_sub( buf[j-1], buf[j] );
    tmp = g729_shr( g729_add( diff, gap), 1 );

    if ( tmp >  0 ) {
      buf[j-1] = g729_sub( buf[j-1], tmp );
      buf[j]   = g729_add( buf[j], tmp );
    }
  }
  return;
}


void g729_Lsp_expand_2(
  G729_Word16 buf[],       /* (i/o) Q13 : LSP vectors */
  G729_Word16 gap          /* (i)   Q13 : gap         */
)
{
  G729_Word16 j, tmp;
  G729_Word16 diff;        /* Q13 */

  for ( j = NC ; j < M ; j++ ) {
    diff = g729_sub( buf[j-1], buf[j] );
    tmp = g729_shr( g729_add( diff, gap), 1 );

    if ( tmp > 0 ) {
      buf[j-1] = g729_sub( buf[j-1], tmp );
      buf[j]   = g729_add( buf[j], tmp );
    }
  }
  return;
}


void g729_Lsp_expand_1_2(
  G729_Word16 buf[],       /* (i/o) Q13 : LSP vectors */
  G729_Word16 gap          /* (i)   Q13 : gap         */
)
{
  G729_Word16 j, tmp;
  G729_Word16 diff;        /* Q13 */

  for ( j = 1 ; j < M ; j++ ) {
    diff = g729_sub( buf[j-1], buf[j] );
    tmp = g729_shr( g729_add( diff, gap), 1 );

    if ( tmp > 0 ) {
      buf[j-1] = g729_sub( buf[j-1], tmp );
      buf[j]   = g729_add( buf[j], tmp );
    }
  }
  return;
}


/*
  Functions which use previous LSP parameter (freq_prev).
*/


/*
  compose LSP parameter from elementary LSP with previous LSP.
*/
void g729_Lsp_prev_compose(
  G729_Word16 lsp_ele[],             /* (i) Q13 : LSP vectors                 */
  G729_Word16 lsp[],                 /* (o) Q13 : quantized LSP parameters    */
  G729_Word16 fg[][M],               /* (i) Q15 : MA prediction coef.         */
  G729_Word16 freq_prev[][M],        /* (i) Q13 : previous LSP vector         */
  G729_Word16 fg_sum[]               /* (i) Q15 : present MA prediction coef. */
)
{
  G729_Word16 j, k;
  G729_Word32 L_acc;                 /* Q29 */

  for ( j = 0 ; j < M ; j++ ) {
    L_acc = g729_L_mult( lsp_ele[j], fg_sum[j] );
    for ( k = 0 ; k < MA_NP ; k++ )
      L_acc = g729_L_mac( L_acc, freq_prev[k][j], fg[k][j] );

    lsp[j] = g729_extract_h(L_acc);
  }
  return;
}


/*
  extract elementary LSP from composed LSP with previous LSP
*/
void g729_Lsp_prev_extract(
  G729_Word16 lsp[M],                /* (i) Q13 : unquantized LSP parameters  */
  G729_Word16 lsp_ele[M],            /* (o) Q13 : target vector               */
  G729_Word16 fg[MA_NP][M],          /* (i) Q15 : MA prediction coef.         */
  G729_Word16 freq_prev[MA_NP][M],   /* (i) Q13 : previous LSP vector         */
  G729_Word16 fg_sum_inv[M]          /* (i) Q12 : inverse previous LSP vector */
)
{
  G729_Word16 j, k;
  G729_Word32 L_temp;                /* Q19 */
  G729_Word16 temp;                  /* Q13 */


  for ( j = 0 ; j < M ; j++ ) {
    L_temp = g729_L_deposit_h(lsp[j]);
    for ( k = 0 ; k < MA_NP ; k++ )
      L_temp = g729_L_msu( L_temp, freq_prev[k][j], fg[k][j] );

    temp = g729_extract_h(L_temp);
    L_temp = g729_L_mult( temp, fg_sum_inv[j] );
    lsp_ele[j] = g729_extract_h( g729_L_shl( L_temp, 3 ) );

  }
  return;
}


/*
  update previous LSP parameter
*/
void g729_Lsp_prev_update(
  G729_Word16 lsp_ele[M],             /* (i)   Q13 : LSP vectors           */
  G729_Word16 freq_prev[MA_NP][M]     /* (i/o) Q13 : previous LSP vectors  */
)
{
  G729_Word16 k;

  for ( k = MA_NP-1 ; k > 0 ; k-- )
    g729_Copy(freq_prev[k-1], freq_prev[k], M);

  g729_Copy(lsp_ele, freq_prev[0], M);
  return;
}

void g729_Lsp_stability(
  G729_Word16 buf[]       /* (i/o) Q13 : quantized LSP parameters      */
)
{
  G729_Word16 j;
  G729_Word16 tmp;
  G729_Word32 L_diff;
  G729_Word32 L_acc, L_accb;

  for(j=0; j<M-1; j++) {
    L_acc = g729_L_deposit_l( buf[j+1] );
    L_accb = g729_L_deposit_l( buf[j] );
    L_diff = g729_L_sub( L_acc, L_accb );

    if( L_diff < 0L ) {
      /* exchange buf[j]<->buf[j+1] */
      tmp      = buf[j+1];
      buf[j+1] = buf[j];
      buf[j]   = tmp;
    }
  }


  if( g729_sub(buf[0], L_LIMIT) <0 ) {
    buf[0] = L_LIMIT;
    printf("lsp_stability warning Low \n");
  }
  for(j=0; j<M-1; j++) {
    L_acc = g729_L_deposit_l( buf[j+1] );
    L_accb = g729_L_deposit_l( buf[j] );
    L_diff = g729_L_sub( L_acc, L_accb );

    if( g729_L_sub(L_diff, GAP3)<0L ) {
      buf[j+1] = g729_add( buf[j], GAP3 );
    }
  }

  if( g729_sub(buf[M-1],M_LIMIT)>0 ) {
    buf[M-1] = M_LIMIT;
    printf("lsp_stability warning High \n");
  }
  return;
}

