//  
// Copyright (C) 2026 SIP Spectrum, Inc.  Al rights reserved.
//  
// Copyright (C) 2007-2011 SIPez LLC.  Al rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpSetSrtpParamsMsg_h_
#define _MpSetSrtpParamsMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"
#include <sdp/SdpMediaLine.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message used to set SRTP parameters on MprFromNet and MprToNet resources
class MpSetSrtpParamsMsg : public MpResourceMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
    MpSetSrtpParamsMsg(const UtlString& targetResourceName,
                       SdpMediaLine::SdpCryptoSuiteType cryptoSuite, 
                       const UtlString& cryptoKey)
      : MpResourceMsg(MPRM_SET_SRTP_PARAMS, targetResourceName)
      , mCryptoSuite(cryptoSuite)
      , mCryptoKey(cryptoKey)
   {};

     /// Copy constructor
    MpSetSrtpParamsMsg(const MpSetSrtpParamsMsg& resourceMsg)
      : MpResourceMsg(resourceMsg)
      , mCryptoSuite(resourceMsg.mCryptoSuite)
      , mCryptoKey(resourceMsg.mCryptoKey)
   {};

     /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return(new MpSetSrtpParamsMsg(*this));
   }

     /// Destructor
   ~MpSetSrtpParamsMsg() {};

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MpSetSrtpParamsMsg& operator=(const MpSetSrtpParamsMsg& rhs)
   {
      if(&rhs == this)
      {
         return(*this);
      }

      MpResourceMsg::operator=(rhs);
      mCryptoSuite = rhs.mCryptoSuite;
      mCryptoKey = rhs.mCryptoKey;

      return *this;
   }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   SdpMediaLine::SdpCryptoSuiteType getCryptoSuite() const {return mCryptoSuite;}
   const UtlString& getCryptoKey() const {return mCryptoKey;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SdpMediaLine::SdpCryptoSuiteType mCryptoSuite;
    UtlString mCryptoKey;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpSetSocketsMsg_h_
