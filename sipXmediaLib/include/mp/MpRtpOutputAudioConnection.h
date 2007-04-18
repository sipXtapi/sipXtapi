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
   MpRtpOutputAudioConnection(UtlString& resourceName,
                              MpConnectionID myID, 
                              int samplesPerFrame, 
                              int samplesPerSec);

     /// Destructor
   virtual
   ~MpRtpOutputAudioConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   /// Process one frame of audio
   UtlBoolean processFrame(void);

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

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// @brief perform the enable operation specific to the MpRtpInputAudioConnection
   virtual UtlBoolean handleEnable();

   /// @brief perform the disable operation specific to the MpRtpInputAudioConnection
   virtual UtlBoolean handleDisable();

   /// @brief This method does the real work for the media processing resource and 
   /// must be defined in each class derived from this one.
   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Default constructor
   MpRtpOutputAudioConnection();

     /// Copy constructor (not implemented for this type)
   MpRtpOutputAudioConnection(const MpRtpOutputAudioConnection& rMpRtpOutputAudioConnection);

     /// Assignment operator (not implemented for this type)
   MpRtpOutputAudioConnection& operator=(const MpRtpOutputAudioConnection& rhs);

   MprEncode*         mpEncode;        ///< Outbound component: Encoder
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRtpOutputAudioConnection_h_
