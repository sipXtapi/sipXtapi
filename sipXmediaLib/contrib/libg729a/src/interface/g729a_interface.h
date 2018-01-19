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

#ifndef __G729A_INTERFACE_H__
#define __G729A_INTERFACE_H__

#include "g729a_typedef.h"
#include "g729a_errors.h"

typedef void * G729A_Enc_state;
typedef void * G729A_Dec_state;

#ifdef __cplusplus
extern "C" {
#endif
    
/*---------------------------------------------*
 * Encoder functions                           *
 *---------------------------------------------*/
    
/**
 *  @brief  Get size in bytes of the g729a encoder state.
 *
 *  @return  Number of bytes in g729a encoder state.
 */
G729_UWord32 G729A_Encoder_Get_Size();
    
/**
 *  @brief  Init or reset encoder.
 *
 *  @param encState,  Encoder state.
 *
 *  @return   0, succeeded
 *           -1, if an error occurs
 */
G729_Word32 G729A_Encoder_Init(G729A_Enc_state encState);
    
/**
 *  @brief  Encode a frame of 16-bit linear PCM data with g729a.
 *
 *  @param encState,  Encoder state.
 *  @param speechIn,  Speech sample input vector (80 samples).
 *  @param outData,   Encoded output vector (10 Bytes).
 *
 *  @return   0, succeeded
 *           -1, if an error occurs, 
 *               and you can use G729A_Encoder_Get_Error to get the last error code.
 */
G729_Word32 G729A_Encoder_Process(G729A_Enc_state encState, G729_Word16 * speechIn, G729_UWord8 * outData);
    
/**
 *  @brief  Get last error code of encoder.
 *
 *  @param encState,  encoder state.
 *
 *  @return  -1, if getting last error code failed
 *           otherwise, return the last error code of decoder
 */
G729_Word32 G729A_Encoder_Get_Error(G729A_Enc_state encState);


/*---------------------------------------------*
 * Decoder functions                           *
 *---------------------------------------------*/
    
/**
 *  @brief  Get size in bytes of the g729a decoder state.
 *
 *  @return  Number of bytes in g729a decoder state.
 */
G729_UWord32 G729A_Decoder_Get_Size();

/**
 *  @brief  Init or reset decoder.
 *
 *  @param decState,  decoder state.
 *
 *  @return   0, succeeded
 *           -1, if an error occurs
 */
G729_Word32 G729A_Decoder_Init(G729A_Dec_state decState);
    
/**
 *  @brief  Decode a frame of g729a encoded bitstream data.
 *
 *  @param decState,   Decoder state.
 *  @param inData,     Encoded input vector (10 Bytes).
 *  @param speechOut,  Decoded output speech vector (80 samples).
 *
 *  @return   0, succeeded
 *           -1, if an error occurs,
 *               and you can use G729A_Decoder_Get_Error to get the last error code.
 */
G729_Word32 G729A_Decoder_Process(G729A_Dec_state decState, G729_UWord8 * inData, G729_Word16 * speechOut);

/**
 *  @brief  Get last error code of decoder.
 *
 *  @param decState,  Decoder state.
 *
 *  @return  -1, if getting last error code failed
 *           otherwise, return the last error code of decoder
 */
G729_Word32 G729A_Decoder_Get_Error(G729A_Dec_state decState);
    
    
/*---------------------------------------------*
 * Generic functions                           *
 *---------------------------------------------*/
    
/**
 *  @brief  Get the version number.
 *
 *  @return  A pointer to string specifying the version.
 */
const char * G729A_Get_Version();
    
    
/*---------------------------------------------*
 * Testing functions                           *
 * Derived from ITU official testing code      *
 *---------------------------------------------*/
    
/**
 *  @param encState,  Encoder state.
 *  @param speechIn,  Speech sample input vector (80 samples).
 *  @param outData,   Encoded output vector (164 Bytes).
 */
G729_Word32 G729A_Encoder_Process_Testing(G729A_Enc_state encState, G729_Word16 * speechIn, G729_Word16 * outData);
    
/**
 *  @param decState,   Decoder state.
 *  @param inData,     Encoded input vector (164 Bytes).
 *  @param speechOut,  Decoded output speech vector (80 samples).
 */
G729_Word32 G729A_Decoder_Process_Testing(G729A_Dec_state decState, G729_Word16 * inData, G729_Word16 * speechOut);
    
#ifdef __cplusplus
}
#endif

#endif  /* __G729A_INTERFACE_H__ */
/* end of file */
