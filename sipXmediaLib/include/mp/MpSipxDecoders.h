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


#ifndef _MpSipxDecoders_h_
#define _MpSipxDecoders_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpTypes.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

#ifdef __cplusplus
extern "C" {
#endif

extern int G711A_Decoder(int numSamples, const uint8_t* inBuff, MpAudioSample* outBuf);
extern int G711U_Decoder(int numSamples, const uint8_t* inBuff, MpAudioSample* outBuf);

extern int G711A_Encoder(int numSamples, const MpAudioSample* inBuff, uint8_t* outBuf);
extern int G711U_Encoder(int numSamples, const MpAudioSample* inBuff, uint8_t* outBuf);

#ifdef __cplusplus
}
#endif

#endif  // _MpSipxDecoders_h_
