//  
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
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
#include "mp/MpCodecInfo.h"
#include "mp/MpRtpBuf.h"
#include "mp/MpTypes.h"
#include "os/OsStatus.h"
#include "mp/MpPlgStaffV1.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsNotification;

/// Base class for all media processing decoders.
class MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpDecoderBase(int payloadType,
                 const MpCodecCallInfoV1& callInfo,
                 const MppCodecInfoV1_1& codecInfo,
                 const char* defaultFmtp);
     /**<
     *  @param[in] payloadType - RTP payload type associated with this decoder
     */

     /// Destructor
   ~MpDecoderBase();

     /// Initializes a codec data structure for use as a decoder.
   OsStatus initDecode(const char* codecFmtString);
     /**<
     *  @retval OS_SUCCESS - Success
     *  @retval OS_NO_MEMORY - Memory allocation failure
     */

     /// Initializes a codec data structure for use as a decoder using default fmtp.
   OsStatus initDecode();
     /**<
     *  @retval OS_SUCCESS - Success
     *  @retval OS_NO_MEMORY - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by initDecode()
   OsStatus freeDecode();
     /**<
     *  @retval OS_SUCCESS - Success.
     *  @retval OS_INVALID_STATE - Object has already been freed.
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Decode incoming RTP packet
   int decode(const MpRtpBufPtr &pPacket,
              unsigned decodedBufferLength,
              MpAudioSample *samplesBuffer);
     /**<
     *  This method is called by MprDecode for every incoming RTP packet. If
     *  codec have internal PLC, then this method is also called for every lost
     *  packet with \p pPacket=NULL to leverage codec's internal PLC.
     *  For signaling codecs (namely RFC2833/4733) right after decode()
     *  MprDecode calls getSignalingData() to retrieve decoded signaling data.
     *  That is decoder should cache this data internally between decode()
     *  and getSignalingData() calls.
     *
     *  @see getSignalingData()
     *
     *  @param[in]  pPacket - Pointer to a RTP packet. May be NULL if packet
     *              was lost.
     *  @param[in]  decodedBufferLength - Length of the samplesBuffer (in samples).
     *  @param[out] samplesBuffer - Buffer for decoded samples.
     *
     *  @returns Number of decoded samples.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get information about the decoder.
   const MpCodecInfo* getInfo() const;
     /**<
     *  @returns A pointer to a MpCodecInfo object that provides
     *           information about the decoder. For codecs with only one mode
     *           supported (like G.711 and GSM) it may return pointer to
     *           static class member. More complex codecs (like Speex) may
     *           return pointer to usual class member.
     */

     /// Returns the RTP payload type associated with this decoder.
   int getPayloadType();

     /// Get signaling data from last decoded packet.
   OsStatus getSignalingData(uint8_t &event,
                             UtlBoolean &isStarted,
                             UtlBoolean &isStopped,
                             uint16_t &duration);
     /**<
     *  If codec is signaling, that is it is able to carry signaling data,
     *  this function is called right after decode() to get signaling data
     *  from last decoded RTP packet.
     *
     *  @note Only DTMF digits are supported. Other signaling data is not yet
     *        supported.
     *  @note Default implementation just return OS_NOT_SUPPORTED.
     *
     *  @see decode()
     *
     *  @retval OS_SUCCESS - Last decoded packet had signaling data, \p key,
     *          \p pressState and \p duration were filled with decoded values.
     *  @retval OS_NO_MORE_DATA - Last packet had no meaningful signaling data.
     *  @retval OS_NOT_SUPPORTED - This codec is pure audio codec and could not
     *          carry any signaling data.
     */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

protected:
/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int mPayloadType;         ///< RTP payload type associated with this decoder.
   MpCodecInfo mCodecInfo;   ///< Info structure for this codec
   const MpCodecCallInfoV1& mCallInfo; ///< Pointers to actual methods of
                             ///< this codec.
   void* plgHandle;          ///< Codec internal handle.
   const char* mDefaultFmtp; ///< Fmtp to use if not passed to initDecode().

   bool isInitialized() const;  ///< Is codec initialized?

     /// Copy constructor
   MpDecoderBase(const MpDecoderBase& rMpDecoderBase);

     /// Assignment operator
   MpDecoderBase& operator=(const MpDecoderBase& rhs);
};

/* ============================ INLINE METHODS ============================ */

inline bool MpDecoderBase::isInitialized() const
{
   return (NULL != plgHandle);
}

#endif  // _MpDecoderBase_h_
