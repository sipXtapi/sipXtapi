//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprnRtpStreamActivityMsg_h_
#define _MprnRtpStreamActivityMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"
#include "MpResNotificationMsg.h"

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
class MprnRtpStreamActivityMsg : public MpResNotificationMsg
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
   MprnRtpStreamActivityMsg(const UtlString& namedResOriginator,
                            StreamState state,
                            RtpSRC ssrc,
                            unsigned address,
                            int port,
                            MpConnectionID connId = MP_INVALID_CONNECTION_ID,
                            int streamId = -1);

     /// Copy constructor
   MprnRtpStreamActivityMsg(const MprnRtpStreamActivityMsg& rMsg);

     /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy() const;

     /// Destructor
   virtual ~MprnRtpStreamActivityMsg();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MprnRtpStreamActivityMsg& operator=(const MprnRtpStreamActivityMsg& rhs);

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
   RtpSRC mSsrc;        ///< SSRC of the stream.
   unsigned mAddress;   ///< IP of the stream source/destination.
   int mPort;           ///< Port of the stream source/destination.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprnRtpStreamActivityMsg_h_
