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
#ifndef _CpPhoneMediaInterface_h_
#define _CpPhoneMediaInterface_h_

// SYSTEM INCLUDES
//#include <>

// APPLICATION INCLUDES
#include <cp/CpMediaInterface.h>
#include <cp/Connection.h>
#include <os/OsStatus.h>
#include <os/OsDefs.h>

#include <net/QoS.h>
#include <net/SdpCodecFactory.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpCallFlowGraph;
class SdpCodec;
class OsDatagramSocket;
class CpPhoneMediaConnection;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CpPhoneMediaInterface : public CpMediaInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum OutputAudioDevice
   {
      UNKNOWN = 0x0,
      HANDSET = 0x1,
      SPEAKER = 0x2,
      HEADSET = 0x4
   };

/* ============================ CREATORS ================================== */

   CpPhoneMediaInterface(int startRtpPort = 8766, 
                         int lastRtpPort = 8800,
                         const char* publicAddress = NULL, 
                         const char* localAddress = NULL,
                         int numCodecs = 0, 
                         SdpCodec* sdpCodecArray[] = NULL,
                         const char* pLocale = "",
                         int expeditedIpTos = QOS_LAYER3_LOW_DELAY_IP_TOS,
                         const char* szStunServer = NULL,
                         int iStunKeepAlivePeriodSecs = 28);
     //:Default constructor

   CpPhoneMediaInterface(const CpPhoneMediaInterface& rCpPhoneMediaInterface);
     //:Copy constructor

   virtual
   ~CpPhoneMediaInterface();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   CpPhoneMediaInterface& operator=(const CpPhoneMediaInterface& rhs);
     //:Assignment operator


   virtual OsStatus createConnection(int& connectionId, int localPort = 0);

   virtual int getNumCodecs(int connectionId);
   virtual OsStatus getCapabilities(int connectionId, 
                                    UtlString& rtpHostAddress, 
                                    int& rtpPort,
                                    int& rtcpPort,
                                    SdpCodecFactory& supportedCodecs);

   virtual OsStatus setConnectionDestination(int connectionId,
                                             const char* rtpHostAddress, 
                                             int rtpPort);

   virtual OsStatus startRtpSend(int connectionId, 
                                 int numCodecs,
                                 SdpCodec* sendCodec[]);
   virtual OsStatus startRtpReceive(int connectionId,
                                    int numCodecs,
                                    SdpCodec* sendCodec[]);
   virtual OsStatus stopRtpSend(int connectionId);
   virtual OsStatus stopRtpReceive(int connectionId);

   virtual OsStatus deleteConnection(int connectionId);

   virtual OsStatus startDtmf(char toneId, UtlBoolean local, UtlBoolean remote);
   virtual OsStatus stopDtmf();

   //virtual OsStatus startRinger(UtlBoolean local, UtlBoolean remote);
   //virtual OsStatus stopRinger();

   virtual OsStatus startTone(int toneId, UtlBoolean local, UtlBoolean remote);
   virtual OsStatus stopTone();

   virtual OsStatus playAudio(const char* url, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote);
   virtual OsStatus playBuffer(char* buf, 
                               unsigned long bufSize,
                               int type, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote,
                              OsNotification* event = NULL);
   virtual OsStatus pauseAudio();
   virtual OsStatus stopAudio();

   virtual OsStatus createPlayer(MpStreamPlayer** ppPlayer, 
                                 const char* szStream, 
                                 int flags, 
                                 OsMsgQ *pMsgQ = NULL, 
                                 const char* szTarget = NULL) ;
   virtual OsStatus destroyPlayer(MpStreamPlayer* pPlayer);
   virtual OsStatus createPlaylistPlayer(MpStreamPlaylistPlayer** 
                                         ppPlayer, 
                                         OsMsgQ *pMsgQ = NULL, 
                                         const char* szTarget = NULL);
   virtual OsStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer);
   virtual OsStatus createQueuePlayer(MpStreamQueuePlayer** ppPlayer, 
                                      OsMsgQ *pMsgQ = NULL, 
                                      const char* szTarget = NULL);
   virtual OsStatus destroyQueuePlayer(MpStreamQueuePlayer* pPlayer);

   virtual void giveFocus();
   virtual void defocus();

   virtual void setCodecCPULimit(int iLimit);
     //:Limits the available codecs to only those within the designated
     //:limit.


   virtual void addToneListener(OsNotification *pListener, int connectionId);
   virtual void removeToneListener(int connectionId);

   virtual OsStatus stopRecording();
   virtual OsStatus ezRecord(int ms, 
           int silenceLength, 
           const char* fileName, 
           double& duration, 
           int& dtmfterm,
           OsProtectedEvent* ev = NULL);

   virtual void setContactType(int connectionId, CONTACT_TYPE eType) ;
 
/* ============================ ACCESSORS ================================= */

   virtual void setPremiumSound(UtlBoolean enabled);
   virtual int getCodecCPUCost();
      //:Calculate the current cost for our sending/receiving codecs

   virtual int getCodecCPULimit();
      //:Calculate the worst cost for our sending/receiving codecs


   virtual OsMsgQ* getMsgQ();
     //:Returns the flowgraph's message queue

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isSendingRtp(int connectionId);
   virtual UtlBoolean isReceivingRtp(int connectionId);
   virtual UtlBoolean isDestinationSet(int connectionId);   

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    CpPhoneMediaConnection* getMediaConnection(int connecctionId);
    CpPhoneMediaConnection* removeMediaConnection(int connecctionId);
    OsStatus doDeleteConnection(CpPhoneMediaConnection* mediaConnection);

   int mNextRtpPort;
   int mLastRtpPort;
   UtlString mRtpReceiveHostAddress; // Advertized as place to send RTP/RTCP
   UtlString mLocalAddress; // On which ports are bound
   MpCallFlowGraph* mpFlowGraph;
   UtlBoolean mRingToneFromFile;
   SdpCodecFactory mSupportedCodecs;
   UtlDList mMediaConnections;
   int mExpeditedIpTos;
   UtlString mStunServer ;
   int mStunRefreshPeriodSecs ;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpPhoneMediaInterface_h_
