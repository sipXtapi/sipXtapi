//
// Copyright (C) 2007-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2007 Plantronics
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Scott Godin (sgodin AT SipSpectrum DOT com)

#ifndef _SdpHelper_h_
#define _SdpHelper_h_

#ifndef EXCLUDE_SIPX_SDP_HELPER

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SdpBody.h>
#include <sdp/Sdp.h>
#include <sdp/SdpMediaLine.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Container for SdpHelper specification
// This class holds the information related to an SdpHelper.
// Included in this information is:  TODO
//

class SdpHelper
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */
   static SdpMediaLine::SdpCryptoSuiteType convertCryptoSuiteType(int sdpBodyType);

   static Sdp* createSdpFromSdpBody(const SdpBody& sdpBody, const SdpCodecList* codecFactory);

   // Fill in the SdpMediaLine container for the indicated media line in the given SdpBody
   static UtlBoolean getMediaLine(const SdpBody& sdpBody, int mediaLineIndex, 
                                  SdpMediaLine& mediaLine,
                                  const SdpCodecList* codecFactory);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif // #ifndef EXCLUDE_SIPX_SDP_HELPER

#endif  // _SdpHelper_h_

