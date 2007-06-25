//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MprFromMic_h_
#define _MprFromMic_h_

#define REAL_SILENCE_DETECTION

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include "mp/MpAudioResource.h"
#include "mp/MpCodec.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*MICDATAHOOK)(const int nLength, short* samples) ;

// FORWARD DECLARATIONS

/**
*  @brief The "From Microphone" media processing resource
*/
class MprFromMic : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum { MAX_MIC_BUFFERS = 10 };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprFromMic(const UtlString& rName, int samplesPerFrame, int samplesPerSec,
              OsMsgQ *pMicQ);

     /// Destructor
   virtual
   ~MprFromMic();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum{EqFilterLen = 24}; ///< Brant, 11 May 2001; was 13, allow for experiments.

   OsMsgQ *mpMicQ;                ///< We will read audio data from this queue.
   int16_t   shpFilterBuf[80 + 10];
   int     mNumFrames;
#ifndef REAL_SILENCE_DETECTION
   unsigned long MinVoiceEnergy;  ///< trigger threshold for silence detection.
#endif

   void  Init_highpass_filter800();
   void  highpass_filter800(int16_t *, int16_t *, short);
   MpAudioBuf::SpeechType speech_detected(int16_t*, int);


   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

     /// Copy constructor (not implemented for this class)
   MprFromMic(const MprFromMic& rMprFromMic);

     /// Assignment operator (not implemented for this class)
   MprFromMic& operator=(const MprFromMic& rhs);

public:
   static MICDATAHOOK s_fnMicDataHook ;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromMic_h_
