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


#ifndef _MpdPtAVT_h_
#define _MpdPtAVT_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"

// FORWARD DECLARATIONS

class OsNotification;
class MprRecorder;

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Derived class for Pingtel AVT/Tone decoder.
class MpdPtAVT: public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpdPtAVT(int payloadType);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this decoder
     */

     /// Destructor
   virtual ~MpdPtAVT(void);

     /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initDecode();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeDecode(void);
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Receive a packet of RTP data
   virtual int decodeIn(const MpRtpBufPtr &pPacket ///< (in) Pointer to a media buffer
                       );
     /**<
     *  @returns length of packet to hand to jitter buffer, 0 means don't.
     */

     /// Decode incoming RTP packet.
   virtual int decode(const MpRtpBufPtr &pPacket, ///< (in) Pointer to a media buffer
                      unsigned decodedBufferLength, ///< (in) Length of the samplesBuffer (in samples)
                      MpAudioSample *samplesBuffer ///< (out) Buffer for decoded samples
                     );
     /**<
     *  @note This function do nothing. All real work is done in decodeIn().
     */

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

     //:Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
   virtual UtlBoolean handleSetDtmfNotify(OsNotification* n);
     /**<
     *  @returns <b>TRUE</b>
     */

   virtual UtlBoolean setDtmfTerm(MprRecorder* pRecorder);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo; ///< static information about the codec

   int mCurrentToneKey;                ///< The key ID
   unsigned int mPrevToneSignature;    ///< The timestamp for last KEYUP event
   unsigned int mCurrentToneSignature; ///< The starting timestamp
   unsigned int mToneDuration;         ///< last reported duration
   OsNotification* mpNotify;     ///< Object to signal on key-down/key-up events
   MprRecorder* mpRecorder;

   void signalKeyDown(const MpRtpBufPtr &pPacket);
   void signalKeyUp(const MpRtpBufPtr &pPacket);
};

#endif  // _MpdPtAVT_h_
