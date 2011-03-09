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
#include "mp/MpResourceMsg.h"

// DEFINES

/// @brief Default filter length (tail length) of AEC filter (in milliseconds).
/// @see documentation on MprSpeexEchoCancel::MprSpeexEchoCancel() for more information.
#ifdef ANDROID
#  define SPEEX_DEFAULT_AEC_FILTER_LENGTH 240
#else
#  define SPEEX_DEFAULT_AEC_FILTER_LENGTH 200
#endif

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
#ifdef WIN32
      MAX_ECHO_QUEUE=21,
      DEFAULT_ECHO_QUEUE_LEN=130
#elif defined(ANDROID)
      MAX_ECHO_QUEUE=40,
      DEFAULT_ECHO_QUEUE_LEN=300
#else
      MAX_ECHO_QUEUE=20,
      DEFAULT_ECHO_QUEUE_LEN=90
#endif
   };

   enum GlobalEnableState {
      GLOBAL_ENABLE,   ///< All MprSpeexEchoCancel resources are forced to enable
      GLOBAL_DISABLE,  ///< All MprSpeexEchoCancel resources are forced to disable
      LOCAL_MODE       ///< Each resource respect its own enable/disable state
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprSpeexEchoCancel(const UtlString& rName,
                      OsMsgQ* pSpkrQ=NULL,
                      int spkrQDelayMs=DEFAULT_ECHO_QUEUE_LEN,
                      int filterLength=SPEEX_DEFAULT_AEC_FILTER_LENGTH);
     /**<
     *  @param[in] rName - resource name.
     *  @param[in] pSpkrQ - pointer to a queue with speaker audio data.
     *  @param[in] spkrQDelayMs - number of milliseconds we should delay
     *             delay data from speaker queue. Set this value to maximum
     *             of the delay audio device will *definitely* add while
     *             playing and recording audio. This value is used to minimize
     *             delay between far and near end audio in AEC and reduce
     *             computational cost of AEC.
     *  @param[in] filterLength  - the length (in msec) of the echo cancelling
     *             filter you want to use (also known as tail length).
     *             The recommended tail length is approximately the third of
     *             the room reverberation time. For example,in a small room,
     *             reverberation time is in the order of 300 ms, so a tail
     *             length of 100 ms is a good choice.
     */

     /// Destructor
   virtual
   ~MprSpeexEchoCancel();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Send message to enable/disable copy queue.
   OsStatus setSpkrQ(const UtlString& namedResource, 
                     OsMsgQ& fgQ,
                     OsMsgQ *pSpkrQ);
     /**<
     *  @param[in] namedResource - the name of the resource to send a message to.
     *  @param[in] fgQ - the queue of the flowgraph containing the resource which
     *             the message is to be received by.
     *  @param[in] pSpkrQ - pointer to a queue with speaker data.
     *  @returns the result of attempting to queue the message to this resource.
     */

     /// Set global enable/disable state.
   static inline void setGlobalEnableState(GlobalEnableState state);
     /**<
     *  @see smGlobalEnableState for more details.
     */

     /// Get global state.
   static inline GlobalEnableState getGlobalEnableState();
     /**<
     *  @see smGlobalEnableState for more details.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @copydoc UtlContainable::getContainableType()
   UtlContainableType getContainableType() const;

     /// Return Speex internal echo state structure.
   inline SpeexEchoState *getSpeexEchoState() const;
     /**<
     *  Used by MprSpeexPreprocess to access echo state to enable residual
     *  echo removal.
     */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   typedef enum
   {
      MPRM_SET_SPEAKER_QUEUE = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START
   } AddlResMsgTypes;

   SpeexEchoState *mpEchoState;       ///< Speex internal AEC state
   bool            mStartedCanceling; ///< Have we started cancelling?
   int             mFilterLength;     ///< Filter tail length. See constructor docs for details
   OsMsgQ         *mpSpkrQ;           ///< Queue with echo reference data
   const int       mSpkrQDelayMs;     ///< How much should we delay data from mpSpkrQ (in ms)?
   int             mSpkrQDelayFrames; ///< How much should we delay data from mpSpkrQ (in frames)?
   MpAudioBufPtr   mpSilenceBuf;      ///< Buffer with silence - used when empty message
                                      ///< arrives from mpSpkrQ or mic input

   static volatile GlobalEnableState smGlobalEnableState;
     ///< Global enable/disable switch for all Speex AEC resources. We need
     ///< this switch because sipXmediaAdapterLib exports only a static method
     ///< to turn AEC on and off.

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond);

     /// @copydoc MpResource::handleMessage()
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

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

void MprSpeexEchoCancel::setGlobalEnableState(GlobalEnableState state)
{
   smGlobalEnableState = state;
}

MprSpeexEchoCancel::GlobalEnableState  MprSpeexEchoCancel::getGlobalEnableState()
{
   return(smGlobalEnableState);
}

SpeexEchoState *MprSpeexEchoCancel::getSpeexEchoState() const
{
   return mpEchoState;
}

#endif  // _MprSpeexEchoCancel_h_
