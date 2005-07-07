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

#ifndef _CpMediaInterface_h_
#define _CpMediaInterface_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsDefs.h>
#include <os/OsProtectEvent.h>
#include <os/OsMsgQ.h>
#include <cp/Connection.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SdpCodec;
class SdpCodecFactory;
class MpStreamPlaylistPlayer;
class MpStreamPlayer;
class MpStreamQueuePlayer;

//! Abstract media control interface
/*! The CpCallManager creates a CpMediaInterface for each
 * call created.  The media inteface is then used to control
 * and query the media sub-system used for that call.  As
 * connections are added to the call the media inteface is
 * used to add those connections to the media control system
 * such that all connections in that call are bridged together.
 * \par
 * This abstract class must be sub-classed and implemented to
 * replace the default media sub-system.
 */
class CpMediaInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   //! Default constructor
   CpMediaInterface();

   virtual
   ~CpMediaInterface();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   //! Create a media connection in the media processing subsystem for this call.
   virtual OsStatus createConnection(int& connectionId, int localPort = 0);

   //! Get the number of supported codecs for this call.
   virtual int getNumCodecs(int connectionId);

   //! Get the codec capabilities for the specified media connection 
   //! in this call. 
   /*! Also get the address and port upon which the RTP stream is 
    * to be received.
    */
   virtual OsStatus getCapabilities(int connectionId, 
                                    UtlString& rtpHostAddress, 
                                    int& rtpPort,
                                    int& rtcpPort,
                                    SdpCodecFactory& supportedCodecs);

   //! Set the destination address and port for sending RTP for 
   //! the specified media connection in this call.
   virtual OsStatus setConnectionDestination(int connectionId,
                                             const char* rtpHostAddress, 
                                             int rtpPort);

   //! Start sending RTP using the specified codec.
   virtual OsStatus startRtpSend(int connectionId, 
                                 int numCodecs,
                                 SdpCodec* sendCodec[]);

   //! Start receiving RTP on the specified connection containing
   //! any of the specified codec payload types. 
   /*! The media connection should be prepared to receive any of 
    * the specified payload types without additional signaling.
    */
   virtual OsStatus startRtpReceive(int connectionId,
                                    int numCodecs,
                                    SdpCodec* sendCodec[]);

   //! Stop sending RTP on the specified connection of this call.
   virtual OsStatus stopRtpSend(int connectionId);

   //! Stop receiving RTP on the specified connection of this call.
   virtual OsStatus stopRtpReceive(int connectionId);

   //! Delete the specified connection.
   virtual OsStatus deleteConnection(int connectionId);

   //! Start playing the indicated DTMF signal on the call 
   //! associated with this media interface.
   virtual OsStatus startDtmf(char toneId, UtlBoolean local, UtlBoolean remote);

   //! Stop playing DTMF on the media for this call.
   virtual OsStatus stopDtmf();

   //! Start playing the ringer device.
   virtual OsStatus startRinger(UtlBoolean local, UtlBoolean remote);

   //! Stop playing the ringer device.
   virtual OsStatus stopRinger();

   //! Start playing the specified tone on the associated call.
   /*! (for example ring back, busy, fast busy, beep, etc.) 
    */
   virtual OsStatus startTone(int toneId, UtlBoolean local, UtlBoolean remote);

   //! Stop playing the specified tone.
   virtual OsStatus stopTone();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
   //! Deprecated, use the player objects
   virtual OsStatus playAudio(const char* url, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote);
  //! Deprecated, use the player objects
   virtual OsStatus playBuffer(char* buf, 
                               unsigned long bufSize,
                               int type, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote,
                              OsNotification* event = NULL) = 0;
  //! Deprecated, use the player objects
   virtual OsStatus pauseAudio();

  //! Deprecated, use the player objects
   virtual OsStatus stopAudio();
#endif // DOXYGEN_SHOULD_SKIP_THIS

   //! Create a simple player for this call to play a single stream 
   /*! (see the CpCallManager createPlayer() method).
    */
   virtual OsStatus createPlayer(MpStreamPlayer** ppPlayer, 
                                 const char* szStream, 
                                 int flags, 
                                 OsMsgQ *pMsgQ = NULL, 
                                 const char* szTarget = NULL) = 0;

   //! Destroy a simple player in this call.
   virtual OsStatus destroyPlayer(MpStreamPlayer* pPlayer) = 0;

   //! Create a single-buffered play list player for this call 
   /*! (see the CpCallManager createPlayer() method).
    */
   virtual OsStatus createPlaylistPlayer(MpStreamPlaylistPlayer** ppPlayer, 
                                         OsMsgQ *pMsgQ = NULL, 
                                         const char* szTarget = NULL) = 0;

   //! Destroy a single-buffered play list player in this call.
   virtual OsStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer) = 0;

   //! Create a double-buffered list player for this call 
   /*! (see the Call Manager's createPlayer() method).
    */
   virtual OsStatus createQueuePlayer(MpStreamQueuePlayer** ppPlayer, 
                                      OsMsgQ *pMsgQ = NULL, 
                                      const char* szTarget = NULL) = 0;

   //! Destroy a double-buffered list player in this call.
   virtual OsStatus destroyQueuePlayer(MpStreamQueuePlayer* pPlayer) = 0;

   //! Give the focus of the local audio device to the associated call 
   /*! (for example, take this call off hold).
    */
   virtual void giveFocus();

   //! Take this call out of focus for the local audio device 
   /*! (for example, put this call on hold).
    */
   virtual void defocus();

   //! Set the CPU resource limit for the media connections in this call. 
   /*! This is used to limit the available codecs to only those within 
    * the designated CPU cost limit. 
    */
   virtual void setCodecCPULimit(int iLimit) = 0 ;

   //! Add a listener event to this call that will receive callback 
   //! or queued event notifications upon receipt of DTMF tone events 
   //! (RFC 2833).
   virtual void addToneListener(OsNotification *pListener, int connectionId) = 0;

   //! Remove the specified DTMF listener from this call.
   virtual void removeToneListener(int connectionId) = 0;

   //! Start recording audio for this call.
   virtual OsStatus ezRecord(int ms, 
                             int silenceLength, 
                             const char* fileName, 
                             double& duration, 
                             int& dtmfterm,
                             OsProtectedEvent* ev = NULL) = 0;

   //! Stop recording for this call.
   virtual OsStatus stopRecording() = 0;

   virtual void setContactType(int connectionId, CONTACT_TYPE eType) = 0 ;

/* ============================ ACCESSORS ================================= */

   //! Query whether the specified media connection is enabled for 
   //! sending RTP.
   virtual UtlBoolean isSendingRtp(int connectionId);

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isReceivingRtp(int connectionId);

   //! Query whether the specified media connection has a destination 
   //! specified for sending RTP.
   virtual UtlBoolean isDestinationSet(int connectionId);

   //! For internal use only
   virtual void setPremiumSound(UtlBoolean enabled) = 0;

   //! Calculate the current cost for the current set of 
   //! sending/receiving codecs.
   virtual int getCodecCPUCost() = 0 ;

   //! Calculate the worst case cost for the current set of 
   //! sending/receiving codecs.
   virtual int getCodecCPULimit() = 0 ;

   //!Returns the flowgraph's message queue
   virtual OsMsgQ* getMsgQ() = 0 ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   //! Assignment operator disabled
   CpMediaInterface& operator=(const CpMediaInterface& rhs);

   //! Copy constructor disabled
   CpMediaInterface(const CpMediaInterface& rCpMediaInterface);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterface_h_
