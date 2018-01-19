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

/*-----------------------------------------------------------------*
 * Main program of the G.729a 8.0 kbit/s decoder.                  *
 *                                                                 *
 *    Usage : decoder  bitstream_file  synth_file                  *
 *                                                                 *
 *-----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include "g729a_typedef.h"
#include "g729a_interface.h"

#define FRAMESIZE      80

#if defined(CONTROL_OPT_ITU) && (CONTROL_OPT_ITU == 1)
#define SERIALSIZE     (80+2)
#else
#define SERIALSIZE     10
#endif

/*
 This variable should be always set to zero unless transmission errors
 in LSP indices are detected.
 This variable is useful if the channel coding designer decides to
 perform error checking on these important parameters. If an error is
 detected on the  LSP indices, the corresponding flag is
 set to 1 signalling to the decoder to perform parameter substitution.
 (The flags should be set back to 0 for correct transmission).
 */

/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int main(int argc, char *argv[] )
{
#if defined(CONTROL_OPT_ITU) && (CONTROL_OPT_ITU == 1)
    G729_Word16
#else
    G729_UWord8
#endif
    serial[SERIALSIZE];
    G729_Word16  speechOut[FRAMESIZE];
    
    G729_Word16 frame;
    FILE   *f_syn, *f_serial;
    
    G729A_Dec_state state;
    
    printf("\n");
    printf("************   G.729a 8.0 KBIT/S SPEECH DECODER  ************\n");
    printf("\n");
    printf("------------------- Fixed point C simulation ----------------\n");
    printf("\n");
    printf("------------ Version 1.1 (Release 2, November 2006) --------\n");
    printf("\n");
    
    /* Passed arguments */
    
    if ( argc != 3)
    {
        printf("Usage :%s bitstream_file  outputspeech_file\n",argv[0]);
        printf("\n");
        printf("Format for bitstream_file:\n");
        printf("  One (2-byte) synchronization word \n");
        printf("  One (2-byte) size word,\n");
        printf("  80 words (2-byte) containing 80 bits.\n");
        printf("\n");
        printf("Format for outputspeech_file:\n");
        printf("  Synthesis is written to a binary file of 16 bits data.\n");
        exit( 1 );
    }
    
    /* Open file for synthesis and packed serial stream */
    
    if( (f_serial = fopen(argv[1],"rb") ) == NULL )
    {
        printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
        exit(0);
    }
    
    if( (f_syn = fopen(argv[2], "wb") ) == NULL )
    {
        printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
        exit(0);
    }
    
    printf("Input bitstream file  :   %s\n",argv[1]);
    printf("Synthesis speech file :   %s\n",argv[2]);
    
    /*-----------------------------------------------------------------*
     *           Initialization of decoder                             *
     *-----------------------------------------------------------------*/
    
    state = malloc(G729A_Decoder_Get_Size());
    if ( NULL == state ) return -1;
    
    G729A_Decoder_Init(state);
    
    /*-----------------------------------------------------------------*
     *            Loop for each "L_FRAME" speech data                  *
     *-----------------------------------------------------------------*/
    
    frame = 0;
    
#if defined(CONTROL_OPT_ITU) && (CONTROL_OPT_ITU == 1)
    while( fread(serial, sizeof(G729_Word16), SERIALSIZE, f_serial) == SERIALSIZE)
#else
    while( fread(serial, sizeof(G729_UWord8), SERIALSIZE, f_serial) == SERIALSIZE)
#endif
    {
        printf("Frame =%d\r", frame++);
        
#if defined(CONTROL_OPT_ITU) && (CONTROL_OPT_ITU == 1)
        G729A_Decoder_Process_Testing(state, serial, speechOut);
#else
        G729A_Decoder_Process(state, serial, speechOut);
#endif
        fwrite(speechOut, sizeof(G729_Word16), FRAMESIZE, f_syn);
    }
    
    printf("Frame =%d\n", frame);
    return(0);
}

