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

#ifndef _MpDecoderBase_h_
#define _MpDecoderBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "mp/MpCodecInfo.h"
#include "mp/MpRtpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpAudioConnection;
class OsNotification;
class MprRecorder;

/// Base class for all media processing decoders.
class MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MprDecode;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpDecoderBase( int payloadType ///< (in) RTP payload type associated with
                                  ///<  this decoder
                , const MpCodecInfo* pInfo ///< (in) pointer to derived class'
                                           ///<  static const MpCodecInfo
                );

     /// Destructor
   virtual
   ~MpDecoderBase();

     /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initDecode(MpAudioConnection* pConnection)=0;
     /**<
     *  @param pConnection - (in) Pointer to the MpAudioConnection container
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeDecode(void)=0;
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
     *  @note This method can be called more than one time per frame interval.
     *
     *  @returns >0 - length of packet to hand to jitter buffer.
     *  @returns 0  - decoder don't want more packets.
     *  @returns -1 - discard packet (e.g. out of order packet).
     */

     /// Decode incoming RTP packet
   virtual int decode(const MpRtpBufPtr &pPacket, ///< (in) Pointer to a media buffer
                      unsigned decodedBufferLength, ///< (in) Length of the samplesBuffer (in samples)
                      MpAudioSample *samplesBuffer ///< (out) Buffer for decoded samples
                     ) =0;
     /**<
     *  @return Number of decoded samples.
     */

     /// @brief This method allows a codec to take action based on the length of
     /// the jitter buffer since last asked.
   virtual int reportBufferLength(int iAvePackets);

     /// DOCME
   virtual void frameIncrement();

     /// Always assert(FALSE) for now.
   virtual UtlBoolean setDtmfTerm(MprRecorder *pRecorder);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get static information about the decoder
   virtual const MpCodecInfo* getInfo(void) const;
     /**<
     *  @returns a pointer to an <i>MpCodecInfo</i> object that provides
     *  static information about the decoder.
     */

     /// Returns the RTP payload type associated with this decoder.
   virtual int getPayloadType(void);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

protected:
/* //////////////////////////// PROTECTED ///////////////////////////////// */

     /// Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
   virtual UtlBoolean handleSetDtmfNotify(OsNotification* pNotify);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor
   MpDecoderBase(const MpDecoderBase& rMpDecoderBase);

     /// Assignment operator
   MpDecoderBase& operator=(const MpDecoderBase& rhs);

   const MpCodecInfo* mpCodecInfo;
   int mPayloadType;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDecoderBase_h_
