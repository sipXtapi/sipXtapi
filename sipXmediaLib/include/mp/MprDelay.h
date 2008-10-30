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
*  This resource buffers data when the active speech starts until 
*  startPlay() is called. It then starts to play out buffered data 
*  in the FIFO manner, adding incoming frames to the end of the buffer.
*  It stops playing when stopPlay() is called and buffer is flushed.
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
