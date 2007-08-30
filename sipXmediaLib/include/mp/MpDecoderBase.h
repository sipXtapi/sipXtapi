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
                 const MpCodecInfo* pInfo);
     /**<
     *  @param[in] payloadType - RTP payload type associated with this decoder
     *  @param[in] pInfo - Pointer to derived class' static const MpCodecInfo
     */

     /// Destructor
   virtual ~MpDecoderBase();

     /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initDecode() = 0;
     /**<
     *  @retval OS_SUCCESS - Success
     *  @retval OS_NO_MEMORY - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeDecode(void) = 0;
     /**<
     *  @retval OS_SUCCESS - Success
     *  @retval OS_DELETED - Object has already been deleted
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Decode incoming RTP packet
   virtual int decode(const MpRtpBufPtr &pPacket,
                      unsigned decodedBufferLength,
                      MpAudioSample *samplesBuffer) =0;
     /**<
     *  @param[in] pPacket - Pointer to a media buffer
     *  @param[in] decodedBufferLength - Length of the samplesBuffer (in samples)
     *  @param[out] samplesBuffer - Buffer for decoded samples
     *  @returns Number of decoded samples.
     */

     /// Set notification to be fired when DTMF tone received.
   virtual UtlBoolean setDtmfNotify(OsNotification* pNotify);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get static information about the decoder
   virtual const MpCodecInfo* getInfo(void) const;
     /**<
     *  @returns a pointer to an MpCodecInfo object that provides
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
