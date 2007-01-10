//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "mp/MpTypes.h"

void ConvertUnsigned8ToSigned16(unsigned char *in_buffer, MpAudioSample *out_buffer, int numBytesToConvert);

/// Returns the GCD of a and b
int gcd(int a, int b);
/**<
*  Don't pass it negative numbers or (0, 0)
*/

/// Downsamples from current rate to new rate. Doesn't upsample yet
int reSample(char * charBuffer, int numBytes, int currentSampleRate, int newSampleRate);

/// Merges two or more channels into one.
int mergeChannels(char * charBuffer, int Size, int nTotalChannels);
/**<
*  Takes size in bytes as input.  Returns new size in bytes
*/

/// Works with 16bit wavs only.  (for now)
OsStatus mergeWaveUrls(UtlString rSourceUrls[], UtlString &rDestFile);

/// Works with 16bit wavs only.  (for now)
OsStatus mergeWaveFiles(UtlString rSourceFiles[], UtlString &rDestFile);

//routines for compressing & decompressing aLaw and uLaw
void InitG711Tables();
size_t DecompressG711MuLaw(MpAudioSample *buffer,size_t length);
size_t DecompressG711ALaw(MpAudioSample *buffer, size_t length);
unsigned char ALawEncode2(MpAudioSample s);
unsigned char MuLawEncode2(MpAudioSample s);
MpAudioSample MuLawDecode2(unsigned char ulaw);
MpAudioSample ALawDecode2(unsigned char alaw);
