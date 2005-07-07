// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
 

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "cp/CpMediaInterface.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInterface::CpMediaInterface()
{
}

// Copy constructor
CpMediaInterface::CpMediaInterface(const CpMediaInterface& rCpMediaInterface)
{
}

// Destructor
CpMediaInterface::~CpMediaInterface()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
CpMediaInterface& 
CpMediaInterface::operator=(const CpMediaInterface& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/*
OsStatus CpMediaInterface::createConnection(int& connectionId)
{
   OsStatus returnCode;

   returnCode = OS_NOT_YET_IMPLEMENTED;
   
   return(returnCode);
}
*/

//? we need mConnectionId for the function 
OsStatus CpMediaInterface::createConnection(int& connectionId, int localPort)
{
    /*
   OsStatus returnCode;
   if(mConnectionId == -1)
   {
      mConnectionId = 1;
      connectionId = mConnectionId;

      // Create the sockets
	   mpRtpSocket = new OsDatagramSocket(0, NULL, mNextRtpPort);
		mpRtcpSocket = new OsDatagramSocket(0, NULL, mNextRtpPort + 1);
		while(!mpRtpSocket->isOk() || !mpRtcpSocket->isOk())
		{
			mNextRtpPort +=2;
			delete mpRtpSocket;
			delete mpRtcpSocket;
			mpRtpSocket = new OsDatagramSocket(0, NULL, mNextRtpPort);
			mpRtcpSocket = new OsDatagramSocket(0, NULL, mNextRtpPort + 1);
		}
      OsSocket::getHostIp(&mRtpAddress);
      mRtpPort = mNextRtpPort;
      mRtcpPort = mNextRtpPort + 1;
      returnCode = OS_SUCCESS;
   }
   else
   {
      // Only allow one connection
      returnCode = OS_NOT_YET_IMPLEMENTED;
   }

   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

//:TODO: we probably need the mNumCodecs for the MediaInterface 
int CpMediaInterface::getNumCodecs(int connectionId)
{
//   return(mNumCodecs);
    return 0;
}

OsStatus CpMediaInterface::getCapabilities(int connectionId,
                                            UtlString& rtpHostAddress, 
                                            int& rtpPort,
                                            int& rtcpPort,
                                            SdpCodecFactory& supportedCodecs)
{
    /*
   OsStatus returnCode;
   rtpHostAddress.remove(0);
   if(connectionId != mConnectionId)
   {
      returnCode = OS_INVALID_ARGUMENT;
   }
   else
   {
      rtpHostAddress.append(mRtpAddress.data());
      rtpPort = mRtpPort;
      int codecIndex;
      for(codecIndex = 0; 
         codecIndex < mNumCodecs && codecIndex < maxNumCodecs;
         codecIndex++)
      {
         aCodecs[codecIndex] = maCodecs[codecIndex];
      }
      numCodecs = codecIndex;
      if(codecIndex < mNumCodecs)
      {
         returnCode = OS_LIMIT_REACHED;
      }
      else
      {
         returnCode = OS_SUCCESS;
      }
   }

   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::setConnectionDestination(int connectionId,
                                                         const char* remoteRtpHostAddress, 
                                                         int remoteRtpPort)
{
    /*
   OsStatus returnCode = OS_NOT_FOUND;
   if(remoteRtpHostAddress && strlen(remoteRtpHostAddress) != 0)
   {
       mDestinationSet = TRUE;
       if(mpRtpSocket && mConnectionId == connectionId)
       {
          mpRtpSocket->doConnect(remoteRtpPort, remoteRtpHostAddress, TRUE);
          returnCode = OS_SUCCESS;
       }
       if(mpRtcpSocket && mConnectionId == connectionId)
       {
          mpRtcpSocket->doConnect(remoteRtpPort + 1, remoteRtpHostAddress, TRUE);
       }
   }
   else
   {
       osPrintf("CpMediaInterface::setConnectionDestination with zero length host address\n");
   }

   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::startRtpSend(int connectionId, 
                                        int numCodecs,
                                        SdpCodec* sendCodec[])
{
    /*
   OsStatus returnCode = OS_NOT_FOUND;
   if(mpFlowGraph && mConnectionId == connectionId)
   {
      mpFlowGraph->startSendRtp(sendCodec, 
					        *mpRtpSocket, *mpRtcpSocket);
      mRtpSending = TRUE;
      returnCode = OS_SUCCESS;
   }
   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::startRtpReceive(int connectionId,
                                           int numCodecs,
                                           SdpCodec* sendCodec[])
{
#if 0
   OsStatus returnCode = OS_NOT_FOUND;
   if(mpFlowGraph && mConnectionId == connectionId)
   {
      mpFlowGraph->startReceiveRtp(receiveCodec, 
                                   *mpRtpSocket, *mpRtcpSocket);
      mRtpReceiving = TRUE;
      returnCode = OS_SUCCESS;
   }
   return(returnCode);
#endif /* 0 */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::stopRtpSend(int connectionId)
{
    /*
   OsStatus returnCode = OS_NOT_FOUND;
   if(mpFlowGraph && mConnectionId == connectionId)
   {
      mpFlowGraph->stopReceiveRtp();
      mRtpSending = FALSE;
      returnCode = OS_SUCCESS;
   }
   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::stopRtpReceive(int connectionId)
{
    /*
   OsStatus returnCode = OS_NOT_FOUND;
   if(mpFlowGraph && mConnectionId == connectionId)
   {
      mpFlowGraph->stopSendRtp();
      mRtpReceiving = FALSE;
      returnCode = OS_SUCCESS;
   }
   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::deleteConnection(int connectionId)
{
    /*
   OsStatus returnCode = OS_NOT_FOUND;
   if(connectionId > 0 && connectionId == mConnectionId)
   {
       returnCode = OS_SUCCESS;
       mDestinationSet = FALSE;
       if(mRtpSending)
       {
          returnCode = stopRtpSend(mConnectionId);
       }
       if(mRtpReceiving)
       {
          returnCode = stopRtpReceive(mConnectionId);
       }
       if(mpRtpSocket)
       {
          delete mpRtpSocket;
          mpRtpSocket = NULL;
       }
       if(mpRtcpSocket)
       {
          delete mpRtcpSocket;
          mpRtcpSocket = NULL;
       }
       mConnectionId = -2;
   }
   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}



OsStatus CpMediaInterface::startDtmf(char toneId, 
                                          UtlBoolean local, 
                                          UtlBoolean remote)
{
    /*
   OsStatus returnCode = OS_SUCCESS;
   if(mpFlowGraph)
   {
      int toneDestination;
      if(remote)
      {
         toneDestination = MpCallFlowGraph::TONE_TO_NET;
      }
      else
      {
         toneDestination = MpCallFlowGraph::TONE_TO_SPKR;
      }
      mpFlowGraph->startTone(toneId, toneDestination);
		// Make sure the DTMF tone is on the minimum length
      OsTask::delay(MINIMUM_DTMF_LENGTH);
   }

   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::stopDtmf()
{
    /*
   OsStatus returnCode = OS_SUCCESS;
   if(mpFlowGraph)
   {
      mpFlowGraph->stopTone();
		// Make sure the DTMF tone is on the minimum length
      OsTask::delay(MINIMUM_DTMF_LENGTH);
   }

   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::startRinger(UtlBoolean local, 
                                            UtlBoolean remote)
{
    /*
   OsStatus returnCode = OS_SUCCESS;
   if(mpFlowGraph)
   {

      // Start the ringer
      if(local)
      {
         if(mpFlowGraph->playFile(RING_TONE_FILE_NAME, 
             TRUE, // Repeat
             MpCallFlowGraph::TONE_TO_SPKR) == OS_SUCCESS)
         {
             osPrintf("Opened ring tone file: \"%s\"\n",
                 RING_TONE_FILE_NAME);
             mRingToneFromFile = TRUE;
         }
         else
         {
             osPrintf("Failed to open ring tone file: \"%s\"\n",
                 RING_TONE_FILE_NAME);
             mRingToneFromFile = FALSE;
            mpFlowGraph->startTone(DTMF_TONE_RINGTONE, 
                MpCallFlowGraph::TONE_TO_SPKR);
         }

         // Enable the speaker for the ringer
         // We probably need to keep a state of what is enable
         // such that we can restore back to the state.
         PsPhoneTask* phoneSet = PsPhoneTask::getPhoneTask();
         phoneSet->speakerModeEnable(PsPhoneTask::RINGER_ENABLED);
      }

      // Ignore remote as we cannot currently do different tones
      // on the local and remote tone generator.
      // Ringback tone goes here for remote.
   }

   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::stopRinger()
{
    /*
   OsStatus returnCode = OS_SUCCESS;
   if(mpFlowGraph)
   {
       if(mRingToneFromFile)
       {
           mpFlowGraph->stopFile(TRUE);
       }
       else
       {
          mpFlowGraph->stopTone();
       }

      // Disable the speaker for the ringer
      PsPhoneTask* phoneSet = PsPhoneTask::getPhoneTask();
      phoneSet->speakerModeDisable(PsPhoneTask::RINGER_ENABLED);
   }

   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::startTone(int toneId, 
                                          UtlBoolean local, 
                                          UtlBoolean remote)
{
    /*
   OsStatus returnCode = OS_SUCCESS;
   if(mpFlowGraph)
   {
      int toneDestination;
      if(remote)
      {
         toneDestination = MpCallFlowGraph::TONE_TO_NET;
      }
      else
      {
         toneDestination = MpCallFlowGraph::TONE_TO_SPKR;
      }
      mpFlowGraph->startTone(toneId, toneDestination);
   }

   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::stopTone()
{
    /*
   OsStatus returnCode = OS_SUCCESS;
   if(mpFlowGraph)
   {
      mpFlowGraph->stopTone();
   }

   return(returnCode);
   */
    return OS_NOT_YET_IMPLEMENTED;
}




void CpMediaInterface::giveFocus()
{/*
    if(mpFlowGraph)
	{
        // There should probably be a lock here
	    // Set the flow graph to have the focus
	    MpMediaTask* mediaTask = MpMediaTask::getMediaTask();
	    mediaTask->setFocus(mpFlowGraph);
        osPrintf("Setting focus for flow graph\n");
   }
   */
}

void CpMediaInterface::defocus()
{
    /*
    if(mpFlowGraph)
	{
		MpMediaTask* mediaTask = MpMediaTask::getMediaTask();

        // There should probably be a lock here
		// take focus away from the flow graph if it is focus
		if(mpFlowGraph == (MpCallFlowGraph*) mediaTask->getFocus())
		{
		    mediaTask->setFocus(NULL);
            osPrintf("Setting NULL focus for flow graph\n");
		}
   }
   */
}

/* ============================ ACCESSORS ================================= */
UtlBoolean CpMediaInterface::isSendingRtp(int connectionId)
{
    /*
   UtlBoolean sending = FALSE;
   if(mConnectionId > 0 && mConnectionId == connectionId)
   {
       sending = mRtpSending;
   }
   else
   {
       osPrintf("CpMediaInterface::isSendingRtp invalid connectionId: %d\n");
   }

   return(sending);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

UtlBoolean CpMediaInterface::isReceivingRtp(int connectionId)
{
    /*
   UtlBoolean receiving = FALSE;
   if(mConnectionId > 0 && mConnectionId == connectionId)
   {
      receiving = mRtpReceiving;
   }
   else
   {
       osPrintf("CpMediaInterface::isReceivingRtp invalid connectionId: %d\n");
   }
   return(receiving);
   */
    return OS_NOT_YET_IMPLEMENTED;
}

UtlBoolean CpMediaInterface::isDestinationSet(int connectionId)
{
    /*
    UtlBoolean isSet = FALSE;
    if(mConnectionId > 0 && connectionId == mConnectionId)
    {
        isSet = mDestinationSet;
    }
    else
    {
       osPrintf("CpMediaInterface::isDestinationSet invalid connectionId: %d\n");
    }
    return(isSet);
    */
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::playAudio(const char* url, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote)
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::pauseAudio()
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::stopAudio()
{
    return OS_NOT_YET_IMPLEMENTED;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

