// 
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

#ifndef _CpMediaInterface_h_
#define _CpMediaInterface_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsDefs.h>
#include <os/OsProtectEvent.h>
#include <os/OsMsgQ.h>
#include <os/OsDatagramSocket.h>
#include <net/SipContactDb.h>
#include <net/SdpBody.h>
#include <os/IStunSocket.h>
#include <os/OsMsgDispatcher.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
  /// The intended data that will be flowing through a socket.
typedef enum SocketPurpose
{
   UNKNOWN,
     /// Socket is intended to transport RTP Audio data
   RTP_AUDIO,
     /// Socket is intended to transport RTCP Audio control data
   RTCP_AUDIO,
     /// Socket is intended to transport RTP Video data
   RTP_VIDEO,
     /// Socket is intended to transport RTCP Video control data
   RTCP_VIDEO
} SocketPurpose;

  /// SipX Media Interface Audio Bandwidth IDs
typedef enum SIPXMI_AUDIO_BANDWIDTH_ID
{
     /// ID for codecs with variable bandwidth requirements
   AUDIO_MICODEC_BW_VARIABLE=0,

     /// ID for codecs with low bandwidth requirements
   AUDIO_MICODEC_BW_LOW,
     /// ID for codecs with normal bandwidth requirements
   AUDIO_MICODEC_BW_NORMAL,
     /// ID for codecs with high bandwidth requirements
   AUDIO_MICODEC_BW_HIGH,

     /// Possible return value for sipxConfigGetAudioCodecPreferences.
     /// This ID indicates the available list of codecs was overridden by a 
     /// sipxConfigSetAudioCodecByName call.
   AUDIO_MICODEC_BW_CUSTOM,       

     /// Value used to signify the default bandwidth level when calling 
     /// sipxCallConnect, sipxCallAccept, or sipxConferenceAdd 
   AUDIO_MICODEC_BW_DEFAULT       

} SIPXMI_AUDIO_BANDWIDTH_ID;


/**
*  @brief Interface declaration for receiving socket idle notifications.
*/
class ISocketEvent
{
public:
   virtual void onIdleNotify(IStunSocket* const pSocket,
                             SocketPurpose purpose,
                             const int millisecondsIdle) = 0;

   virtual void onReadData(IStunSocket* const pSocket,
                           SocketPurpose purpose) = 0;

   virtual ~ISocketEvent() { } ;
};

class IMediaEventEmitter
{
public:
   virtual void onListenerRemoved() = 0;
};

typedef enum IMediaEvent_DeviceErrors
{
   IError_DeviceUnplugged
} IMediaEvent_DeviceErrors;

typedef enum IMediaEvent_DeviceTypes
{
   IDevice_Audio,
   IDevice_Video
} IMediaEvent_DeviceTypes;

/**
*  @brief Interface declaration for receiving device error 
*  notifications/audio events
*/
class IMediaEventListener
{
public:    
   virtual void onFileStart(IMediaEvent_DeviceTypes type) = 0 ;
   virtual void onFileStop(IMediaEvent_DeviceTypes type) = 0 ;
   virtual void onBufferStart(IMediaEvent_DeviceTypes type) = 0 ;
   virtual void onBufferStop(IMediaEvent_DeviceTypes type) = 0 ;
   virtual void onDeviceError(IMediaEvent_DeviceTypes type, IMediaEvent_DeviceErrors errCode) = 0;
   virtual void onListenerAddedToEmitter(IMediaEventEmitter* pEmitter) = 0;

   virtual ~IMediaEventListener() { } ;
};

// FORWARD DECLARATIONS
class SdpCodec;
class SdpCodecFactory;
class MpStreamPlaylistPlayer;
class MpStreamPlayer;
class MpStreamQueuePlayer;
class CpMediaInterfaceFactoryImpl ;

/** 
 * @brief Abstract media control interface.
 * 
 * The CpCallManager creates a CpMediaInterface for each call created.
 * The media inteface is then used to control and query the media sub-system 
 * used for that call.  As connections are added to the call, the 
 * media inteface is used to add those connections to the media control system 
 * such that all connections in that call are bridged together.
 * 
 * @note This abstract class must be sub-classed and implemented to replace 
 *       the default media sub-system.
 */
class CpMediaInterface : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////// */
public:

/* =========================== CREATORS =========================== */

     /// @brief Default constructor
   CpMediaInterface(CpMediaInterfaceFactoryImpl *pFactoryImpl);

/* ========================= DESTRUCTORS ========================== */

protected:
     /// @brief Protected Destructor so that we can manage media 
     ///        interface pointers.
   virtual ~CpMediaInterface();
public:

     /// @brief public interface for destroying this media interface
   virtual void release() = 0; 

/* ========================= MANIPULATORS ========================= */
     /// @brief Create a media connection in the media processing subsystem.
   virtual 
   OsStatus createConnection(
               int& connectionId,
               const char* szLocalAddress,
               int localPort = 0,
               void* videoWindowHandle = NULL,
               void* const pSecurityAttributes = NULL,
               ISocketEvent* pSocketIdleSink = NULL,
               IMediaEventListener* pMediaEventListener = NULL,
               const RtpTransportOptions rtpTransportOptions=RTP_TRANSPORT_UDP) = 0 ;
     /**<
     *  One instance of the CpMediaInterface exists for each call, however, 
     *  each leg of the call requires in individual connection.
     *
     *  @param[out] connectionId - A newly allocated connection id returned via 
     *              this call.  The connection passed to many other media 
     *              processing methods in this interface.
     *  @param[in]  szLocalAddress - Local address (interface) that should 
     *              be used for this connection.
     *  @param[in]  localPort - Local port that should be used for this
     *              connection.
     *              Note, that in fact two ports will be allocated - 
     *              (localPort) for RTP and (localPort+1) for RTCP.
     *              If 0 is passed, port number will be selected automatically.
     *  @param[in]  videoWindowHandle - Video Window handle if using video.  
     *              Supply a window handle of \c NULL to disable video for this 
     *              call/connection.
     *  @param      pSecurityAttributes - Pointer to a 
     *              SIPXVE_SECURITY_ATTRIBUTES object.  
     *  @param      pSocketIdleSink - <<UNKNOWN -- What does this do? 
     *              -- kkyzivat 20070801 >>
     *  @param      pMediaEventListener - <<UNKNOWN -- What does this do?
     *              -- kkyzivat 20070801 >>
     *  @param[in]  rtpTransportOptions RTP_TRANSPORT_UDP, RTP_TRANSPORT_TCP, 
     *              or BOTH
     *  @retval     UNKNOWN - << TODO: Add useful return values here - i.e.
     *              failure codes to expect, etc. -- kkyzivat 20070801 >>
     */ 


     /// @brief Add or replace the media notification dispatcher held by the MI.
   virtual OsMsgDispatcher* 
   setMediaNotificationDispatcher(OsMsgDispatcher* pNoteDisper);
     /**<
     *  Gives the Media Interface an object to help in the dispatching of 
     *  notification messages to users of the media interface.  Users
     *  are free to subclass OsMsgDispatcher to filter messages as
     *  they see fit, and should hold on to it to receive their messages.
     *  
     *  @param[in] pNoteDisper - A notification dispatcher to give to the MI.
     *  @retval Pointer to the previous media notification dispatcher set in this MI -
     *          If there was no previous media notification dispatcher, NULL is returned.
     */

     /// @brief Enable or disable media notifications for one/all resource(s).
   virtual OsStatus
   setMediaNotificationsEnabled(bool enabled, 
                                const UtlString& resourceName = NULL
                                ) { return OS_NOT_YET_IMPLEMENTED; } //= 0;
     /**<
     *  Enable or disable media notifications for a given resource or all resources.
     *
     *  @NOTE If /p NULL is passed for resource name, then all resources 
     *        will have all notifications enabled/disabled
     *  
     *  @param[in] enabled - Whether notification type is to be enabled or disabled.
     *  @param[in] resourceName - the name of the resource to have notifications 
     *             enabled/disabled on.
     *  @retval OS_SUCCESS if the initial sending of a message to enable/disable
     *          notifications succeeded.
     *  @retval OS_NOT_FOUND if there is no resource named /p resourceName.
     *  @retval OS_FAILED if some other failure in queueing the message occured.
     */

     /// @brief Set the secure RTP parameters.
   virtual OsStatus setSrtpParams(SdpSrtpParameters& srtpParameters);
     /**<
     *  @param[in] srtpParameters - the parameter block to pull requested
     *             srtp settings from.
     *  @retval     UNKNOWN - << TODO: Add useful return values here - i.e.
     *              failure codes to expect, etc. -- kkyzivat 20070801 >>
     */


     /// @brief Set the connection destination (target) for the designated
     ///        media connection.
   virtual OsStatus setConnectionDestination(int connectionId,
                                             const char* rtpHostAddress, 
                                             int rtpAudioPort,
                                             int rtcpAudioPort,
                                             int rtpVideoPort,
                                             int rtcpVideoPort) = 0 ;
     /**<
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @param[in] rtpHostAddress - IP (or host) address of remote party.
     *  @param[in] rtpAudioPort - RTP audio port of remote party
     *  @param[in] rtcpAudioPort - RTCP audio port of remote party
     *  @param[in] rtpVideoPort - RTP video port of remote party
     *  @param[in] rtcpVideoPort - RTCP video port of remote party
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Add an alternate Audio RTP connection destination for 
     ///        this media interface.
   virtual OsStatus addAudioRtpConnectionDestination(int connectionId,
                                                     int iPriority,
                                                     const char* candidateIp, 
                                                     int candidatePort) = 0 ;
     /**<
     * Alternerates are generally obtained from the SdpBody in the form
     * of candidate addresses.  When adding an alternate connection, the
     * implementation should use an ICE-like method to determine the 
     * best destination address.
     *
     * @param[in] connectionId - Connection Id for the call leg obtained from 
     *            createConnection
     * @param[in] iPriority - Relatively priority of the destination. 
     *            Higher numbers have greater priority.
     * @param[in] candidateIp - Target/Candidate IP Address
     * @param[in] candidatePort - Target/Candidate Port
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Add an alternate Audio RTCP connection destination for 
     ///        this media interface.
   virtual OsStatus addAudioRtcpConnectionDestination(int connectionId,
                                                       int iPriority,
                                                       const char* candidateIp,
                                                       int candidatePort) = 0 ;
     /**<
     *  @see CpMediaInterface::addAudioRtpConnectionDestination
     */

     /// @brief Add an alternate Video RTP connection destination for 
     ///        this media interface.
   virtual OsStatus addVideoRtpConnectionDestination(int connectionId,
                                                     int iPriority,
                                                     const char* candidateIp,
                                                     int candidatePort) = 0 ;
     /**<
     *  @see CpMediaInterface::addAudioRtpConnectionDestination
     */

     /// @brief Add an alternate Video RTCP connection destination for 
     ///        this media interface.
   virtual OsStatus addVideoRtcpConnectionDestination(int connectionId,
                                                      int iPriority,
                                                      const char* candidateIp,
                                                      int candidatePort) = 0 ;
     /**<
     *  @see CpMediaInterface::addAudioRtpConnectionDestination
     */



     /// @brief Start sending RTP using the specified codec list.
   virtual OsStatus startRtpSend(int connectionId, 
                                 int numCodecs,
                                 SdpCodec* sendCodec[]) = 0 ;
     /**<
     *  Generally, this codec list is the intersection between both parties.
     *
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @param[in] numCodec Number of codecs supplied in the sendCodec array
     *  @param[in] sendCodec Array of codecs ordered in sending preference.
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */ 

     /// @brief Start receiving RTP using the specified codec list.
   virtual OsStatus startRtpReceive(int connectionId,
                                    int numCodecs,
                                    SdpCodec* sendCodec[]) = 0;
     /**<
     *  Generally, this codec list is the intersection between both parties.
     *  The media processing subsystem should be prepared to receive any of 
     *  the specified payload type without additional signaling.
     *  For example, it is perfectly legal to switch between codecs on a whim 
     *  if multiple codecs are agreed upon.
     *
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @param[in] numCodec - Number of codecs supplied in the sendCodec array
     *  @param[in] sendCodec - Array of receive codecs 
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Enables RTP read notification.
   virtual OsStatus enableRtpReadNotification(int connectionId,
                                              UtlBoolean bEnable = TRUE);
     /**<
     *  Enables read notification through the ISocketEvent listener passed 
     *  in via createConnection.  This should be enabled immediately after 
     *  calling startRtpReceive.  It is automatically disabled as part of 
     *  stopRtpReceive.
     *
     *  @param[in] connectionId - the ID of the connection to enable RTP read
     *             notification from.
     *  @param[in] bEnable - Whether or not you want to enable or disable 
     *             read notification.
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */ 

     /// @brief Stop sending RTP and RTCP data for the specified connection
   virtual OsStatus stopRtpSend(int connectionId) = 0 ;
     /**<
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */ 

     /// @brief Stop receiving RTP and RTCP data for the specified connection
   virtual OsStatus stopRtpReceive(int connectionId) = 0 ;
     /**<
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Delete the specified RTP or RTCP connetion.
   virtual OsStatus deleteConnection(int connectionId) = 0 ;
     /**<
     *  Delete the specified connection and free up any resources associated 
     *  with that connection.
     *
     *  @param[in] connectionId - Connection Id for the call leg obtained 
     *             from createConnection.
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Start playing the specified tone for this call.
   virtual OsStatus startTone(int toneId, 
                              UtlBoolean local, 
                              UtlBoolean remote) = 0 ;
     /**<
     *  If the tone is a DTMF tone and the remote flag is set, the interface 
     *  should send out of band DTMF using RFC 2833.  Inband audio should be 
     *  sent to all connections.  If a previous tone was playing, calling 
     *  startTone should automatically stop existing tone.
     * 
     *  @param[in] toneId - The designated tone to play (TODO: make enum)
     *  @param[in] local - True indicates that sound should be played to 
     *             the local speaker (assuming call is in focus).
     *  @param[in] remote - True indicates that the sound should be played to 
     *             all remote parties.  
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Stop playing all tones.
   virtual OsStatus stopTone() = 0 ;
     /**
     * Some tones/implementations may not support this.
     * For example, some DTMF playing implementations will only play DTMF 
     * for a fixed interval.
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

   virtual OsStatus startChannelTone(int connectiondId,
                                     int toneId, 
                                     UtlBoolean local, 
                                     UtlBoolean remote) = 0 ;

   virtual OsStatus stopChannelTone(int connectiondId) = 0 ;


   virtual OsStatus recordChannelAudio(int connectionId,
                                       const char* szFile) = 0 ;

   virtual OsStatus stopRecordChannelAudio(int connectionId) = 0 ;

     /// @brief Play the specified audio URL to the call.
   virtual OsStatus playAudio(const char* url, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote,
                              UtlBoolean mixWithMic = false,
                              int downScaling = 100,
                              OsNotification *event = NULL) = 0 ;
     /**<
     *
     *  @param[in] url - Audio url to be played -- The sipX implementation is limited 
     *             to file paths (not file Urls).
     *  @param[in] repeat - If set, loop the audio file until stopAudio is called.
     *  @param[in] local - True indicates that sound should be played to the local 
     *             speaker (assuming call is in focus).
     *  @param[in] remote - True indicates that the sound should be played to all 
     *             remote parties.  
     *  @param[in] mixWithMic - True to mix with microphone or False to replace it.
     *  @param[in] downScaling - 100 for no down scaling (range from 0 to 100) 
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

   virtual OsStatus playChannelAudio(int connectionId,
                                     const char* url, 
                                     UtlBoolean repeat,
                                     UtlBoolean local, 
                                     UtlBoolean remote,
                                     UtlBoolean mixWithMic = false,
                                     int downScaling = 100,
                                     OsNotification *event = NULL) = 0 ;

     /// @brief Play the specified audio buffer to the call. 
   virtual OsStatus playBuffer(char* buf, 
                               unsigned long bufSize,
                               int type, 
                               UtlBoolean repeat,
                               UtlBoolean local, 
                               UtlBoolean remote,
                               OsProtectedEvent* event = NULL,
                               UtlBoolean mixWithMic = false,
                               int downScaling = 100) = 0 ;
     /**<
     *
     *  @TODO This method should also specify the audio format (e.g. samples/per 
     *        second, etc.).
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     * 
     *  @see CpMediaInterface::playAudio
     */


     /// @brief Stop playing any URLs or buffers
   virtual OsStatus stopAudio()  = 0 ;
     /**<
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     */

   virtual OsStatus stopChannelAudio(int connectionId) = 0 ;


     /// @brief Give the focus of the local audio device to the associated call 
   virtual OsStatus giveFocus() = 0 ;
     /**<
     *  (for example, take this call off hold).
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     */

     /// @brief Take this call out of focus for the local audio device 
   virtual OsStatus defocus() = 0 ;
     /**<
     *  (for example, put this call on hold).
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     */

     /// @brief Create a simple player for this call to play a single stream
   virtual OsStatus createPlayer(MpStreamPlayer** ppPlayer, 
                                 const char* szStream, 
                                 int flags, 
                                 OsMsgQ *pMsgQ = NULL, 
                                 const char* szTarget = NULL) = 0;
     /**<
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     * 
     *  @see CpCallManager::createPlayer
     */

     /// @brief Destroy a simple player in this call.
   virtual OsStatus destroyPlayer(MpStreamPlayer* pPlayer) = 0;
     /**<
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     */

     /// @brief Create a single-buffered play list player for this call 
   virtual OsStatus createPlaylistPlayer(MpStreamPlaylistPlayer** ppPlayer, 
                                         OsMsgQ *pMsgQ = NULL, 
                                         const char* szTarget = NULL) = 0;
     /**<
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     *
     *  @see CpCallManager::createPlayer
     */

     /// @brief Destroy a single-buffered play list player in this call.
   virtual OsStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer) = 0;
     /**
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     */

     /// @brief Create a double-buffered list player for this call 
   virtual OsStatus createQueuePlayer(MpStreamQueuePlayer** ppPlayer, 
                                      OsMsgQ *pMsgQ = NULL, 
                                      const char* szTarget = NULL) = 0;
     /**<
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     *  
     *  @see CpCallManager::createPlayer
     */

   //! Destroy a double-buffered list player in this call.
   virtual OsStatus destroyQueuePlayer(MpStreamQueuePlayer* pPlayer) = 0;

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

     /// @brief Record the microphone data to a file
   virtual OsStatus recordMic(int ms,
                              int silenceLength,
                              const char* fileName) = 0 ;
     /**<
     *  Record a fixed amount of audio from the microphone to a file.
     *  @note The flowgraph must be in focus for this to work properly.
     *
     *  @param[in] ms - The amount of time, in milliseconds, to record.
     *  @param[in] silenceLength - The amount of silence, in SECONDS, before
     *             recording is terminated.
     *  @param[in] fileName - The path and name of a file to record to.
     */

     /// Record the microphone data
   virtual OsStatus recordMic(UtlString* pAudioBuf);
     /**<
     *  Record a fixed amount of audio from the microphone to a buffer 
     *  passed in.
     *  @note The flowgraph must be in focus for this to work properly.
     *
     *  @param pAudioBuf a fixed audio buffer to record to.
     */

   //! Stop recording for this call.
   virtual OsStatus stopRecording() = 0;

   //! Set the preferred contact type for this media connection
   virtual void setContactType(int connectionId, SIPX_CONTACT_TYPE eType, SIPX_CONTACT_ID contactId) = 0 ;

   //! Rebuild the codec factory on the fly
   virtual OsStatus setAudioCodecBandwidth(int connectionId, int bandWidth) = 0;

   //! Rebuild codec factory with one video codec
   virtual OsStatus rebuildCodecFactory(int connectionId, 
                                        int audioBandwidth, 
                                        int videoBandwidth, 
                                        UtlString& videoCodec) = 0;


   //! Set connection bitrate on the fly
   virtual OsStatus setConnectionBitrate(int connectionId, int bitrate) = 0 ;

   //! Set connection framerate on the fly
   virtual OsStatus setConnectionFramerate(int connectionId, int framerate) = 0;

    /// Provide an invalid connectionId
   static int getInvalidConnectionId();

   //! For internal use only
   virtual void setPremiumSound(UtlBoolean enabled) = 0;

   virtual OsStatus setVideoWindowDisplay(const void* hWnd) = 0;

   virtual OsStatus setSecurityAttributes(const void* security) = 0;

   virtual OsStatus generateVoiceQualityReport(int         connectionId,
                                               const char* callId,
                                               UtlString&  report) = 0 ;


   virtual void setConnectionTcpRole(const int connectionId,
                                     const RtpTcpRoles role) = 0;

/* ============================ ACCESSORS ================================= */

   /**
    * Get the port, address, and codec capabilities for the specified media 
    * connection.  The CpMediaInterface implementation is responsible for 
    * managing port allocations.
    *  
    * @param connectionId Connection Id for the call leg obtained from 
    *        createConnection
    * @param rtpHostAddress IP address or hostname that should be advertised
    *        in SDP data.
    * @param rtpPort RTP port number that should be advertised in SDP.
    * @param rtcpPort RTCP port number that should be advertised in SDP.
    * @param supportedCodecs List of supported codecs.
    * @param srtParams supported SRTP parameters
    * @param bandWidth bandwidth limitation id
    */
   virtual OsStatus getCapabilities(int connectionId, 
                                    UtlString& rtpHostAddress, 
                                    int& rtpAudioPort,
                                    int& rtcpAudioPort,
                                    int& rtpVideoPort,
                                    int& rtcpVideoPort, 
                                    SdpCodecFactory& supportedCodecs,
                                    SdpSrtpParameters& srtpParams,
                                    int bandWidth,
                                    int& videoBandwidth,
                                    int& videoFramerate) = 0;
    
   /**
    * DOCME
    */
   virtual OsStatus getCapabilitiesEx(int connectionId, 
                                      int nMaxAddresses,
                                      UtlString rtpHostAddresses[], 
                                      int rtpAudioPorts[],
                                      int rtcpAudioPorts[],
                                      int rtpVideoPorts[],
                                      int rtcpVideoPorts[],
                                      RTP_TRANSPORT transportTypes[],
                                      int& nActualAddresses,
                                      SdpCodecFactory& supportedCodecs,
                                      SdpSrtpParameters& srtpParameters,
                                      int bandWidth,
                                      int& videoBandwidth,
                                      int& videoFramerate) = 0 ;


   //! Calculate the current cost for the current set of 
   //! sending/receiving codecs.
   virtual int getCodecCPUCost() = 0 ;

   //! Calculate the worst case cost for the current set of 
   //! sending/receiving codecs.
   virtual int getCodecCPULimit() = 0 ;

   //!Returns the flowgraph's message queue
   virtual OsMsgQ* getMsgQ() = 0 ;

   //!Returns the flowgraph's Media Notification dispatcher.
   virtual OsMsgDispatcher* getMediaNotificationDispatcher();

   // Returns the primary codec for the connection
   virtual OsStatus getPrimaryCodec(int connectionId, 
                                    UtlString& audioCodec,
                                    UtlString& videoCodec,
                                    int* audiopPayloadType,
                                    int* videoPayloadType,
                                    bool& isEncrypted) = 0;

   virtual const void* getVideoWindowDisplay() = 0;

   virtual OsStatus getAudioEnergyLevels(int& iInputEnergyLevel,
                                         int& iOutputEnergyLevel)
        { return OS_NOT_SUPPORTED ;} ;

   virtual OsStatus getAudioEnergyLevels(int connectionId,
                                         int& iInputEnergyLevel,
                                         int& iOutputEnergyLevel,
                                         int& nContributors,
                                         unsigned int* pContributorSRCIds,
                                         int* pContributorEngeryLevels) 
        { return OS_NOT_SUPPORTED ;} ;

   virtual OsStatus getAudioRtpSourceIDs(int connectionId,
                                         unsigned int& uiSendingSSRC,
                                         unsigned int& uiReceivingSSRC) 
        { return OS_NOT_SUPPORTED ;} ;

   virtual OsStatus enableAudioTransport(int connectionId, UtlBoolean bEnable)
   {
       return OS_NOT_SUPPORTED; 
   };

   virtual OsStatus enableVideoTransport(int connectionId, UtlBoolean bEnable)
   {
       return OS_NOT_SUPPORTED; 
   };


   //! Set a media property on the media interface
    /*
     * Media interfaces that wish to inter-operate should implement the following properties
     * and values:
     *
     * Property Name                  Property Values
     * =======================        ===============
     * "audioInput1.muteState"        "true", "false" for systems that may have a microphone for each conference or 2-way call
     * "audioInput1.device"           same value as szDevice in sipxAudioSetCallInputDevice
     * "audioOutput1.deviceType"      "speaker", "ringer" same as sipxAudioEnableSpeaker, but for specific conference or 2-way call
     * "audioOutput1.ringerDevice"    same value as szDevice in sipxAudioSetRingerOutputDevice 
     * "audioOutput1.speakerDevice"   same values as szDevice in sipxAudioSetCallOutputDevice
     * "audioOutput1.volume"          string value of iLevel in sipxAudioSetVolume
     */
   virtual OsStatus setMediaProperty(const UtlString& propertyName,
                                     const UtlString& propertyValue) = 0;

   //! Get a media property on the media interface
   virtual OsStatus getMediaProperty(const UtlString& propertyName,
                                     UtlString& propertyValue) = 0;

   //! Set a media property associated with a connection
   virtual OsStatus setMediaProperty(int connectionId,
                                     const UtlString& propertyName,
                                     const UtlString& propertyValue) = 0;

   //! Get a media property associated with a connection
   virtual OsStatus getMediaProperty(int connectionId,
                                     const UtlString& propertyName,
                                     UtlString& propertyValue) = 0;

     ///< Get the specific type of this media interface
   virtual UtlString getType() = 0;



/* ============================ INQUIRY =================================== */

    /// Query if connectionId is valid
   virtual UtlBoolean isConnectionIdValid(int connectionId);

   //! Query whether the specified media connection is enabled for 
   //! sending RTP.
   virtual UtlBoolean isSendingRtpAudio(int connectionId) = 0 ;

   //! Query whether the specified media connection is enabled for 
   //! sending RTP.
   virtual UtlBoolean isSendingRtpVideo(int connectionId) = 0 ;

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isReceivingRtpAudio(int connectionId) = 0 ;

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isReceivingRtpVideo(int connectionId) = 0 ;

   //! Query whether the specified media connection has a destination 
   //! specified for sending RTP.
   virtual UtlBoolean isDestinationSet(int connectionId) = 0 ;

   //! Query whether a new party can be added to this media interfaces
   virtual UtlBoolean canAddParty() = 0 ;

   //! Query whether the connection has started sending or receiving video
   virtual UtlBoolean isVideoInitialized(int connectionId) = 0 ;

   //! Query whether the connection has started sending or receiving audio
   virtual UtlBoolean isAudioInitialized(int connectionId) = 0 ;

   //! Query if the audio device is available.
   virtual UtlBoolean isAudioAvailable() = 0;

   //! Query if we are mixing a video conference
   virtual UtlBoolean isVideoConferencing() = 0 ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    CpMediaInterfaceFactoryImpl *mpFactoryImpl ;
    SdpSrtpParameters mSrtpParams;
    OsMsgDispatcher* mpMediaNotfDispatcher;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   //! Assignment operator disabled
   CpMediaInterface& operator=(const CpMediaInterface& rhs);

   //! Copy constructor disabled
   CpMediaInterface(const CpMediaInterface& rCpMediaInterface);

   static int sInvalidConnectionId; ///< Number of connection, assigned to invalid connection.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterface_h_
