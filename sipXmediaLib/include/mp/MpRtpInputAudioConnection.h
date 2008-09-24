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


#ifndef _MpRtpInputAudioConnection_h_
#define _MpRtpInputAudioConnection_h_

// FORWARD DECLARATIONS
class MpFlowGraphBase;
class MpDecoderBase;
class MpResource;
class MprDecode;
class SdpCodec;
class OsNotification;

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpRtpInputConnection.h"
#include "mp/MpResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Connection container for audio part of call.
*/
class MpRtpInputAudioConnection : public MpRtpInputConnection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpRtpInputAudioConnection(const UtlString& resourceName,
                             MpConnectionID myID,
                             const UtlString &plcName = "");

     /// Destructor
   virtual
   ~MpRtpInputAudioConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Process one frame of audio
   UtlBoolean processFrame();

     /// Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
   UtlBoolean handleSetDtmfNotify(OsNotification* n);
     /**<
     *  @todo This should become a resource message handled by the resource!
     *
     *  @Returns <b>TRUE</b>
     */

     /// Queue a message to start receiving RTP and RTCP packets.
   static OsStatus startReceiveRtp(OsMsgQ& messageQueue,
                                   const UtlString& resourceName,
                                   SdpCodec* pCodecs[], 
                                   int numCodecs,
                                   OsSocket& rRtpSocket, 
                                   OsSocket& rRtcpSocket);


     /// Queue a message to stop receiving RTP and RTCP packets.
   static OsStatus stopReceiveRtp(OsMsgQ& messageQueue,
                                  const UtlString& resourceName);
   
     /// Change PLC algorithm to one with given name.
   static OsStatus setPlc(const UtlString& namedResource,
                          OsMsgQ& fgQ,
                          const UtlString& plcName = "");
     /**<
     *  Sends an MPRM_SET_PLC message to the named MpRtpInputAudioConnection
     *  resource within the flowgraph who's queue is supplied. When the message 
     *  is received, the above resource will change PLC algorithm to
     *  chosen one.
     *
     *  THIS IS A HACK! This function should not live here, it should be in
     *  MprDecode. But MprDecode is not part of flowgraph thus could not handle
     *  messages. We have to solve this someday.
     *
     *  @param[in] namedResource - the name of the resource to send a message to.
     *  @param[in] fgQ - the queue of the flowgraph containing the resource which
     *             the message is to be received by.
     *  @param[in] plcName - name of PLC algorithm to use.
     *             See MpJitterBuffer::setPlc() for more details.
     *  @returns the result of attempting to queue the message to this resource.
     */

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
   
   typedef enum
   {
      MPRM_SET_PLC = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START
   } AddlResMsgTypes;

     /// Handles an incoming resource message for this media processing object.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);
     /**<
     *  @returns TRUE if the message was handled, otherwise FALSE.
     */

     /// Perform the enable operation specific to the MpRtpInputAudioConnection
   virtual UtlBoolean handleEnable();

     /// Perform the disable operation specific to the MpRtpInputAudioConnection
   virtual UtlBoolean handleDisable();

     /// Starts receiving RTP and RTCP packets.
   void handleStartReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                              OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Stops receiving RTP and RTCP packets.
   void handleStopReceiveRtp(void);

     /// Sets the flowgraph in any child resources of this resource.
   virtual OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);
     /**<
     *  @param[in] pFlowGraph - pointer to a flowgraph to store for future use.
     *  
     *  @see MpResource::setFlowGraph()
     */

     /// Sets the notification enabled status in any child resources of this resource.
   virtual OsStatus setNotificationsEnabled(UtlBoolean enable);
     /**<
     *  @see MpResource::setNotificationsEnabled()
     */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MprDecode*         mpDecode;        ///< Inbound component: Decoder

     /// Copy constructor (not implemented for this type)
   MpRtpInputAudioConnection(const MpRtpInputAudioConnection& rMpRtpInputAudioConnection);

     /// Assignment operator (not implemented for this type)
   MpRtpInputAudioConnection& operator=(const MpRtpInputAudioConnection& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRtpInputAudioConnection_h_
