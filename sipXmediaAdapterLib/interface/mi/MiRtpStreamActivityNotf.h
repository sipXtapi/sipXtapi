//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MiRtpStreamActivityNotf_h_
#define _MiRtpStreamActivityNotf_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"
#include "mi/MiNotification.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  Notification used to communicate RTP stream activity.
*/ 
class MiRtpStreamActivityNotf : public MiNotification
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

   enum StreamState {
      STREAM_START,  ///< Stream have been started
      STREAM_STOP,   ///< Stream have been stopped
      STREAM_CHANGE  ///< Stream attributes have been changed
   };

     /// Constructor
   MiRtpStreamActivityNotf(const UtlString& namedResOriginator,
                           StreamState state,
                           unsigned ssrc,
                           unsigned address,
                           int port,
                           int connId = -1,
                           int streamId = -1);

     /// Copy constructor
   MiRtpStreamActivityNotf(const MiRtpStreamActivityNotf& rMsg);

     /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy() const;

     /// Destructor
   virtual ~MiRtpStreamActivityNotf();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MiRtpStreamActivityNotf& operator=(const MiRtpStreamActivityNotf& rhs);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get the stream state.
   StreamState getState() const;

     /// Get the stream SSRC.
   unsigned getSsrc() const;

     /// Get IP address of the stream source/destination.
   unsigned getAddress() const;

     /// Get port of the stream source/destination.
   int getPort() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   StreamState mState;  ///< Stream state to be reported.
   unsigned mSsrc;      ///< SSRC of the stream.
   unsigned mAddress;   ///< IP of the stream source/destination.
   int mPort;           ///< Port of the stream source/destination.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MiRtpStreamActivityNotf_h_
