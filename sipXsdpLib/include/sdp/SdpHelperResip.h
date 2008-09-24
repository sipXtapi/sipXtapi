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

#ifndef _SdpHelperResip_h_
#define _SdpHelperResip_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#ifdef BUILD_RESIP_SDP_HELPER
#include <resip/stack/SdpContents.hxx>
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

//: Container for SdpHelperResip specification
// This class holds the information related to an SdpHelperResip.
// Included in this information is:  TODO
//
namespace resip
{
   class ParseBuffer;
   class Data;
}

class SdpHelperResip
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */
   static Sdp::SdpAddressType convertResipAddressType(resip::SdpContents::AddrType resipAddrType);
   static SdpMediaLine::SdpEncryptionMethod convertResipEncryptionMethod(resip::SdpContents::Session::Encryption::KeyType resipMethod);
   static Sdp* createSdpFromResipSdp(const resip::SdpContents& resipSdp);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static void parseCryptoParams(resip::ParseBuffer& pb, 
                                 SdpMediaLine::SdpCryptoKeyMethod& keyMethod, 
                                 resip::Data& keyValue, 
                                 unsigned int& srtpLifetime, 
                                 unsigned int& srtpMkiValue, 
                                 unsigned int& srtpMkiLength);
};

/* ============================ INLINE METHODS ============================ */
#endif  // BUILD_RESIP_SDP_HELPER
#endif  // _SdpHelperResip_h_
