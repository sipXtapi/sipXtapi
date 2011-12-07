//  
// Copyright (C) 2006-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpEncoderBase_h_
#define _MpEncoderBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "mp/MpAudioBuf.h"
#include "mp/MpCodecInfo.h"
#include "mp/MpPlgStaffV1.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Base class for all media processing encoders.
class MpEncoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpEncoderBase(int payloadType,
                 const MpCodecCallInfoV1& callInfo,
                 const MppCodecInfoV1_1& codecInfo,
                 const char* defaultFmtp);
     /**<
     *  @param[in] payloadType - RTP payload type associated with this encoder.
     *  @param[in] callInfo - codec manipulator functions.
     *  @param[in] codecInfo - codec information struct.
     *  @param[in] defaultFmtp - default codec fmtp string.
     */

     /// Destructor
   ~MpEncoderBase();

     /// Initializes encoder with given fmtp parameters
   OsStatus initEncode(const char* fmt);
     /**<
     *  @retval OS_SUCCESS - Success.
     *  @retval OS_FAILED - Failure.
     */

     /// Initializes encoder with default fmtp parameters
   OsStatus initEncode();
     /**<
     *  @retval OS_SUCCESS - Success.
     *  @retval OS_FAILED - Failure.
     */

     /// Frees all memory allocated to the encoder by initEncode()
   OsStatus freeEncode();
     /**<
     *  @retval OS_SUCCESS - Success.
     *  @retval OS_INVALID_STATE - Object has already been freed.
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Encode audio samples
   OsStatus encode(const MpAudioSample* pAudioSamples,
                   const int numSamples,
                   int& rSamplesConsumed,
                   unsigned char* pCodeBuf,
                   const int bytesLeft,
                   int& rSizeInBytes,
                   UtlBoolean& isPacketReady,
                   UtlBoolean& isPacketSilent,
                   UtlBoolean& shouldSetMarker);
     /**<
     *  Processes the array of audio samples.  If sufficient samples to encode
     *  a frame are now available, the encoded data will be written to the
     *  \p pCodeBuf array.  The number of bytes written to the
     *  \p pCodeBuf array is returned in \p rSizeInBytes.
     *
     *  @param[in]  pAudioSamples - Pointer to array of PCM samples.
     *  @param[in]  numSamples - Number of samples at pAudioSamples.
     *  @param[out] rSamplesConsumed - Number of samples encoded.
     *  @param[out] pCodeBuf - Pointer to array for encoded data.
     *  @param[in]  bytesLeft - Number of bytes available at pCodeBuf.
     *  @param[out] rSizeInBytes - Number of bytes written to the <i>pCodeBuf</i> array.
     *  @param[out] isPacketReady - If TRUE, encoder finished encoding, so packet
     *              may be sent. If DTX is enabled, \p sendNow should be set
     *              to TRUE for all supressed packets too along with 
     *              \p isPacketSilent set to TRUE.
     *  @param[out] isPacketSilent - If TRUE, packet may be not transmitted
     *              if DTX is enabled, if FALSE, packet contain active voice
     *              data and should always be transmitted. Value of
     *              \p isPacketSilent is ignored with \p isPacketReady is set
     *              to FALSE.
     *
     *  @retval OS_SUCCESS - Success.
     */


//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get static information about the encoder
   const MpCodecInfo* getInfo() const;
     /**<
     *  @returns A pointer to an MpCodecInfo object that provides static
     *           information about the encoder.
     */

     /// Returns the RTP payload type associated with this encoder.
   int getPayloadType();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int mPayloadType;           ///< RTP payload type, associated with this codec.
   MpCodecInfo mCodecInfo;     ///< Codec info structure
   void* plgHandle;            ///< Internal codec handle.
   const MpCodecCallInfoV1& mCallInfo; ///< Actual codec's manipulator functions.
   UtlBoolean mInitialized;    ///< Is codec initialized?
   const char* mDefaultFmtp;   ///< Default fmtp string.

     /// Copy constructor
   MpEncoderBase(const MpEncoderBase& rMpEncoderBase);

     /// Assignment operator
   MpEncoderBase& operator=(const MpEncoderBase& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpEncoderBase_h_
