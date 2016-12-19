//
// Copyright (C) 2006-2016 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpCallFlowGraph_h_
#define _MpCallFlowGraph_h_

#include <rtcp/RtcpConfig.h>

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpMisc.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/StreamDefs.h>
#include <mp/MpStreamMsg.h>
#include <os/OsProtectEvent.h>
#include <mp/MprRecorder.h>
#include <os/OsSocket.h>

// DEFINES
#define DEBUG_POSTPONE
#undef DEBUG_POSTPONE

/// Undefine this to fully disable AEC
#define DOING_ECHO_CANCELATION
/// Undefine this to enable internal AEC.
#define SPEEX_ECHO_CANCELATION

// Disable Speex AEC if Speex is not available
#ifndef HAVE_SPEEX // [
#  undef SPEEX_ECHO_CANCELATION
#endif // !HAVE_SPEEX ]

// Make sure that at least one canceler enabled if AEC enabled at all.
// And make sure that all cancelers disabled if AEC is disabled.
#ifdef DOING_ECHO_CANCELATION // [
#  ifndef SPEEX_ECHO_CANCELATION // [
//#     define SIPX_ECHO_CANCELATION
// $$$ Ipse: sipX AEC is not working now!
#     undef DOING_ECHO_CANCELATION
#  endif // !SPEEX_ECHO_CANCELATION ]
#else // DOING_ECHO_CANCELATION ][
#  undef SPEEX_ECHO_CANCELATION
#  undef SIPX_ECHO_CANCELATION
#endif // DOING_ECHO_CANCELATION ]

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum FLOWGRAPH_AEC_MODE
{
    FLOWGRAPH_AEC_DISABLED,
    FLOWGRAPH_AEC_SUPPRESS,
    FLOWGRAPH_AEC_CANCEL,
    FLOWGRAPH_AEC_CANCEL_AUTO
} FLOWGRAPH_AEC_MODE ;

// FORWARD DECLARATIONS
class MprBridge;
class MprFromStream;
class MprFromFile;
class MprFromMic;
class MprEchoSuppress;
class MprSpeexEchoCancel;
class MprSpeexPreprocess;
class MprMixer;
class MprSplitter;
class MprToSpkr;
class MprToneGen;
class SdpCodec;
class MpRtpInputConnection;
class MpRtpOutputConnection;
class MprEncode;
class MprDecode;

/// Flow graph used to handle a basic call
class MpCallFlowGraph : public MpFlowGraphBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum
   {
      TONE_TO_SPKR = 0x1,   ///< Play locally
      TONE_TO_NET  = 0x2    ///< Mix the tone to play out the network connection
   } ToneOptions;

   enum RecorderChoice {
#ifndef DISABLE_LOCAL_AUDIO // [
      RECORDER_MIC = 0,
      RECORDER_ECHO_OUT,
      RECORDER_ECHO_IN8,
#ifdef HIGH_SAMPLERATE_AUDIO // [
      RECORDER_MIC32K,
      RECORDER_SPKR32K,
      RECORDER_ECHO_IN32,
#endif // HIGH_SAMPLERATE_AUDIO ]
#endif // DISABLE_LOCAL_AUDIO ]
      RECORDER_SPKR,
      RECORDER_CALL,    ///< full conversation recorder
      MAX_RECORDERS = 10
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MpCallFlowGraph(const char* pLocale = "",
                   int samplesPerFrame=0,
                   int samplesPerSec=0,
                   OsMsgDispatcher *pNotifDispatcher= NULL);

     /// Destructor
   virtual
   ~MpCallFlowGraph();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Starts playing the indicated tone.
   void startTone(int toneId, int toneOptions);

     /// Stops playing the tone (applies to all tone destinations).
   void stopTone(void);

   int closeRecorders(void);

   OsStatus Record(int ms,
            const char* playFilename, ///< if NULL, defaults to previous string
            const char* baseName,     ///< if NULL, defaults to previous string
            const char* endName,      ///< if NULL, defaults to previous string
            int recorderMask);

   OsStatus recordMic(int ms, MpAudioSample* pAudioBuf, int bufferSize);

   OsStatus recordMic(int ms,
                      int silenceLength,
                      const char* fileName) ;

   OsStatus record(int timeMS,
                   int silenceLength,
                   const char* micName = NULL,
                   const char* echoOutName = NULL,
                   const char* spkrName = NULL,
                   const char* mic32Name = NULL,
                   const char* spkr32Name = NULL,
                   const char* echoIn8Name = NULL,
                   const char* echoIn32Name = NULL,
                   const char* playName = NULL,
                   const char* callName = NULL,
                   int toneOptions = 0,
                   int repeat = 0,
                   MprRecorder::RecordFileFormat format = MprRecorder::RAW_PCM_16);

   UtlBoolean setupRecorder(RecorderChoice which, const char* audioFileName,
                  int time, int silenceLength,
                  MprRecorder::RecordFileFormat format = MprRecorder::RAW_PCM_16);

     /// @brief Start playing audio from a buffer passed in.
   OsStatus playBuffer(char* audioBuf,
                  unsigned long bufSize,
                  uint32_t inRate,
                  int type,
                  UtlBoolean repeat,
                  int toneOptions,
                  OsProtectedEvent* event = NULL,
                  UtlBoolean autoStopOnFinish = TRUE);
     /**<
     *  The passed in audio /p audioBuf is converted (if necessary) to the 
     *  flowgraph's sample rate, then played through the flowgraph as it is set up.
     *  
     *  @param[in] audioBuf - the audio buffer to play.
     *  @param[in] bufSize - the size of the audio buffer /p audioBuf
     *  @param[in] inRate - the sample rate of audioBuf
     *  @param[in] type - Currently unused! Please pass '0'.
     *  @param[in] repeat - Whether this audio should be looped until explicitly stopped.
     *  @param[in] toneOptions - TODO: Fill in docs here
     *  @param[in,out] event - an old event object for signaling status changes.
     */

     /// Start playing audio from a file
   OsStatus playFile( const char* audioFileName ///< name of the audio file
                    , UtlBoolean repeat ///< TRUE/FALSE continue playing audio
                                        ///< from the beginning after the end of
                                        ///< file is reached.
                    , int toneOptions   ///< TONE_TO_SPKR/TONE_TO_NET file audio
                                        ///< played locally or both locally and
                                        ///< remotely.
                    , UtlBoolean autoStopOnFinish = TRUE);
     /**<
     *  @returns <b>OS_INVALID_ARGUMENT</b> - if open on the given file name
     *                                        failed.
     */

     /// Tell fromFile resource to pause the currently playing file, if one is being played.
   OsStatus pauseFile();

     /// Tell fromFile resource to resume playing, if previously paused.
   OsStatus resumeFile();

     /// Stop playing audio from a file
   void stopFile(UtlBoolean closeFile);
     /**<
     *  @param closeFile - TRUE/FALSE whether to close the audio file.
     */

     /// Starts sending RTP and RTCP packets.
   void startSendRtp(OsSocket& rRtpSocket, OsSocket& rRtcpSocket,
                     MpConnectionID connID=1, SdpCodec* pPrimaryCodec = NULL,
                     SdpCodec* pDtmfCodec = NULL);

     /// Stops sending RTP and RTCP packets.
   void stopSendRtp(MpConnectionID connID=1);

     /// Starts receiving RTP and RTCP packets.
   void startReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                        OsSocket& rRtpSocket, OsSocket& rRtcpSocket,
                        MpConnectionID connID=1);

     /// Stops receiving RTP and RTCP packets.
   void stopReceiveRtp(MpConnectionID connID=1);

     /// Informs the flow graph that it now has the MpMediaTask focus.
   virtual OsStatus gainFocus(void);
     /**<
     *  Only the flow graph that has the focus is permitted to access
     *  the audio hardware.  This may only be called if this flow graph
     *  is managed and started!
     *
     *  @returns  OS_SUCCESS, always
     */

     /// Informs the flow graph that it has lost the MpMediaTask focus.
   virtual OsStatus loseFocus(void);
     /**<
     *  Only the flow graph that has the focus is permitted to access
     *  the audio hardware.  This should only be called if this flow graph
     *  is managed and started!
     *
     *  @returns  OS_SUCCESS, always
     */

     /// @copydoc MpFlowGraphBase::postNotification()
   OsStatus postNotification(const MpResNotificationMsg& msg);

     /// Creates a new MpAudioConnection; returns -1 if failure.
   MpConnectionID createConnection(int maxRtpStreams = 1,
                                   UtlBoolean isMcast = FALSE);

     /// enables hearing audio data from a source
   UtlBoolean unmuteInput(MpConnectionID connID);

     /// enables sending audio data to a remote party
   UtlBoolean unmuteOutput(MpConnectionID connID);

     /// disables hearing audio data from a source
   UtlBoolean muteInput(MpConnectionID connID);

     /// disables sending audio data to a remote party
   UtlBoolean muteOutput(MpConnectionID connID);

     /// Removes an MpAudioConnection and deletes it and all its resources.
   OsStatus deleteConnection(MpConnectionID connID);

//@}

     /// Enables/Disable the transmission of inband DTMF audio
   static UtlBoolean setInbandDTMF(UtlBoolean bEnable);

     /// Set Acoustic Echo Cancelation mode
   static UtlBoolean setAECMode(FLOWGRAPH_AEC_MODE mode);
     /**<
     *  @warning Only available when Speex or internal AEC module is enabled!
     */

     /// Enable/disable Automatic Gain Control.
   static UtlBoolean setAGC(UtlBoolean bEnable);
     /**<
     *  @warning Only available when Speex is enabled!
     */

     /// Enable/disable speex noise reduction.
   static UtlBoolean setAudioNoiseReduction(UtlBoolean bEnable);
     /**<
     *  @note Enabling this also enables echo residue filtering.
     *
     *  @warning Only available when Speex is enabled!
     */

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Returns the type of this flow graph.
   virtual MpFlowGraphBase::FlowGraphType getType();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// Returns TRUE if the indicated codec is supported.
   UtlBoolean isCodecSupported(SdpCodec& rCodec);

//@}
/* ============================ CALLBACKS ================================= */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   typedef enum {
      START_PLAY_NONE = 0,
      START_PLAY_FILE,
      START_PLAY_SPKR
   } PlayStart;

   static UtlBoolean sbSendInBandDTMF ;
   static UtlBoolean sbEnableAEC ;
   static UtlBoolean sbEnableAGC ;
   static UtlBoolean sbEnableNoiseReduction ;

   enum { MAX_CONNECTIONS = 10 };

   MprBridge*    mpBridge;
   MprFromFile*  mpFromFile;
#ifndef DISABLE_STREAM_PLAYER
   MprFromStream*  mpFromStream;
#endif
#ifndef DISABLE_LOCAL_AUDIO // [
   MprFromMic*   mpFromMic;
   MprSplitter*  mpMicSplitter;
   MprRecorder* mpBufferRecorder;
#  ifdef HAVE_SPEEX // [
      MprSpeexPreprocess* mpSpeexPreProcess;
#  endif // HAVE_SPEEX ]
#  if defined (SPEEX_ECHO_CANCELATION)
      MprSpeexEchoCancel* mpEchoCancel;
#  elif defined (SIPX_ECHO_CANCELATION)
      MprEchoSuppress*    mpEchoCancel;
#  endif
      MprToSpkr*    mpToSpkr;
#endif // DISABLE_LOCAL_AUDIO ]
   MprMixer*     mpTFsMicMixer;
   MprMixer*     mpTFsBridgeMixer;
   MprMixer*     mpCallrecMixer;
   MprSplitter*  mpMicCallrecSplitter;
   MprSplitter*  mpSpeakerCallrecSplitter;
   MprSplitter*  mpToneFileSplitter;
   MprToneGen*   mpToneGen;
   OsBSem        mConnTableLock;
   UtlBoolean    mToneIsGlobal;
   MpRtpInputConnection* mpInputConnections[MAX_CONNECTIONS];
   int           mNumRtpStreams[MAX_CONNECTIONS];
   UtlBoolean    mIsMcastConnection[MAX_CONNECTIONS];
   MprDecode**   mpDecoders[MAX_CONNECTIONS];
   MprMixer*     mpMcastMixer[MAX_CONNECTIONS];
   MpRtpOutputConnection* mpOutputConnections[MAX_CONNECTIONS];
   MprEncode*    mpEncoders[MAX_CONNECTIONS];
   UtlBoolean    mToneGenDefocused; ///< disabled during defocused state flag

   /// these array should really be made into a structure
   /// but for now we'll just use em this way.
   ///
   ///  D.W.
   MprRecorder* mpRecorders[MAX_RECORDERS];

     /// Handles an incoming message for the flow graph.
   virtual UtlBoolean handleMessage(OsMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_REMOVE_CONNECTION message.
   UtlBoolean handleRemoveConnection(MpFlowGraphMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_START_PLAY message for MprFromFile.
   UtlBoolean handleStartPlay(MpFlowGraphMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_START_RECORD message.
   UtlBoolean handleStartRecord(MpFlowGraphMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STOP_RECORD message.
   UtlBoolean handleStopRecord(MpFlowGraphMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_START_TONE message.
   UtlBoolean handleStartTone(MpFlowGraphMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STOP_TONE and FLOWGRAPH_STOP_PLAY messages.
   UtlBoolean handleStopToneOrPlay(void);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

   /** Handle the ON_MPRRECORDER_ENABLED message. It is sent when
   *   a recorder is really enabled.
   */
   UtlBoolean handleOnMprRecorderEnabled(const UtlString &resourceName);
   /**<
   *  @returns <b>TRUE</b> if the message was handled
   *  @returns <b>FALSE</b> otherwise.
   */

   /** Handle the ON_MPRRECORDER_DISABLED message. It is sent when
   *   a recorder is really disabled, so we can send an event to
   *   sipxtapi in the future. Currently we only disable some other
   *   resources if the recorder is call recorder.
   */
   UtlBoolean handleOnMprRecorderDisabled(const UtlString &resourceName);
   /**<
   *  @returns <b>TRUE</b> if the message was handled
   *  @returns <b>FALSE</b> otherwise.
   */

#ifdef DEBUG_POSTPONE /* [ */
     /// sends a message requesting a delay for race condition detection...
   void postPone(int ms);
#endif /* DEBUG_POSTPONE ] */

#ifndef DISABLE_STREAM_PLAYER
     /// Handle the FLOWGRAPH_STREAM_REALIZE_URL message.
   UtlBoolean handleStreamRealizeUrl(MpStreamMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STREAM_REALIZE_BUFFER message.
   UtlBoolean handleStreamRealizeBuffer(MpStreamMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STREAM_PREFETCH message.
   UtlBoolean handleStreamPrefetch(MpStreamMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STREAM_PLAY message.
   UtlBoolean handleStreamPlay(MpStreamMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STREAM_REWIND message.
   UtlBoolean handleStreamRewind(MpStreamMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STREAM_PAUSE message.
   UtlBoolean handleStreamPause(MpStreamMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STREAM_STOP message.
   UtlBoolean handleStreamStop(MpStreamMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STREAM_DESTROY message.
   UtlBoolean handleStreamDestroy(MpStreamMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */
#endif // DISABLE_STREAM_PLAYER

     /// Copy constructor (not implemented for this class)
   MpCallFlowGraph(const MpCallFlowGraph& rMpCallFlowGraph);

     /// Assignment operator (not implemented for this class)
   MpCallFlowGraph& operator=(const MpCallFlowGraph& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCallFlowGraph_h_
