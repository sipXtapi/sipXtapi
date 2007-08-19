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

#ifndef _MprDecode_h_ /* [ */
#define _MprDecode_h_

#include "mp/MpMisc.h"
#include "os/OsBSem.h"

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MprDejitter.h"
#include "mp/MpFlowGraphMsg.h"
#include "sdp/SdpCodec.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpRtpInputAudioConnection;
class MpDecoderBase;
class MprRecorder;
class MpJitterBuffer;

/// The "Decode" media processing resource
class MprDecode : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MpRtpInputAudioConnection;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprDecode(const UtlString& rName, MpRtpInputAudioConnection* pConn,
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

     /// Returns a pointer to the JB instance, creating it if necessary
   MpJitterBuffer* getJBinst(UtlBoolean optional = FALSE);
     /**<
     *  If the instance has not been created, but the argument "optional" is
     *  TRUE, then do not create it, just return NULL.
     */

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
   typedef enum
   {
      SELECT_CODECS = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      DESELECT_CODECS
   } AddlMsgTypes;

   enum {
      MAX_RTP_FRAMES = 25,
      MAX_PAYLOAD_TYPES = 128,
      NUM_TRACKED_PACKETS = 128
   };

   unsigned int mNextPullTimerCount;
   int          mWaitTimeInFrames;
   unsigned int   sTimerCountIncrement;
   int          mMissedFrames;
   int            saveDebug;
//   MpRtpBufPtr  mSavedRtp[MAX_PAYLOAD_TYPES];
   MpJitterBuffer* mpJB;            ///< Pointer to JitterBuffer instance

   MprDejitter* mpMyDJ;

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

   MpRtpInputAudioConnection*   mpConnection;   ///< Link to the parent Connection.

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
   UtlBoolean handleDeselectCodecs(UtlBoolean shouldLock = TRUE);

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

};

/* ============================ INLINE METHODS ============================ */

#endif  /* _MprDecode_h_ ] */
