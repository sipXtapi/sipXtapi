//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MprDelay_h_
#define _MprDelay_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MpAudioBuf.h"
#include "mp/MpResourceMsg.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
*  @brief Delay Resource
*
*  This resource is intended to add user-controlled delay to audio stream.
*  Such a strange feature may be useful in many advanced use-cases, e.g. when
*  our network may be not ready to accept data at the moment data becomes
*  ready.
*
*  <h3> Internal state machine description </h3>
*  
*  Delay resource can be found in four internal states:
*  1) WAITING. This is initial state of resource after creation and also is
*     the state after stopPlay() call. WAITING state means that resource is idle
*     and do not appear to have active audio input. No frames are produced on
*     output.
*
*     WAITING state will be changed to BUFFERING automatically when frame with
*     active audio is received, and it can be manually changed to PLAYING state
*     by calling startPlay() method.
*
*  <pre>    active voice
*  WAITING ---------------> BUFFERING
*     |
*     | startPlay()
*     v
*  PLAYING
*  </pre>
*
*  <pre>       stopPlay()
*  WAITING <--------------- BUFFERING
*     ^ ^______________________
*     |        stopPlay()      |
*     | stopPlay()             |
*  PLAYING            PLAYING_WITHOUT_DELAY
*  </pre>
*
*  2) BUFFERING. In this state resource stores all incoming frames to internal
*     circular buffer and does not produce any output. Thus the longer resource
*     remains in this state, the longer delay will be introduced.
*
*     This state can only be reached from WAITING state in case active voice has
*     been received. Calling startPlay() in BUFFERING state leads to transition
*     to PLAYING state, calling stopPlay() moves resource back to WAITING state.
*
*
*  <pre>    active voice
*  WAITING ---------------> BUFFERING
*     ^______________________|  |
*            stopPlay()         |
*     __________________________|
*     |      startPlay()
*     v
*  PLAYING
*  </pre>
*
*  3) PLAYING. In this state resource pops recorded data from the head of
*     internal buffer and sends it to output, adding newly arrived data to
*     the tail of internal buffer. I.e. data is played back with constant delay,
*     which depends on the time spent in BUFFERING state.
*
*     Resource can be put to PLAYING state only manually by calling startPlay()
*     method. Calling stopPlay() method when in PLAYING state will move resource
*     to WAITING state. If internal buffer is empty then resource will
*     automatically fall through to PLAYING_WITHOUT_DELAY state.
*
*  4) PLAYING_WITHOUT_DELAY. In this case every input frame is directly
*     forwarded to the output, i.e. no delay is introduced. At this moment
*     this state is hardly useful, but it is thought to become useful when
*     delay resource is able to "catch up", i.e. reduce delay to zero using
*     e.g. timescaling technique. From application perspective this state
*     is indistinguishable from PLAYING state. 
*
*     This state can only be reached from PLAYING state if internal buffer
*     is empty. Calling stopPlay() will move resource to WAITING state.
*
*  <pre>          stopPlay()
*  WAITING <------------------------.
*                                   |
*                                   |
*           empty buffer            |
*  PLAYING --------------> PLAYING_WITHOUT_DELAY
*  </pre>
*
*
*  <h3> Notifications </h3>
*
*  1) When state changes from WAITING to BUFFERING (i.e. when active audio is
*     detected on input) resource emits MPRNM_DELAY_SPEECH_STARTED notification.
*     User should respond to this notification by calling startPlay() when
*     he thinks relevant. User can also call stopPlay() to put resource into
*     WAITING state again. Until one of this action is performed resource
*     will buffer input data and thus increase induced delay.
*
*  2) When resource in PLAYING state detects, that its buffer contain only
*     NULL frames and frames with inactive audio it emits MPRNM_DELAY_QUIESCENCE
*     for user to perform any relevant actions. E.g. user can call stopPlay()
*     and power down network interface, if there are no other active audio
*     sources. Note, that at in current implementation MPRNM_DELAY_QUIESCENCE
*     notification is not emited in PLAYING_WITHOUT_DELAY state.
*
*  3) When resource detects active audio after quescent situation have been
*     recently detected resource emits MPRNM_DELAY_SPEECH_STARTED notification.
*     This should warn user that resource's buffer contain active audio again.
*
*  4) When resource in PLAYING state detects empty internal buffer it emits
*     MPRNM_DELAY_NO_DELAY notification and enters PLAYING_WITHOUT_DELAY state.
*
*  <h3> How to control Delay resource </h3>
*
*  If you're using flowgraph with Delay resources in it you must control them
*  with care. Use case deemed to be most useful consists of three steps repeated
*  foreveris as outlined below:
*
*  1) Initially Delay resource is in WAITING state. User is listenning for
*     notifications and honestly expects MPRNM_DELAY_SPEECH_STARTED to appear
*     soon.
*  2) MPRNM_DELAY_SPEECH_STARTED is received by user which means that resource
*     is now in BUFFERING state. It will remain in this state until user calls
*     either startPlay() or stopPlay(). Once user thinks it's the time to start
*     playback (e.g. network interface have woken) he calls startPlay()
*     (goto (3) then) and resource is put into PLAYING state. If for any reason
*     user thinks buffered audio should be discarded he calls stopPlay() to put
*     resource to WAITING state again (goto (1) then).
*  3) In PLAYING state user can call stopPlay() at any time to discard buffered
*     frames and put resource into WAITING state (goto (1) then). But it's most
*     reasonable to wait for MPRNM_DELAY_QUIESCENCE notification and call
*     stopPlay() only them. This will minimize impact on audio quality caused
*     by dropped frames.
*
*  @nosubgrouping
*/
class MprDelay : public MpAudioResource
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:
   enum
   {
      DEFAULT_FIFO_CAPACITY = 1000
   };
/* =============================== CREATORS =============================== */
///@name Creators
//@{
     /// Constructor
   MprDelay(const UtlString& rName, int capacity = DEFAULT_FIFO_CAPACITY);

     /// Destructor
   ~MprDelay();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Send message to start play
   static UtlBoolean startPlay(const UtlString& namedResource, 
                               OsMsgQ& fgQ);

     /// Send message to stop play
   static UtlBoolean stopPlay(const UtlString& namedResource, 
                              OsMsgQ& fgQ);

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{


//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{
   
     /// Get delay in milliseconds.
   int getDelayMs();

     /// Get delay in samples.
   int getDelaySamples();

     /// Get delay in frames.
   int getDelayFrames();

//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:

   UtlBoolean doDelay(MpBufPtr& inBuf,
                      MpBufPtr& outBuf,
                      int samplesPerFrame,
                      int samplesPerSecond);
   
     /// @copydoc MpResource::handleMessage
   UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// @copydoc MpAudioResource::doProcessFrame
   UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                             MpBufPtr outBufs[],
                             int inBufsSize,
                             int outBufsSize,
                             UtlBoolean isEnabled,
                             int samplesPerFrame,
                             int samplesPerSecond);


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   enum MprDelayState
   {
      MPDS_WAITING,
      MPDS_BUFFERING,
      MPDS_PLAYING,
      MPDS_PLAYING_WITHOUT_DELAY
   };

   typedef enum
   {
      MPRM_DELAY_START_PLAY = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
      MPRM_DELAY_STOP_PLAY
   } AddlMsgTypes;

   /**
   *  @brief Fifo for Delay resource
   *
   *  @nosubgrouping
   */
   class MprDelayFifo
   {
   /* ///////////////////////////// PUBLIC //////////////////////////////// */
   public:

   /* ============================ CREATORS =============================== */
   ///@name Creators
   //@{
        /// Constructor
      MprDelayFifo(int capacity);

        /// Destructor
      ~MprDelayFifo();

   //@}

   /* ========================== MANIPULATORS ============================= */
   ///@name Manipulators
   //@{
        /// Reset FIFO
      void reset();

        /// Push audio buffer to FIFO
      void pushData(MpAudioBufPtr& databuff);

        /// Pop audio buffer from FIFO
      void popData(MpAudioBufPtr& databuff);
        /**<
        *  User must ensure that queue is not empty by himself.
        *  Pop from an empty queue will corrupt its internal state.
        */

   //@}

   /* =========================== ACCESSORS =============================== */
   ///@name Accessors
   //@{

        /// Get number of buffers in queue
      int getFifoLength() const;


   //@}

   /* ============================ INQUIRY ================================ */
   ///@name Inquiry
   //@{

        /// Does FIFO contain only silent frames?
      UtlBoolean isQuiescent() const;

        /// Is FIFIO empty
      UtlBoolean isEmpty() const;

   //@}

   /* /////////////////////////// PROTECTED /////////////////////////////// */
   protected:

   /* //////////////////////////// PRIVATE //////////////////////////////// */
   private:
      MpAudioBufPtr* mBuff;
      int mBuffSize;
      int mEndPosition;
      int mStartPosition;
      int mNumActiveFrames;
   };

   MprDelayState mState; ///< State of delay resource
   UtlBoolean mIsQuiescenceNotified; ///< Have FIFO quiescent state been notified?
   MprDelayFifo mFifo;   ///< FIFO for the stored data
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDelay_h_
