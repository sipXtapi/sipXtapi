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

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// The "Speex Echo Canceler" media processing resource.
/**
*  It takes mic data from the first input, speaker data from the mpSpkrQ message
*  queue and produce echo canceled audio data on the first output. To increase
*  echo cancellation quality you should connect output of the resource directly
*  to MprSpeexPreprocess input.
*/
class MprSpeexEchoCancel : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const UtlContainableType TYPE; ///< Class name, used for run-time checks.

   enum {
      MAX_ECHO_QUEUE=21
   };

/* ============================ CREATORS ================================== */

     /// Constructor
   MprSpeexEchoCancel(const UtlString& rName,
                      OsMsgQ* pSpkrQ,
                      int filterLength=SPEEX_DEFAULT_AEC_FILTER_LENGTH);
     /**<
     *  @param[in] rName - resource name.
     *  @param[in] pSpkrQ - pointer to a queue with speaker audio data.
     *  @param[in] filterLength  - the amount of data (in samples) you want to
     *             process at once and filter_length is the length (in samples) 
     *             of the echo cancelling filter you want to use (also known as
     *             tail length). The recommended tail length is approximately
     *             the third of the room reverberation time. For example,in
     *             a small room, reverberation time is in the order of 300 ms,
     *             so a tail length of 100 ms is a good choice (800 samples at
     *             8000 Hz sampling rate).
     */

     /// Destructor
   virtual
   ~MprSpeexEchoCancel();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

     /// @copydoc UtlContainable::getContainableType()
   UtlContainableType getContainableType() const;

     /// Return Speex internal echo state structure.
   inline SpeexEchoState *getSpeexEchoState() const;
     /**<
     *  Used by MprSpeexPreprocess to access echo state to enable residual
     *  echo removal.
     */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SpeexEchoState *mpEchoState;
   bool            mStartedCanceling;
   int             mFilterLength;
   OsMsgQ         *mpSpkrQ;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond);

     /// @brief Associates this resource with the indicated flow graph.
   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);
     /**<
     *  We use this overloaded method for initialization of some of our member
     *  variables, which depend on flowgraph's properties (like frame size).
     *
     *  @retval OS_SUCCESS - for now, this method always returns success
     */

     /// Copy constructor (not implemented for this class)
   MprSpeexEchoCancel(const MprSpeexEchoCancel& rMprSpeexEchoCancel);

     /// Assignment operator (not implemented for this class)
   MprSpeexEchoCancel& operator=(const MprSpeexEchoCancel& rhs);

};

/* ============================ INLINE METHODS ============================ */

SpeexEchoState *MprSpeexEchoCancel::getSpeexEchoState() const
{
   return mpEchoState;
}

#endif  // _MprSpeexEchoCancel_h_
