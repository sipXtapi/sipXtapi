//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprDecode_h_ /* [ */
#define _MprDecode_h_

#include "mp/MpMisc.h"
#include "os/OsBSem.h"

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MprDejitter.h"
#include "mp/MpFlowGraphMsg.h"
#include "net/SdpCodec.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpConnection;
class MpDecoderBase;
class MprRecorder;

/// The "Decode" media processing resource
class MprDecode : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MpConnection;

#ifdef _DEPRECATED_IPSE_
   /* This was 3 in the past, which is probably far too pessimistic most of
    * the time.  1 is probably fine on a LAN or with only a router or two.
    * Let's leave it at 3 for now...
    */
   /* ... that was then, this is now.  VON is coming, squeeze it down! */
   /* ... VON is coming again, and so is NetEQ.  Make it at least 5, or lose */
   enum { MIN_RTP_PACKETS = 5};
#endif

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprDecode(const UtlString& rName, MpConnection* pConn,
             int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MprDecode();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   OsStatus selectCodecs(SdpCodec* codecs[], int numCodecs);

   OsStatus selectCodec(SdpCodec& rCodec);

   OsStatus deselectCodec(void);

   void setMyDejitter(MprDejitter* newDJ);

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

     /// Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
   UtlBoolean handleSetDtmfNotify(OsNotification* n);
     /**<
     *  @returns <b>TRUE</b>
     */

   UtlBoolean setDtmfTerm(MprRecorder *pRecorder);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   enum AddlMsgTypes
   {
      SELECT_CODECS = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      DESELECT_CODECS,
   };

   enum { MAX_RTP_FRAMES = 25};

   unsigned int mTimeStampOffset;
   int          mPreloading;

   MprDejitter* mpMyDJ;

   /// Semaphore that protects access to the m*Codecs members.
   /**
   *  This is needed because outside threads can call the destructor.
   */
   OsMutex mLock;

   /// List of the codecs to be used to decode media.
   /**
   *  Pointer to array of length mNumCurrentCodecs of MpDecoderBase*'s
   *  which represent the codecs, or NULL if mNumCurrentCodecs == 0.
   */
   MpDecoderBase** mpCurrentCodecs;
   int             mNumCurrentCodecs; ///< Length of mpCurrentCodecs array.

   /// Similar list of all codecs that have ever been listed on mpCurrentCodecs.
   MpDecoderBase** mpPrevCodecs;
   int             mNumPrevCodecs; ///< Length of mpPrevCodecs array.

   MpConnection*   mpConnection;   ///< Link to the parent Connection.

   enum {
      NUM_TRACKED_PACKETS = 128
   };

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

   UtlBoolean isPayloadTypeSupported(int payloadType);

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Replace mpCurrentCodecs with pCodecs.
   UtlBoolean handleSelectCodecs(SdpCodec* pCodecs[], int numCodecs);
     /**<
     *  Copy the codecs in mpCurrentCodecs onto mpPrevCodecs and deletes pCodecs.
     */

     /// @brief Remove all codecs from mpCurrentCodecs, transferring them to
     /// mpPrevCodecs.
   UtlBoolean handleDeselectCodecs(void);

     /// Remove one codec from mpConnection's payload type decoder table.
   UtlBoolean handleDeselectCodec(MpDecoderBase* pDecoder);
     /**<
     *  @note Caller must hold mLock.
     */

     /// Copy constructor (not implemented for this class)
   MprDecode(const MprDecode& rMprDecode);

     /// Assignment operator (not implemented for this class)
   MprDecode& operator=(const MprDecode& rhs);

   MprDejitter* getMyDejitter(void);

   void pushIntoJitterBuffer(MpRtpBufPtr &rtp, int packetLen);

};

/* ============================ INLINE METHODS ============================ */

#endif  /* _MprDecode_h_ ] */
