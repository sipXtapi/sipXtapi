//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 ProfitFuel Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _MprSpeexEchoCancel_h_
#define _MprSpeexEchoCancel_h_

// SYSTEM INCLUDES
#include <speex/speex_echo.h>

// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MpBufPool.h"

// DEFINES

/// @brief Default filter length (tail length) of AEC filter. See documentation
/// on MprSpeexEchoCancel::MprSpeexEchoCancel() for more information.
#define SPEEX_DEFAULT_AEC_FILTER_LENGTH 100

/// @brief Default number of messages, reserved for echo residue. See
/// documentation on MprSpeexEchoCancel::MprSpeexEchoCancel() for more
/// information.
#define SPEEX_DEFAULT_ECHO_RESIDUE_POOL_SIZE 1

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// The "Speex Echo Canceler" media processing resource.
/**
*  It takes mic data from the first input, speaker data from the pEchoQ message
*  queue and produce echo canceled audio data on the first output. On the second
*  output there are echo residue power spectrum data, which may be used to 
*  remove echo residue by Speex preprocessor (MprSpeexPreprocess).
*  MprSpeexPreprocess expect echo reside data on its second input.
*/
class MprSpeexEchoCancel : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum {
      MAX_ECHO_QUEUE=21
   };

/* ============================ CREATORS ================================== */

     /// Constructor
   MprSpeexEchoCancel(const UtlString& rName,
                      int samplesPerFrame, int samplesPerSec,
                      int filterLength=SPEEX_DEFAULT_AEC_FILTER_LENGTH,
                      int echoResiduePoolSize=SPEEX_DEFAULT_ECHO_RESIDUE_POOL_SIZE);
     /**<
     *  @param rName - resource name.
     *  @param samplesPerFrame - number of audio samples per frame. It is
     *                           recommended to use a frame size in the order of
     *                           20 ms (or equal to the codec frame size) and
     *                           make sure it is easy to perform an FFT of that
     *                           size (powers of two are better than prime sizes).
     *  @param samplesPerSec - number of audio samples per second.
     *  @param filterLength  - the amount of data (in samples) you want to
     *                         process at once and filter_length is the length
     *                         (in samples) of the echo cancelling filter you
     *                         want to use (also known as tail length).
     *                         The recommended tail length is approximately the
     *                         third of the room reverberation time. For example,
     *                         in a small room, reverberation time is in the
     *                         order of 300 ms, so a tail length of 100 ms is a
     *                         good choice (800 samples at 8000 Hz sampling rate).
     *  @param echoResiduePoolSize - number of messages, reserved for echo
     *                               residue. If some sort of caching will appear
     *                               at the other end, this value should be
     *                               accordingly increased.
     */

     /// Destructor
   virtual
   ~MprSpeexEchoCancel();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SpeexEchoState *mpEchoState;
   bool            mStartedCanceling;
   bool            mEchoResidueCurrent;
   MpBufPool       mEchoResiduePool;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

     /// Copy constructor (not implemented for this class)
   MprSpeexEchoCancel(const MprSpeexEchoCancel& rMprSpeexEchoCancel);

     /// Assignment operator (not implemented for this class)
   MprSpeexEchoCancel& operator=(const MprSpeexEchoCancel& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprSpeexEchoCancel_h_