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
#include "mp/MpCodecInfo.h"
#include "mp/MpRtpBuf.h"
#include "mp/MpTypes.h"
#include "os/OsStatus.h"

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
   MpDecoderBase(int payloadType);
     /**<
     *  @param[in] payloadType - RTP payload type associated with this decoder
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

     /// Get information about the decoder.
   virtual const MpCodecInfo* getInfo() const =0;
     /**<
     *  @returns A pointer to a MpCodecInfo object that provides
     *           information about the decoder. For codecs with only one mode
     *           supported (like G.711 and GSM) it may return pointer to
     *           static class member. More complex codecs (like Speex) may
     *           return pointer to usual class member.
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

   int mPayloadType;  ///< RTP payload type associated with this decoder.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDecoderBase_h_
