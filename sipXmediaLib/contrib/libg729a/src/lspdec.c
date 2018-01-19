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
#include "ld8a.h"
#include "basic_op.h"
#include "tab_ld8a.h"

#include "g729a_decoder.h"

/* static memory */

static G729_Word16 freq_prev_reset[M] = { /* Q13 */
    2339, 4679, 7018, 9358, 11698, 14037, 16377, 18717, 21056, 23396
};     /* PI*(float)(j+1)/(float)(M+1) */

static void g729_Lsp_iqua_cs(g729a_lspdec_state * state, G729_Word16 prm[], G729_Word16 lsp_q[], G729_Word16 erase);

/*----------------------------------------------------------------------------
 * g729_Lsp_decw_reset -   set the previous LSP vectors
 *----------------------------------------------------------------------------
 */
void g729_Lsp_decw_reset(g729a_lspdec_state * state)
{
    G729_Word16 i;
    
    for(i=0; i<MA_NP; i++)
        g729_Copy( &freq_prev_reset[0], &(state->freq_prev[i][0]), M );
    
    state->prev_ma = 0;
    
    g729_Copy( freq_prev_reset, state->prev_lsp, M);
}

/*----------------------------------------------------------------------------
 * g729_Lsp_iqua_cs -  LSP main quantization routine
 *----------------------------------------------------------------------------
 */
static void g729_Lsp_iqua_cs(
    g729a_lspdec_state * state,
    G729_Word16 prm[],          /* (i)     : indexes of the selected LSP */
    G729_Word16 lsp_q[],        /* (o) Q13 : Quantized LSP parameters    */
    G729_Word16 erase           /* (i)     : frame erase information     */
)
{
    G729_Word16 mode_index;
    G729_Word16 code0;
    G729_Word16 code1;
    G729_Word16 code2;
    G729_Word16 buf[M];     /* Q13 */
    
    if( erase==0 ) {  /* Not frame erasure */
        mode_index = g729_shr(prm[0] ,NC0_B) & (G729_Word16)1;
        code0 = prm[0] & (G729_Word16)(NC0 - 1);
        code1 = g729_shr(prm[1] ,NC1_B) & (G729_Word16)(NC1 - 1);
        code2 = prm[1] & (G729_Word16)(NC1 - 1);
        
        /* compose quantized LSP (lsp_q) from indexes */
        
        g729_Lsp_get_quant(g729_lspcb1, g729_lspcb2, code0, code1, code2,
                           g729_fg[mode_index], state->freq_prev, lsp_q, g729_fg_sum[mode_index]);
        
        /* save parameters to use in case of the frame erased situation */
        
        g729_Copy(lsp_q, state->prev_lsp, M);
        state->prev_ma = mode_index;
    }
    else {           /* Frame erased */
        /* use revious LSP */
        
        g729_Copy(state->prev_lsp, lsp_q, M);
        
        /* update freq_prev */
        
        g729_Lsp_prev_extract(state->prev_lsp, buf,
                              g729_fg[state->prev_ma], state->freq_prev, g729_fg_sum_inv[state->prev_ma]);
        g729_Lsp_prev_update(buf, state->freq_prev);
    }
    
    return;
}

/*-------------------------------------------------------------------*
 * Function  g729_D_lsp:                                             *
 *           ~~~~~~                                                  *
 *-------------------------------------------------------------------*/

void g729_D_lsp(
    g729a_lspdec_state * state,
    G729_Word16 prm[],          /* (i)     : indexes of the selected LSP */
    G729_Word16 lsp_q[],        /* (o) Q15 : Quantized LSP parameters    */
    G729_Word16 erase           /* (i)     : frame erase information     */
)
{
    G729_Word16 lsf_q[M];       /* domain 0.0<= lsf_q <PI in Q13 */
    
    
    g729_Lsp_iqua_cs(state, prm, lsf_q, erase);
    
    /* Convert LSFs to LSPs */
    
    g729_Lsf_lsp2(lsf_q, lsp_q, M);
    
    return;
}

