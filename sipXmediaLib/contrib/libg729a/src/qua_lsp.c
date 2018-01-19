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

/*-------------------------------------------------------------------*
 * Function  g729_Qua_lsp:                                           *
 *           ~~~~~~~~                                                *
 *-------------------------------------------------------------------*/
#include "g729a_typedef.h"
#include "basic_op.h"

#include "ld8a.h"
#include "tab_ld8a.h"

#include "g729a_encoder.h"

/* static memory */

static G729_Word16 freq_prev_reset[M] = {  /* Q13:previous LSP vector(init) */
    2339, 4679, 7018, 9358, 11698, 14037, 16377, 18717, 21056, 23396
};     /* PI*(float)(j+1)/(float)(M+1) */

/* declaration of static functions */

static void g729_Lsp_qua_cs(g729a_lspenc_state * state, G729_Word16 flsp_in[M], G729_Word16 lspq_out[M], G729_Word16 *code);

static void g729_Relspwed(
    G729_Word16 lsp[],                       /* Q13 */
    G729_Word16 wegt[],                      /* normalized */
    G729_Word16 lspq[],                      /* Q13 */
    G729_Word16 lspcb1[][M],                 /* Q13 */
    G729_Word16 lspcb2[][M],                 /* Q13 */
    G729_Word16 fg[MODE][MA_NP][M],          /* Q15 */
    G729_Word16 freq_prev[MA_NP][M],         /* Q13 */
    G729_Word16 fg_sum[MODE][M],             /* Q15 */
    G729_Word16 fg_sum_inv[MODE][M],         /* Q12 */
    G729_Word16 code_ana[]
);

static void g729_Lsp_pre_select(G729_Word16 rbuf[], G729_Word16 lspcb1[][M], G729_Word16 *cand);
static void g729_Lsp_select_1(G729_Word16 rbuf[], G729_Word16 lspcb1[], G729_Word16 wegt[], G729_Word16 lspcb2[][M], G729_Word16 *index);
static void g729_Lsp_select_2( G729_Word16 rbuf[], G729_Word16 lspcb1[], G729_Word16 wegt[], G729_Word16 lspcb2[][M], G729_Word16 *index);
static void g729_Lsp_get_tdist(G729_Word16 wegt[], G729_Word16 buf[], G729_Word32 *L_tdist, G729_Word16 rbuf[], G729_Word16 fg_sum[]);
static void g729_Lsp_last_select(G729_Word32 L_tdist[], G729_Word16 *mode_index);
static void g729_Get_wegt(G729_Word16 flsp[], G729_Word16 wegt[]);

/* declaration end */

void g729_Qua_lsp(
    g729a_lspenc_state * state,
    G729_Word16 lsp[],       /* (i) Q15 : Unquantized LSP            */
    G729_Word16 lsp_q[],     /* (o) Q15 : Quantized LSP              */
    G729_Word16 ana[]        /* (o)     : indexes                    */
)
{
    G729_Word16 lsf[M], lsf_q[M];  /* domain 0.0<= lsf <PI in Q13 */
    
    /* Convert LSPs to LSFs */
    g729_Lsp_lsf2(lsp, lsf, M);
    
    g729_Lsp_qua_cs(state, lsf, lsf_q, ana );
    
    /* Convert LSFs to LSPs */
    g729_Lsf_lsp2(lsf_q, lsp_q, M);
    
    return;
}

void g729_Lsp_encw_reset(g729a_lspenc_state * state)
{
    G729_Word16 i;
    
    for(i=0; i<MA_NP; i++)
        g729_Copy( &freq_prev_reset[0], &(state->freq_prev[i][0]), M );
}

/* implementation of static functions */

static void g729_Lsp_qua_cs(
    g729a_lspenc_state * state,
    G729_Word16 flsp_in[M],    /* (i) Q13 : Original LSP parameters    */
    G729_Word16 lspq_out[M],   /* (o) Q13 : Quantized LSP parameters   */
    G729_Word16 *code          /* (o)     : codes of the selected LSP  */
)
{
    G729_Word16 wegt[M];       /* Q11->normalized : weighting coefficients */
    
    g729_Get_wegt( flsp_in, wegt );
    
    g729_Relspwed( flsp_in, wegt, lspq_out, g729_lspcb1, g729_lspcb2, g729_fg,
                  state->freq_prev, g729_fg_sum, g729_fg_sum_inv, code);
}

static void g729_Relspwed(
    G729_Word16 lsp[],                 /* (i) Q13 : unquantized LSP parameters */
    G729_Word16 wegt[],                /* (i) norm: weighting coefficients     */
    G729_Word16 lspq[],                /* (o) Q13 : quantized LSP parameters   */
    G729_Word16 lspcb1[][M],           /* (i) Q13 : first stage LSP codebook   */
    G729_Word16 lspcb2[][M],           /* (i) Q13 : Second stage LSP codebook  */
    G729_Word16 fg[MODE][MA_NP][M],    /* (i) Q15 : MA prediction coefficients */
    G729_Word16 freq_prev[MA_NP][M],   /* (i) Q13 : previous LSP vector        */
    G729_Word16 fg_sum[MODE][M],       /* (i) Q15 : present MA prediction coef.*/
    G729_Word16 fg_sum_inv[MODE][M],   /* (i) Q12 : inverse coef.              */
    G729_Word16 code_ana[]             /* (o)     : codes of the selected LSP  */
)
{
    G729_Word16 mode, j;
    G729_Word16 index, mode_index;
    G729_Word16 cand[MODE], cand_cur;
    G729_Word16 tindex1[MODE], tindex2[MODE];
    G729_Word32 L_tdist[MODE];         /* Q26 */
    G729_Word16 rbuf[M];               /* Q13 */
    G729_Word16 buf[M];                /* Q13 */
    
    for(mode = 0; mode<MODE; mode++) {
        g729_Lsp_prev_extract(lsp, rbuf, fg[mode], freq_prev, fg_sum_inv[mode]);
        
        g729_Lsp_pre_select(rbuf, lspcb1, &cand_cur );
        cand[mode] = cand_cur;
        
        g729_Lsp_select_1(rbuf, lspcb1[cand_cur], wegt, lspcb2, &index);
        
        tindex1[mode] = index;
        
        for( j = 0 ; j < NC ; j++ )
            buf[j] = g729_add( lspcb1[cand_cur][j], lspcb2[index][j] );
        
        g729_Lsp_expand_1(buf, GAP1);
        
        g729_Lsp_select_2(rbuf, lspcb1[cand_cur], wegt, lspcb2, &index);
        
        tindex2[mode] = index;
        
        for( j = NC ; j < M ; j++ )
            buf[j] = g729_add( lspcb1[cand_cur][j], lspcb2[index][j] );
        
        g729_Lsp_expand_2(buf, GAP1);
        
        g729_Lsp_expand_1_2(buf, GAP2);
        
        g729_Lsp_get_tdist(wegt, buf, &L_tdist[mode], rbuf, fg_sum[mode]);
    }
    
    g729_Lsp_last_select(L_tdist, &mode_index);
    
    code_ana[0] = g729_shl( mode_index,NC0_B ) | cand[mode_index];
    code_ana[1] = g729_shl( tindex1[mode_index],NC1_B ) | tindex2[mode_index];
    
    g729_Lsp_get_quant(lspcb1, lspcb2, cand[mode_index],
                       tindex1[mode_index], tindex2[mode_index],
                       fg[mode_index], freq_prev, lspq, fg_sum[mode_index]) ;
    
    return;
}


static void g729_Lsp_pre_select(
    G729_Word16 rbuf[],              /* (i) Q13 : target vetor             */
    G729_Word16 lspcb1[][M],         /* (i) Q13 : first stage LSP codebook */
    G729_Word16 *cand                /* (o)     : selected code            */
)
{
    G729_Word16 i, j;
    G729_Word16 tmp;                 /* Q13 */
    G729_Word32 L_dmin;              /* Q26 */
    G729_Word32 L_tmp;               /* Q26 */
    G729_Word32 L_temp;
    
    /* avoid the worst case. (all over flow) */
    
    *cand = 0;
    L_dmin = G729A_MAX_32;
    for ( i = 0 ; i < NC0 ; i++ ) {
        L_tmp = 0;
        for ( j = 0 ; j < M ; j++ ) {
            tmp = g729_sub(rbuf[j], lspcb1[i][j]);
            L_tmp = g729_L_mac( L_tmp, tmp, tmp );
        }
        
        L_temp = g729_L_sub(L_tmp,L_dmin);
        if (  L_temp< 0L) {
            L_dmin = L_tmp;
            *cand = i;
        }
    }
    return;
}

static void g729_Lsp_select_1(
    G729_Word16 rbuf[],                   /* (i) Q13 : target vector             */
    G729_Word16 lspcb1[],            /* (i) Q13 : first stage lsp codebook  */
    G729_Word16 wegt[],                   /* (i) norm: weighting coefficients    */
    G729_Word16 lspcb2[][M],         /* (i) Q13 : second stage lsp codebook */
    G729_Word16 *index                    /* (o)     : selected codebook index   */
)
{
    G729_Word16 j, k1;
    G729_Word16 buf[M];              /* Q13 */
    G729_Word32 L_dist;              /* Q26 */
    G729_Word32 L_dmin;              /* Q26 */
    G729_Word16 tmp,tmp2;            /* Q13 */
    G729_Word32 L_temp;
    
    for ( j = 0 ; j < NC ; j++ )
        buf[j] = g729_sub(rbuf[j], lspcb1[j]);
    
    /* avoid the worst case. (all over flow) */
    *index = 0;
    L_dmin = G729A_MAX_32;
    for ( k1 = 0 ; k1 < NC1 ; k1++ ) {
        L_dist = 0;
        for ( j = 0 ; j < NC ; j++ ) {
            tmp = g729_sub(buf[j], lspcb2[k1][j]);
            tmp2 = g729_mult( wegt[j], tmp );
            L_dist = g729_L_mac( L_dist, tmp2, tmp );
        }
        
        L_temp =g729_L_sub(L_dist,L_dmin);
        if ( L_temp <0L ) {
            L_dmin = L_dist;
            *index = k1;
        }
    }
    return;
}

static void g729_Lsp_select_2(
    G729_Word16 rbuf[],              /* (i) Q13 : target vector             */
    G729_Word16 lspcb1[],            /* (i) Q13 : first stage lsp codebook  */
    G729_Word16 wegt[],              /* (i) norm: weighting coef.           */
    G729_Word16 lspcb2[][M],         /* (i) Q13 : second stage lsp codebook */
    G729_Word16 *index               /* (o)     : selected codebook index   */
)
{
    G729_Word16 j, k1;
    G729_Word16 buf[M];              /* Q13 */
    G729_Word32 L_dist;              /* Q26 */
    G729_Word32 L_dmin;              /* Q26 */
    G729_Word16 tmp,tmp2;            /* Q13 */
    G729_Word32 L_temp;
    
    for ( j = NC ; j < M ; j++ )
        buf[j] = g729_sub(rbuf[j], lspcb1[j]);
    
    /* avoid the worst case. (all over flow) */
    *index = 0;
    L_dmin = G729A_MAX_32;
    for ( k1 = 0 ; k1 < NC1 ; k1++ ) {
        L_dist = 0;
        for ( j = NC ; j < M ; j++ ) {
            tmp = g729_sub(buf[j], lspcb2[k1][j]);
            tmp2 = g729_mult( wegt[j], tmp );
            L_dist = g729_L_mac( L_dist, tmp2, tmp );
        }
        
        L_temp = g729_L_sub(L_dist, L_dmin);
        if ( L_temp <0L ) {
            L_dmin = L_dist;
            *index = k1;
        }
    }
    return;
}

static void g729_Lsp_get_tdist(
    G729_Word16 wegt[],          /* (i) norm: weight coef.                */
    G729_Word16 buf[],           /* (i) Q13 : candidate LSP vector        */
    G729_Word32 *L_tdist,        /* (o) Q27 : distortion                  */
    G729_Word16 rbuf[],          /* (i) Q13 : target vector               */
    G729_Word16 fg_sum[]         /* (i) Q15 : present MA prediction coef. */
)
{
    G729_Word16 j;
    G729_Word16 tmp, tmp2;     /* Q13 */
    G729_Word32 L_acc;         /* Q25 */
    
    *L_tdist = 0;
    for ( j = 0 ; j < M ; j++ ) {
        /* tmp = (buf - rbuf)*fg_sum */
        tmp = g729_sub( buf[j], rbuf[j] );
        tmp = g729_mult( tmp, fg_sum[j] );
        
        /* *L_tdist += wegt * tmp * tmp */
        L_acc = g729_L_mult( wegt[j], tmp );
        tmp2 = g729_extract_h( g729_L_shl( L_acc, 4 ) );
        *L_tdist = g729_L_mac( *L_tdist, tmp2, tmp );
    }
    
    return;
}

static void g729_Lsp_last_select(
    G729_Word32 L_tdist[],     /* (i) Q27 : distortion         */
    G729_Word16 *mode_index    /* (o)     : the selected mode  */
)
{
    G729_Word32 L_temp;
    *mode_index = 0;
    L_temp =g729_L_sub(L_tdist[1] ,L_tdist[0]);
    if (  L_temp<0L){
        *mode_index = 1;
    }
    return;
}

static void g729_Get_wegt(
    G729_Word16 flsp[],    /* (i) Q13 : M LSP parameters  */
    G729_Word16 wegt[]     /* (o) Q11->norm : M weighting coefficients */
)
{
    G729_Word16 i;
    G729_Word16 tmp;
    G729_Word32 L_acc;
    G729_Word16 sft;
    G729_Word16 buf[M]; /* in Q13 */
    
    
    buf[0] = g729_sub( flsp[1], (PI04+8192) );           /* 8192:1.0(Q13) */
    
    for ( i = 1 ; i < M-1 ; i++ ) {
        tmp = g729_sub( flsp[i+1], flsp[i-1] );
        buf[i] = g729_sub( tmp, 8192 );
    }
    
    buf[M-1] = g729_sub( (PI92-8192), flsp[M-2] );
    
    /* */
    for ( i = 0 ; i < M ; i++ ) {
        if ( buf[i] > 0 ){
            wegt[i] = 2048;                    /* 2048:1.0(Q11) */
        }
        else {
            L_acc = g729_L_mult( buf[i], buf[i] );           /* L_acc in Q27 */
            tmp = g729_extract_h( g729_L_shl( L_acc, 2 ) );       /* tmp in Q13 */
            
            L_acc = g729_L_mult( tmp, CONST10 );             /* L_acc in Q25 */
            tmp = g729_extract_h( g729_L_shl( L_acc, 2 ) );       /* tmp in Q11 */
            
            wegt[i] = g729_add( tmp, 2048 );                 /* wegt in Q11 */
        }
    }
    
    /* */
    L_acc = g729_L_mult( wegt[4], CONST12 );             /* L_acc in Q26 */
    wegt[4] = g729_extract_h( g729_L_shl( L_acc, 1 ) );       /* wegt in Q11 */
    
    L_acc = g729_L_mult( wegt[5], CONST12 );             /* L_acc in Q26 */
    wegt[5] = g729_extract_h( g729_L_shl( L_acc, 1 ) );       /* wegt in Q11 */
    
    /* wegt: Q11 -> normalized */
    tmp = 0;
    for ( i = 0; i < M; i++ ) {
        if ( g729_sub(wegt[i], tmp) > 0 ) {
            tmp = wegt[i];
        }
    }
    
    sft = g729_norm_s(tmp);
    for ( i = 0; i < M; i++ ) {
        wegt[i] = g729_shl(wegt[i], sft);                  /* wegt in Q(11+sft) */
    }
    
    return;
}

