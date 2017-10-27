//
// Copyright (C) 2007-2017 SIPez LLC.
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

#ifndef _SdpDefaultCodecFactory_h_
#define _SdpDefaultCodecFactory_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <sdp/SdpCodec.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Factory class for generating SDPs for known codec types and
*         converting readable codec names to codec types and vice versa.
*
*  @nosubgrouping
*/
class SdpDefaultCodecFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

     /// Get a codec given an internal codec id.
   static
   SdpCodec getCodec(SdpCodec::SdpCodecTypes internalCodecId);

     /// Converts the readable text codec name into a codec type.
   static
   SdpCodec::SdpCodecTypes getCodecType(const char* pCodecName);

     /// Get specific codec identified by iCodec.
   static
   OsStatus getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName);

     /// Get MIME-subtype and fmtp strings for given codec type.
   static
   OsStatus getMimeInfoByType(SdpCodec::SdpCodecTypes codecType,
                              UtlString& mimeSubtype,
                              UtlString& fmtp);
     /**<
     *  @param[in]  codecType - codec type to look for.
     *  @param[out] mimeSubtype - MIME-subtype string (if found).
     *  @param[out] fmtp - fmtp string (if found).
     *
     *  @retval OS_SUCESS if appropriate mapping found and \p mimeSubtype and
     *          \p fmtp parameters filled with data.
     *  @retval OS_NOT_FOUND if appropriate mapping not found.
     */

     /// Get codec type by MIME-subtype/sample rate/channels number/fmtp combination.
   static
   OsStatus getCodecType(const UtlString &mimeSubtype,
                         unsigned sampleRate,
                         unsigned numChannels,
                         const UtlString &fmtp,
                         SdpCodec::SdpCodecTypes &codecType);
     /**<
     *  @param[in]  mimeSubtype - MIME-subtype string to look for.
     *  @param[in]  sampleRate - sample rate of codec to look for.
     *  @param[in]  numChannels - number of channels of codec to look for.
     *  @param[in]  fmtp - fmtp string to look for.
     *  @param[out] codecType - codec type (if found).
     *
     *  @retval OS_SUCESS if appropriate mapping found and \p codecType set.
     *  @retval OS_NOT_FOUND if appropriate mapping not found.
     */

    /// Get the string names for all of the codec types known by the codec factory
    static int getCodecNames(UtlContainer& codecNames);

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpDefaultCodecFactory_h_
