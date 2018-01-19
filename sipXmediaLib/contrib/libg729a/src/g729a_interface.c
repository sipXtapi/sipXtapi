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

#include "g729a_interface.h"

#include "g729a_encoder.h"
#include "g729a_decoder.h"

#include "ld8a.h"

/*---------------------------------------------*
 * Encoder functions                           *
 *---------------------------------------------*/

G729_UWord32 G729A_Encoder_Get_Size()
{
    return sizeof(g729a_encoder_state);
}

G729_Word32 G729A_Encoder_Init(G729A_Enc_state encState)
{
    g729a_encoder_state * state;
    if ( NULL == encState ) return -1;
    
    state = (g729a_encoder_state *)encState;
    
    g729_Init_Pre_Process(&(state->pre_process_state));
    g729_Init_Coder_ld8a(state);
    
    return 0;
}

G729_Word32 G729A_Encoder_Process(G729A_Enc_state encState, G729_Word16 * speechIn, G729_UWord8 * outData)
{
    g729a_encoder_state * state;
    G729_Word16 prm[PRM_SIZE];  /* Analysis parameters. */
    
    if ( NULL == encState ) return -1;
    
    state = (g729a_encoder_state *)encState;
    
    g729_Pre_Process(&(state->pre_process_state), speechIn, state->new_speech, L_FRAME);
    g729_Coder_ld8a(state, prm);
    g729_prm2bits_ld8k_compressed(prm, outData);
    
    return 0;
}

G729_Word32 G729A_Encoder_Get_Error(G729A_Enc_state encState)
{
    g729a_encoder_state * state;
    if ( NULL == encState ) return -1;
    
    state = (g729a_encoder_state *)encState;
    
    return state->error;
}

/*---------------------------------------------*
 * Decoder functions                           *
 *---------------------------------------------*/

G729_UWord32 G729A_Decoder_Get_Size()
{
    return sizeof(g729a_decoder_state);
}

G729_Word32 G729A_Decoder_Init(G729A_Enc_state decState)
{
    g729a_decoder_state * state;
    if ( NULL == decState ) return -1;
    
    state = (g729a_decoder_state *)decState;
    
    g729_Set_zero(state->synth_buf, M);
    state->synth = state->synth_buf + M;
    
    g729_Init_Decod_ld8a(state);
    g729_Init_Post_Filter(&(state->post_filter_state));
    g729_Init_Post_Process(&(state->post_process_state));
    
    return 0;
}

G729_Word32 G729A_Decoder_Process(G729A_Dec_state decState, G729_UWord8 * inData, G729_Word16 * speechOut)
{
    static G729_Word16 bad_lsf = 0;          /* Initialize bad LSF indicator */
    
    G729_Word16  parm[PRM_SIZE+1];           /* Synthesis parameters        */
    G729_Word16  Az_dec[MP1*2];              /* Decoded Az for post-filter  */
    G729_Word16  T2[2];                      /* Pitch lag for 2 subframes   */

    g729a_decoder_state *state;
    if ( NULL == decState ) return -1;
    
    state = (g729a_decoder_state *)decState;

    g729_bits2prm_ld8k_compressed(inData, &parm[1]);
    
    parm[0] = 0;           /* No frame erasure */
    
    /* check pitch parity and put 1 in parm[4] if parity error */
    parm[4] = g729_Check_Parity_Pitch(parm[3], parm[4]);
    
    g729_Decod_ld8a(state, parm, state->synth, Az_dec, T2, bad_lsf);
    g729_Post_Filter(&(state->post_filter_state), state->synth, Az_dec, T2);
    g729_Post_Process(&(state->post_process_state), state->synth, speechOut, L_FRAME);
    
    return 0;
}

G729_Word32 G729A_Decoder_Get_Error(G729A_Enc_state decState)
{
    g729a_decoder_state * state;
    if ( NULL == decState ) return -1;
    
    state = (g729a_decoder_state *)decState;
    
    return state->error;
}

/*---------------------------------------------*
 * Generic functions                           *
 *---------------------------------------------*/

const char * G729A_Get_Version()
{
    static const char * version = "1.1";
    return version;
}

/*---------------------------------------------*
 * Testing functions                           *
 *---------------------------------------------*/

G729_Word32 G729A_Encoder_Process_Testing(G729A_Enc_state encState, G729_Word16 * speechIn, G729_Word16 * outData)
{
    g729a_encoder_state * state;
    G729_Word16 prm[PRM_SIZE];  /* Analysis parameters. */
    
    if ( NULL == encState ) return -1;
    
    state = (g729a_encoder_state *)encState;
    
    g729_Pre_Process(&(state->pre_process_state), speechIn, state->new_speech, L_FRAME);
    g729_Coder_ld8a(state, prm);
    g729_prm2bits_ld8k(prm, outData);
    
    return 0;
}

G729_Word32 G729A_Decoder_Process_Testing(G729A_Dec_state decState, G729_Word16 * inData, G729_Word16 * speechOut)
{
    static G729_Word16 bad_lsf = 0;          /* Initialize bad LSF indicator */
    
    G729_Word16 i;
    G729_Word16 parm[PRM_SIZE+1];           /* Synthesis parameters        */
    G729_Word16 Az_dec[MP1*2];              /* Decoded Az for post-filter  */
    G729_Word16 T2[2];                      /* Pitch lag for 2 subframes   */
    
    g729a_decoder_state *state;
    if ( NULL == decState ) return -1;
    
    state = (g729a_decoder_state *)decState;
    
    g729_bits2prm_ld8k(&inData[2], &parm[1]);
    
    parm[0] = 0;           /* No frame erasure */
    for ( i = 2; i < SERIAL_SIZE; ++i )
    {
        if (inData[i] == 0 ) parm[0] = 1;  /* frame erased */
    }
    
    /* check pitch parity and put 1 in parm[4] if parity error */
    parm[4] = g729_Check_Parity_Pitch(parm[3], parm[4]);
    
    g729_Decod_ld8a(state, parm, state->synth, Az_dec, T2, bad_lsf);
    g729_Post_Filter(&(state->post_filter_state), state->synth, Az_dec, T2);
    g729_Post_Process(&(state->post_process_state), state->synth, speechOut, L_FRAME);
    
    return 0;
}
/* end of file */
