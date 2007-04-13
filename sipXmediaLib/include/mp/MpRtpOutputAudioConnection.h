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


#ifndef _MpRtpOutputAudioConnection_h_
#define _MpRtpOutputAudioConnection_h_

// FORWARD DECLARATIONS
class MpFlowGraphBase;
class MpResource;
class MprEncode;

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpRtpOutputConnection.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/**
*  @brief Connection container for audio part of call.
*/
class MpRtpOutputAudioConnection : public MpRtpOutputConnection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpRtpOutputAudioConnection(MpConnectionID myID, 
                              MpFlowGraphBase* pParent,
                              int samplesPerFrame, 
                              int samplesPerSec);

     /// Destructor
   virtual
   ~MpRtpOutputAudioConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// Enables the output path of the connection.
   virtual OsStatus enable();
     /**<
     *  @see See MpConnection::enable() for more information.
     */

     /// Starts sending RTP and RTCP packets.
   void startSendRtp(OsSocket& rRtpSocket, OsSocket& rRtcpSocket,
                     SdpCodec* pPrimary, SdpCodec* pDtmf);

     /// Starts sending RTP and RTCP packets.
   void startSendRtp(SdpCodec& rCodec,
                     OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Stops sending RTP and RTCP packets.
   void stopSendRtp();

     /// Start sending DTMF tone.
   void startTone(int toneId);

     /// Stop sending DTMF tone.
   void stopTone(void);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Returns the resource to link to upstream resource's outPort.
   MpResource* getSinkResource(void);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Default constructor
   MpRtpOutputAudioConnection();

     /// Copy constructor (not implemented for this type)
   MpRtpOutputAudioConnection(const MpRtpOutputAudioConnection& rMpRtpOutputAudioConnection);

     /// Assignment operator (not implemented for this type)
   MpRtpOutputAudioConnection& operator=(const MpRtpOutputAudioConnection& rhs);

   MpFlowGraphBase*   mpFlowGraph;     ///< Parent flowgraph
   MprEncode*         mpEncode;        ///< Outbound component: Encoder
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRtpOutputAudioConnection_h_
