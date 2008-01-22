//  
// Copyright (C) 2006-2008 SIPez LLC. 
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


#ifndef _MpCodecInfo_h_
#define _MpCodecInfo_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/codecs/PlgDefsV1.h"
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Static information describing a codec.
*/
class MpCodecInfo : protected MppCodecInfoV1_1, protected MppCodecFmtpInfoV1_1
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Full constructor
   MpCodecInfo(const MppCodecInfoV1_1 &codecInfo,
               const MppCodecFmtpInfoV1_1 &fmtpInfo);

     /// Partial constructor
   MpCodecInfo(const MppCodecInfoV1_1 &codecInfo);

     /// Destructor
   ~MpCodecInfo();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @copydoc MppCodecInfoV1_1::codecManufacturer
   const char*  getCodecManufacturer() const;
     /// @copydoc MppCodecInfoV1_1::codecName
   const char*  getCodecName() const;
     /// @copydoc MppCodecInfoV1_1::codecVersion
   const char*  getCodecVersion() const;
     /// @copydoc MppCodecInfoV1_1::codecType
   unsigned     getCodecType() const;

     /// @copydoc MppCodecInfoV1_1::mimeSubtype
   const char*  getMimeSubtype() const;
     /// @copydoc MppCodecInfoV1_1::fmtpsNum
   unsigned     getFmtpsNum() const;
     /// @copydoc MppCodecInfoV1_1::fmtps
   const char** getFmtps() const;
     /// @copydoc MppCodecInfoV1_1::sampleRate
   unsigned     getSampleRate() const;
     /// @copydoc MppCodecInfoV1_1::numChannels
   unsigned     getNumChannels() const;
     /// @copydoc MppCodecInfoV1_1::framePacking
   unsigned     getFramePacking() const;

     /// @copydoc MppCodecFmtpInfoV1_1::minBitrate
   unsigned     getMinBitrate() const;
     /// @copydoc MppCodecFmtpInfoV1_1::maxBitrate
   unsigned     getMaxBitrate() const;
     /// @copydoc MppCodecFmtpInfoV1_1::numSamplesPerFrame
   unsigned     getNumSamplesPerFrame() const;
     /// @copydoc MppCodecFmtpInfoV1_1::minFrameBytes
   unsigned     getMinFrameBytes() const;
     /// @copydoc MppCodecFmtpInfoV1_1::maxFrameBytes
   unsigned     getMaxFrameBytes() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// @copydoc MppCodecFmtpInfoV1_1::signalingCodec
   UtlBoolean isSignalingCodec(void) const;
     /// @copydoc MppCodecFmtpInfoV1_1::doesVadCng
   UtlBoolean doesVadCng(void) const;
     /// @copydoc MppCodecFmtpInfoV1_1::haveInternalPLC
   UtlBoolean haveInternalPLC() const;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCodecInfo_h_
