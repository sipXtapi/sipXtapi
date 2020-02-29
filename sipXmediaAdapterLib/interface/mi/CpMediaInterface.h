// 
// Copyright (C) 2006-2020 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2009 SIPfoundry Inc.
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
#include <net/SdpBody.h>
#include <os/IStunSocket.h>

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
   RTP_AUDIO,  ///< Socket is intended to transport RTP Audio data
   RTCP_AUDIO, ///< Socket is intended to transport RTCP Audio control data
   RTP_VIDEO,  ///< Socket is intended to transport RTP Video data
   RTCP_VIDEO  ///< Socket is intended to transport RTCP Video control data
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

     /**
     *  Possible return value for sipxConfigGetAudioCodecPreferences.
     *  This ID indicates the available list of codecs was overridden by a 
     *  sipxConfigSetAudioCodecByName call.
     */
   AUDIO_MICODEC_BW_CUSTOM,       

     /**
     *  Value used to signify the default bandwidth level when calling 
     *  sipxCallConnect, sipxCallAccept, or sipxConferenceAdd 
     */
   AUDIO_MICODEC_BW_DEFAULT       

} SIPXMI_AUDIO_BANDWIDTH_ID;

// FORWARD DECLARATIONS
class SdpCodec;
class SdpCodecList;
class MpStreamPlaylistPlayer;
class MpStreamPlayer;
class MpStreamQueuePlayer;
class CpMediaInterfaceFactoryImpl;
class OsMsgDispatcher;
class CircularBufferPtr;

/** 
 * @brief Abstract media control interface.
 * 
 * The CpCallManager creates a CpMediaInterface for each call created.
 * The media interface is then used to control and query the media sub-system 
 * used for that call.  As connections are added to the call, the 
 * media interface is used to add those connections to the media control system 
 * such that all connections in that call are bridged together.
 * 
 * @note This abstract class must be sub-classed and implemented to replace 
 *       the default media sub-system.
 */
class CpMediaInterface : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////// */
public:

   enum MEDIA_STREAM_TYPE
   {
      MEDIA_TYPE_UNKNOWN = 0,
      AUDIO_STREAM,
      VIDEO_STREAM
   };
   
   enum CpAudioFileFormat
   {
       CP_UNKNOWN_FORMAT = 0,
       CP_WAVE_PCM_16, ///< Wave file format containing PCM 16 bit signed little endian encoded audio
       CP_WAVE_ALAW,
       CP_WAVE_MULAW,
       CP_WAVE_GSM,    ///< Wafe file format contain GSM encoded audio
       CP_OGG_OPUS     ///< OGG file container using OPUS encoder.
   };


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
               const RtpTransportOptions rtpTransportOptions=RTP_TRANSPORT_UDP) = 0 ;
     /**<
     *  One instance of the CpMediaInterface exists for each call, however, 
     *  each leg of the call requires an individual connection.
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

     /// @brief Set PLC method to use for codecs which does not have own PLC.
   virtual OsStatus setPlcMethod(int connectionId,
                                 const UtlString &methodName = "") = 0;
     /**<
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @param[in] methodName - Name of PLC method. If method name is unknown,
     *                          default method will be used. This also means
     *                          you could pass "" to select default method.
     *
     *  @retval OS_SUCCESS - message to connection was sent, PLC method will
     *          be changed on next message processing interval.
     *  @retval OS_NOT_FOUND - connection with given \p connectionId was not found.
     *  @retval OS_NOT_SUPPORTED - this implementation of CpMediaInterface
     *          does not support PLC algorithm changing.
     *  @retval OS_NOT_YET_IMPLEMENTED - this implementation of CpMediaInterface
     *          might support PLC algorithm changing, but this feature is not
     *          yet implemented.
     */

     /// @brief Add, replace, or clear the media notification dispatcher held by the MI.
   virtual OsMsgDispatcher* 
   setNotificationDispatcher(OsMsgDispatcher* pNotificationDispatcher) = 0;
     /**<
     *  Gives the Media Interface an object to help in the dispatching of 
     *  notification messages to users of the media interface.  Users
     *  are free to subclass OsMsgDispatcher to filter messages as
     *  they see fit, and should hold on to it to receive their messages.
     *  
     *  @param[in] pNotificationDispatcher - A notification dispatcher to give 
     *             to the MI.
     *  @return Pointer to the previous media notification dispatcher set in
     *          this MI.  If there was no previous media notification dispatcher, 
     *          \p NULL is returned.
     */

     /// @brief Enable or disable media notifications for one/all resource(s).
   virtual OsStatus
   setNotificationsEnabled(bool enabled, 
                           const UtlString& resourceName = NULL) = 0;
     /**<
     *  Enable or disable media notifications for a given resource or all resources.
     *
     *  @NOTE If /p NULL is passed for resource name, then all resources 
     *        will have all notifications enabled/disabled
     *  @NOTE This is an asynchronous operation.  After calling this, it may
     *        take a bit of time before the new state takes effect.
     *  
     *  @param[in] enabled - Whether notification type is to be enabled or disabled.
     *  @param[in] resourceName - the name of the resource to have notifications 
     *             enabled/disabled on.
     *  @retval OS_SUCCESS if the initial sending of a message to enable/disable
     *          notifications succeeded.
     *  @retval OS_NOT_FOUND if there is no resource named /p resourceName.
     *  @retval OS_FAILED if some other failure in queuing the message occurred.
     */

     /// Return the number of ports on the bridge (by default, not implemented in base class)
   virtual int getNumBridgePorts() {return(0);};

     /// @brief Set the secure RTP parameters.
   virtual OsStatus setSrtpParams(SdpSrtpParameters& srtpParameters);
     /**<
     *  @param[in] srtpParameters - the parameter block to pull requested
     *             srtp settings from.
     *  @retval     UNKNOWN - << TODO: Add useful return values here - i.e.
     *              failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Set pass through port and address to send RTP stream to.
   virtual OsStatus setMediaPassThrough(int connectionId,
                                        MEDIA_STREAM_TYPE mediaType,
                                        int mediaTypeStreamIndex,
                                        UtlString& receiveAddress,
                                        int rtpPort,
                                        int rtcpPort) = 0;
     /**<
      * Set up so that RTP stream does not go through the media subsystem, but get
      * send directly to this port and address.  The idea is that this address and
      * port(s) are provided for use in advertizing in the SDP for the local address
      * and port that the remote side should send the media stream to.  This address
      * and port is provided in the getCapabilitiesEx/getCapabilities methods.
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

     /// @brief Set the connection destination (target) for the designated
     ///        stream on the given media connection.
  virtual OsStatus setConnectionDestination(int connectionId,
                                             CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                             int streamIndex,
                                             const char* rtpHostAddress,
                                             int rtpPort,
                                             int rtcpPort) = 0;
     /**<
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection *  @param[in] rtpHostAddress - IP (or host) address of remote party.
     *  @param[in] mediaType - stream media type (e.g. audio or video)
     *  @param[in] streamIndex - stream index for given type in the given connection
     *  @param[in] rtpPort - RTP port of remote party
     *  @param[in] rtcpPort - RTCP port of remote party
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

   /// @brief copies payload IDs for matching codecs to interfaces codec list
   virtual OsStatus copyPayloadIds(int connectionId, int numCodecs, SdpCodec* remoteCodecs[]) = 0;
   /* Generally this is used when we recieve an SDP offer so that our answer will
    * use the same payload IDs as the remote side for the codecs in common.  This
    * is an inter-op friendly thing to do.
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

     /// @brief Get the list of codecs and payload IDs for the connection
   virtual const SdpCodecList* getConnectionCodecList(int connectionId) = 0;
     /**<
      * @retval pointer to connection's SdpCodecList
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


     /// @brief Set an offset to the NTP time provided in the RTCP SR
   virtual OsStatus setRtcpTimeOffset(int connectionId,
                                      CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                      int streamIndex,
                                      int timeOffset) = 0;
     /**
     *  Allows the RTCP synchronization time to be offset from the local wall clock
     *  time (NTP time).  This does not efect the RTP timestamps in the RTP packets.
     *  It only effects the synchronization time in the RTCP sender report by offsetting
     *  the calculation of the RTP timestamp in the RTCP sender report for the current
     *  wall clock time in the RTCP sender report.  This has the effect of adding a 
     *  syncrhonization offset from other RTP streams (for receiving end points that
     *  use the RTCP sender report for synchronization).
     *
     *  @param[in] connectionId - connection in which the RTP stream to be offset is
     *             contained.
     *  @param[in] mediaType - media time of the stream to be offset (e.g. video or audio)
     *  @param[in] streamIndex - index to specific RTP stream of the give media type in 
     *             the given connection.
     *  @param[in] timeOffset - offset to apply to the stream synchronization time in milliseconds
     *
     *  @retval
     */


   /// @brief Start playing DTMF tone
   virtual OsStatus startChannelTone(int connectiondId,
                                     int toneId, 
                                     UtlBoolean local, 
                                     UtlBoolean remote) = 0 ;

   /// @brief Stop playing DTMF tone
   virtual OsStatus stopChannelTone(int connectiondId) = 0 ;


   /// @brief Start recording to file
   virtual OsStatus recordChannelAudio(int connectionId,
                                       const char* szFile,
                                       CpAudioFileFormat cpFileFormat = CP_WAVE_PCM_16,
                                       UtlBoolean appendToFile = FALSE,
                                       int numChannels = 1) = 0;
   /**
    * @param[in] connectionId - handle for media connection to which recorder is relate
    * @param[in] szFile - name of file to record to
    * @param[in] cpFileFormat - audio encoding format to use in the file
    * @param[in] appendToFile - if file exist TRUE/FALSE should append to existing audio
    *            file.  Note: If file is existing, it must be of the same audio format.
    *            Also be aware that when stopping and starting in rapid succession, you
    *            should wait for the stop notification before starting to record again
    *            with the same file or results are unpredictable.
    * @param[in] numChannels - number of channels of audio to record to file
    *
    * @returns OS_SUCCESS if file was successfully opened (and if append, format is the same)
    *          OS_FAILED if existing file was of a different audio format than requested or
    *                    unable to determine existing file audio format, or error opening
    *                    file for read and write.
    */

   /// @brief Stop buffer or file recording which has been started
   virtual OsStatus stopRecordChannelAudio(int connectionId) = 0 ;

   /// @brief Pause buffer or file recording which has been started
   virtual OsStatus pauseRecordChannelAudio(int connectionId) = 0;

   /// @brief Resume buffer or file recording which has been paused
   virtual OsStatus resumeRecordChannelAudio(int connectionId) = 0;

   /// @brief Start recording to buffer
   virtual OsStatus recordBufferChannelAudio(int connectionId,
                                             char* pBuffer,
                                             int bufferSize,
                                             int maxRecordTime = -1,
                                             int maxSilence = -1) = 0 ;

   /// @brief Stop recording to buffer
   virtual OsStatus stopRecordBufferChannelAudio(int connectionId) = 0 ;

   virtual OsStatus recordCircularBufferChannelAudio(int connectionId,
                                                     CircularBufferPtr & buffer,
                                                     CpMediaInterface::CpAudioFileFormat recordingFormat,
                                                     unsigned long recordingBufferNotificationWatermark) = 0;

   virtual OsStatus stopRecordCircularBufferChannelAudio(int connectionId) = 0;

     /// @brief Play the specified audio URL to the call.
   virtual OsStatus playAudio(const char* url, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote,
                              UtlBoolean mixWithMic = false,
                              int downScaling = 100,
                              UtlBoolean autoStopAfterFinish = TRUE) = 0 ;
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
     *  @param[in] autoStopAfterFinish - if set to TRUE you don't need to call
     *             stopAudio() when playback finishes because of end of the file.
     *             Otherwise you need to call stopAudio() on receiving of FINISHED
     *             notification.
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
                                     UtlBoolean autoStopOnFinish = TRUE) = 0 ;

     /// @brief Play the specified audio buffer to the call. 
   virtual OsStatus playBuffer(char* buf, 
                               unsigned long bufSize,
                               uint32_t bufRate, 
                               int type, 
                               UtlBoolean repeat,
                               UtlBoolean local, 
                               UtlBoolean remote,
                               OsProtectedEvent* event = NULL,
                               UtlBoolean mixWithMic = false,
                               int downScaling = 100,
                               UtlBoolean autoStopOnFinish = TRUE) = 0 ;
     /**<
     *  @todo This method should also specify the audio format (e.g. samples/per 
     *        second, etc.).
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     * 
     *  @see CpMediaInterface::playAudio
     */

     /// @brief Pause all playing URLs or buffers
   virtual OsStatus pauseAudio() = 0;
     /**<
     *  @todo This method should also take an optional uniqueId representing
     *        a particular playing instance to pause, instead of all of them.
     *  @retval OS_SUCCESS if the asynchronous request to pause audio succeeded.
     *  @retval OS_NOT_FOUND if required underlying media resources are not found.
     */

     /// @brief Resume all paused URLs or buffers
   virtual OsStatus resumeAudio() = 0;
     /**<
     *  @todo This method should also take an optional uniqueId representing
     *        a particular paused instance to resume, instead of all of them.
     *  @retval OS_SUCCESS if the asynchronous request to pause audio succeeded.
     *  @retval OS_NOT_FOUND if required underlying media resources are not found.
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

     /// @brief Set microphone gain.
   virtual OsStatus setMicGain(float gain) = 0 ;

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
   virtual OsStatus recordMic(int ms, int16_t* pAudioBuf,
                              int bufferSize) = 0;
     /**<
     *  Record audio from the microphone to a buffer passed in.
     *
     *  @note The flowgraph must be in focus for this to work properly.
     *
     *  @param[in] ms - The amount of time (in milliseconds) to record.
     *  @param[in] pAudioBuf - Audio buffer to record to.
     *  @param[in] bufferSize - Size of the buffer (in samples).
     */

     /// Set the mix weight for all inputs, but its own to the given weight and output on bridge.
   virtual OsStatus setMixWeightForOutput(int bridgeOutputPort, float weight) = 0;
    /**<
     *  @param[in] bridgeOutputPort - output port on the bridge to which input gains are to be set
     *  @param[in] weight - the new weights to set for the inputs.
     *             1.0f is a net gain of zero.  2.0f is a 3Db gain.
     *             The input at port bridgeOutputPort, will get a weight of 0.0f.
     */

    /// Set gain for inputs to given output on bridge
   virtual OsStatus setMixWeightsForOutput(int bridgeOutputPort, int numWeights, float weights[]) = 0;
    /**<
     *  @param[in] bridgeOutputPort - output port on the bridge to which input gains are to be set
     *  @param[in] numWeights - number of input weights provided.
     *             If this number is less than the number of inputs
     *             on the bridge the inputs after numWeights are left
     *             unchanged.
     *  @param[in] weights - the new weights to set for the inputs.
     *             One for each of the numWeights to set.
     *             1.0f is a net gain of zero.  2.0f is a 3Db gain.
     */

   //! Set the preferred contact type for this media connection
   virtual void setContactType(int connectionId, SIPX_CONTACT_TYPE eType, SIPX_CONTACT_ID contactId) = 0 ;

   //! Rebuild the codec factory on the fly
   virtual OsStatus setAudioCodecBandwidth(int connectionId, int bandWidth) = 0;

   //! Further restrict the set of codecs to the list provided.
   virtual OsStatus limitCodecs(int connectionId, const SdpCodecList& includeOnlyCodecList) = 0;
   /**
    * No codecs will be added.  This method only removes currently enabled codecs which
    * are not included in the given list.
    *
    * @returns the number of codecs remaining enabled
    */

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
                                    SdpCodecList& supportedCodecs,
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
                                      SdpCodecList& supportedCodecs,
                                      SdpSrtpParameters& srtpParameters,
                                      int bandWidth,
                                      int& videoBandwidth,
                                      int& videoFramerate) = 0 ;


     /// @brief Calculate the current cost for the current set of sending/receiving codecs.
   virtual int getCodecCPUCost() = 0 ;

     /// @brief Calculate the worst case cost for the current set of sending/receiving codecs.
   virtual int getCodecCPULimit() = 0 ;

     /// @brief Returns the sample rate of the flowgraph.
   virtual uint32_t getSamplesPerSec() = 0;

     /// @brief Returns the samples per frame of the flowgraph.
   virtual uint32_t getSamplesPerFrame() = 0;

     /// @brief Returns the flowgraph's message queue
   virtual OsMsgQ* getMsgQ() = 0 ;

     /// @brief Returns the Media Notification dispatcher this controls.
   virtual OsMsgDispatcher* getNotificationDispatcher() = 0;

     /// @brief Returns the primary codec for the connection
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

   ///< Set IP address to advertise in SDP
   virtual void setConfiguredIpAddress(const UtlString& ipAddress);

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
    UtlString mConfiguredIpAddress;  ///< Address to use instead of local address in SDP

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
