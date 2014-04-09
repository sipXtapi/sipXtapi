//  
// Copyright (C) 2008-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpStaticCodecInit_h_
#define _MpStaticCodecInit_h_

#ifdef __pingtel_on_posix__  /* [ */
#  include <config.h>
#else  /* ] [ */
// Set interesting codec to be compiled statically

// For VisualStudio:
// After you have selected Debug_Static/Release_Static as Solution configuration 
// you must select Debug_Static/Release_Static and uncomment same-name define for
// interested codec.
// If you selected Debug/Release without Static ensure that following defines are 
// disabled.
// For CODEC_SPEEX_STATIC you may select to link statically or dynamically libspeex
// For CODEC_G722_STATIC and CODEC_G726_STATIC libspandsp will link dynamically 
// because libspandsp published under GPL (but if you wish you may modify project
// settings)

//#define CODEC_PCMA_PCMU_STATIC
//#define CODEC_TONES_STATIC
//#define CODEC_GSM_STATIC
//#define CODEC_SPEEX_STATIC
//#define CODEC_G722_STATIC
//#define CODEC_G726_STATIC
//#define CODEC_OPUS_STATIC


#endif /* __pingtel_on_posix__ ] */

void mpStaticCodecInitializer(void);
void mpStaticCodecUninitializer(void);

#endif
