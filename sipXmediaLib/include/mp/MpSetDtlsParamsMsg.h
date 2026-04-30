//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpSetDtlsParamsMsg_h_
#define _MpSetDtlsParamsMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpDtls;

/// Message used to wire an MpDtls handshake engine into the MprFromNet
/// (inbound) and MprToNet (outbound) network resources for a connection.
///
/// Posted by CpTopologyGraphInterface::setDtlsSrtpParams() and consumed
/// by MpRtpInputConnection::handleMessage and
/// MpRtpOutputConnection::handleMessage. The same MpDtls* is delivered
/// to both connections (with different target resource names) so that:
///   - MprFromNet learns it must demultiplex inbound packets per RFC
///     7983 and feed DTLS records to the engine.
///   - MprToNet learns it must drop outbound RTP until the handshake
///     completes.
///
/// The MpDtls* is borrowed; the message does not own it. Its lifetime
/// is tied to the owning CpTopologyMediaConnection.
class MpSetDtlsParamsMsg : public MpResourceMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
    MpSetDtlsParamsMsg(const UtlString& targetResourceName,
                       MpDtls* pDtls)
      : MpResourceMsg(MPRM_SET_DTLS_PARAMS, targetResourceName)
      , mpDtls(pDtls)
   {};

     /// Copy constructor
    MpSetDtlsParamsMsg(const MpSetDtlsParamsMsg& resourceMsg)
      : MpResourceMsg(resourceMsg)
      , mpDtls(resourceMsg.mpDtls)
   {};

     /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const
   {
      return(new MpSetDtlsParamsMsg(*this));
   }

     /// Destructor
   ~MpSetDtlsParamsMsg() {};

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MpSetDtlsParamsMsg& operator=(const MpSetDtlsParamsMsg& rhs)
   {
      if(&rhs == this)
      {
         return(*this);
      }

      MpResourceMsg::operator=(rhs);
      mpDtls = rhs.mpDtls;

      return *this;
   }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   MpDtls* getDtls() const {return mpDtls;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    MpDtls* mpDtls;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpSetDtlsParamsMsg_h_
