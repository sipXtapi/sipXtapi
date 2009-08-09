// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _VoiceEngineFactoryImpl_h_
#define _VoiceEngineFactoryImpl_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mediaBaseImpl/CpMediaDeviceMgr.h"
#include "include/VoiceEngineDefs.h"
#include "include/VoiceEngine.h"
#include "include/VoiceEngineMediaInterface.h"
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#ifndef GIPS_STUB
#  define DEFAULT_VIDEOENGINE_CODEC_LIST "VP71-VGA VP71-CIF VP71-QVGA VP71-QCIF VP71-SQCIF H263-CIF H263-QVGA H263-QCIF H263-SQCIF"
#  define DEFAULT_VOICEENGINE_CODEC_LIST "IPCMWB ISAC ISACLC EG711U EG711A PCMU PCMA iLBC GSM telephone-event G723"
#else
#  define DEFAULT_VOICEENGINE_CODEC_LIST "PCMU telephone-event"
#  define DEFAULT_VIDEOENGINE_CODEC_LIST "H263-CIF H263-QCIF H263-SQCIF"
#endif

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpMediaTask ;
class OsConfigDb ; 
class VoiceEngineMediaInterface;
class VoiceEngineBufferInStream;
class VoiceEngineSocketFactory;


class VoiceEngineLogger : public GIPSVoiceEngineObserver
{
public:
    VoiceEngineLogger() ;
    virtual ~VoiceEngineLogger() ;
    
    virtual void CallbackOnError(int errCode, int channel) ;
    virtual void CallbackOnTrace(char* message, int length) ;
};

class VideoEngineLogger : public GIPS_TraceCallback
{
public: 
    VideoEngineLogger() ;
    virtual ~VideoEngineLogger() ;

    virtual void Print(char *traceString, int length) ;
} ;


/**
 *
 */
class VoiceEngineFactoryImpl : public CpMediaDeviceMgr, public GIPSVEMediaProcess
{    
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:
      friend class VoiceEngineMediaInterface ;

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   VoiceEngineFactoryImpl();
     

   /**
    * Destructor
    */
   virtual ~VoiceEngineFactoryImpl();

    virtual void initialize(  OsConfigDb* pConfigDb, 
                         uint32_t fameSizeMs = 0, 
                         uint32_t maxSamplesPerSec = 0,
                         uint32_t defaultSamplesPerSec = 0);

    virtual void setSysLogHandler(OsSysLogHandler sysLogHandler) ;

    virtual void destroy();

/* ============================ MANIPULATORS ============================== */
    virtual IMediaInterface* createMediaInterface( const char* publicAddress,
                                                    const char* localAddress,
                                                    int numCodecs,
                                                    SdpCodec* sdpCodecArray[],
                                                    const char* locale,
                                                    int expeditedIpTos,
                                                    const ProxyDescriptor& stunServer,
                                                    const ProxyDescriptor& turnProxy,
                                                    const ProxyDescriptor& arsProxy,
                                                    const ProxyDescriptor& arsHttpProxy,
                                                    SIPX_MEDIA_PACKET_CALLBACK pMediaPacketCallback,
                                                    UtlBoolean enableIce,
                                                    uint32_t samplesPerSec = 0);



    /**
     * Removes a media interface from our stored list of media interfaces
     */
    void releaseInterface(VoiceEngineMediaInterface* pMediaInterface);

    VoiceEngine* getNewVoiceEngineInstance() const ;
    void releaseVoiceEngineInstance(VoiceEngine* pInstance) ;

    GipsVideoEnginePlatform* getNewVideoEngineInstance(VoiceEngine* pVoiceEngine) const ;
    void releaseVideoEngineInstance(GipsVideoEnginePlatform* pInstance) ;

    // Get either the global or one of the instance-versions of VoiceEngine
    VoiceEngine* getAnyVoiceEngine() const ;
    GipsVideoEnginePlatform* getAnyVideoEngine() const ;    

    virtual OsStatus enableSpeakerVolumeAdjustment(bool bEnable) ;

    virtual OsStatus setSpeakerVolume(int iVolume) ;
    virtual OsStatus setSpeakerDevice(const UtlString& device) ;

    virtual OsStatus setMicrophoneGain(int iGain) ;
    virtual OsStatus setMicrophoneDevice(const UtlString& device) ;
    virtual OsStatus muteMicrophone(UtlBoolean bMute) ;

    virtual OsStatus setAudioAECMode(const MEDIA_AEC_MODE mode);    
    virtual OsStatus setAudioNoiseReductionMode(const MEDIA_NOISE_REDUCTION_MODE mode);
    virtual OsStatus enableAGC(UtlBoolean bEnable);
    virtual OsStatus enableOutOfBandDTMF(UtlBoolean bEnable);
    virtual OsStatus enableInBandDTMF(UtlBoolean bEnable);
    virtual OsStatus enableRTCP(UtlBoolean bEnable) ;
    virtual OsStatus setRTCPName(const char* szName) ;

//    virtual UtlBoolean checkCamera(const char* szCamera) ;

    virtual OsStatus buildCodecFactory(SdpCodecList *pFactory, 
                                       const UtlString& sAudioPreferences, 
                                       const UtlString& sVideoPreferences,
                                       int videoFormat,
                                       int* iRejected);

    virtual OsStatus setVideoPreviewDisplay(void* pDisplay) ;     
    virtual void* getVideoPreviewDisplay() ;     
    virtual OsStatus updateVideoPreviewWindow(void* displayContext) ;
    virtual OsStatus setVideoQuality(int quality);
    virtual OsStatus setVideoParameters(int bitRate, int frameRate);
    virtual OsStatus setVideoBitrate(int bitrate);
    virtual OsStatus setVideoFramerate(int framerate);
    virtual OsStatus setVideoCpuValue(int cpuValue);
    virtual OsStatus setConnectionIdleTimeout(const int idleTimeout);

    virtual OsStatus startPlayAudio(VoiceEngineBufferInStream* pStream, GIPS_FileFormats formatType, int downScaling) ;
    virtual OsStatus stopPlay(const void* pSource = NULL) ;   

    virtual OsStatus playTone(int toneId) ;  

    OsStatus createLocalAudioConnection();
    OsStatus destroyLocalAudioConnection();

    virtual void Process(int channel_no, short* audio_10ms_16kHz, int len, int sampfreq) ;    

    virtual OsStatus translateToneId(const SIPX_TONE_ID toneId,
                                 SIPX_TONE_ID&      xlateId ) const;

/* ============================ ACCESSORS ================================= */

    virtual OsStatus getSpeakerVolume(int& iVolume) const  ;
    virtual OsStatus getSpeakerDevice(UtlString& device) const ;
    virtual OsStatus getMicrophoneGain(int& iVolume) const ;
    virtual OsStatus getMicrophoneDevice(UtlString& device) const ;

    virtual OsStatus setMicDataHook(MICDATAHOOK pFunc)
    {
        return OS_NOT_SUPPORTED;
    }

    virtual OsStatus setSpeakerDataHook(SPEAKERDATAHOOK pFunc)
    {
        return OS_NOT_SUPPORTED;
    }

    virtual OsStatus getVideoCaptureDevice(UtlString& videoDevice);
    virtual OsStatus getVideoCaptureDevices(UtlSList& videoDevices) const;
    virtual OsStatus setVideoCaptureDevice(const UtlString& videoDevice);
    virtual OsStatus getVideoQuality(int& quality) const;
    virtual OsStatus getVideoBitRate(int& bitRate) const;
    virtual OsStatus getVideoFrameRate(int& frameRate) const;
    virtual OsStatus getVideoCpuValue(int& cpuValue) const;
    virtual bool getGipsTracing() const;
    virtual void setGipsTracing(const bool bEnable) ;

    virtual OsStatus getConnectionIdleTimeout(int& idleTimeout) const;

    virtual OsStatus getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const;

    virtual OsStatus getLocalAudioConnectionId(int& connectionId) const;

    virtual void* const getAudioEnginePtr() const;

    virtual void* const getVideoEnginePtr() const ;

    virtual OsMutex* getLock() ;

    virtual VoiceEngineSocketFactory* getSocketFactory() { return mpFactory; } ;

    MediaDeviceInfo& getAudioInputDeviceInfo() ;

    MediaDeviceInfo& getAudioOutputDeviceInfo() ;

/* ============================ INQUIRY =================================== */

    virtual OsStatus getAudioAECMode(MEDIA_AEC_MODE& mode) const ;
    virtual OsStatus getAudioNoiseReductionMode(MEDIA_NOISE_REDUCTION_MODE& mode) const ;
    virtual OsStatus isAGCEnabled(UtlBoolean& bEnable) const ;
    virtual OsStatus isOutOfBandDTMFEnabled(UtlBoolean& bEnabled) const;
    virtual OsStatus isInBandDTMFEnabled(UtlBoolean& bEnabled) const;
    virtual bool     isSpeakerAdjustSet() const { return mbSpeakerAdjust;} ;
        
    const bool isMuted() const;

    /**
     * Do we have any active video sessions?
     */ 
    bool isVideoSessionActive() const;  

#ifdef _WIN32
    HBITMAP getNoCameraBitmap() const { return mhNoCameraBitmap; } ;
    HBITMAP getConnectingBitmap() const { return mhConnectingBitmap; } ;
#endif

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
    mutable OsMutex mRtpPortLock;

    bool doSetAudioAECMode(VoiceEngine* pVoiceEngine, 
                           const MEDIA_AEC_MODE mode) const ;
    bool doSetAudioNoiseReductionMode(VoiceEngine*  pVoiceEngine, 
                                      const MEDIA_NOISE_REDUCTION_MODE mode) const ;
    bool doEnableAGC(VoiceEngine* pVoiceEngine, bool bEnable) const ;

    void constructGlobalInstance(bool bNoLocalChannel = false, bool bStartVideo = false) const ;

    void doEnableLocalChannel(bool bEnable) const ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
    /**
     * Converts a system device string to a device id.
     *
     * @param deviceIndex Integer reference - to be filled in with the system
     *                    device id.
     * @param device The name of the system audio device.
     */
    OsStatus outputDeviceStringToIndex(int& deviceIndex, const UtlString& device) const;

    /**
     * Converts a system device id to a device string.
     *
     * @param device UtlString reference - to be filled in with the system device string.
     * @param deviceIndex The index of the system audio device.
     */
    OsStatus outputDeviceIndexToString(UtlString& device, const int deviceIndex) const;
    
    /**
     * Converts a system device string to a device id.
     *
     * @param deviceIndex Integer reference - to be filled in with the system
     *                    device id.
     * @param device The name of the system audio device.
     */
    OsStatus inputDeviceStringToIndex(int& deviceIndex, const UtlString& device) const;

    /**
     * Converts a system device id to a device string.
     *
     * @param device UtlString reference - to be filled in with the system device string.
     * @param deviceIndex The index of the system audio device.
     */
    OsStatus inputDeviceIndexToString(UtlString& device, const int deviceIndex) const;    

    mutable VoiceEngine* mpVoiceEngine;
    mutable GipsVideoEnginePlatform* mpVideoEngine;
    mutable GipsVideoEnginePlatform* mpStaticVideoEngine;
    mutable int  mLocalConnectionId ;
    mutable bool mbLocalConnectionInUse ;

    int mCurrentWaveInDevice;
    int mCurrentWaveOutDevice;
    int mVideoQuality;
    int mVideoBitRate;
    int mVideoFrameRate;
    int mCpu;
    int mIdleTimeout;
    bool mTrace;
    UtlBoolean mbDTMFOutOfBand;
    UtlBoolean mbDTMFInBand;
    UtlBoolean mbEnableRTCP;
    UtlString  mRtcpName ;
    UtlSList mInterfaceList;
    bool mbMute;
    bool mbEnableAGC ;
    void* mpPreviewWindowDisplay;
    mutable UtlString mVideoCaptureDevice;
    MEDIA_AEC_MODE mAECMode ;
    MEDIA_NOISE_REDUCTION_MODE mNRMode ;
    VoiceEngineBufferInStream* mpInStream ;
    bool mbSpeakerAdjust ;
    int  miVolume ;
    bool mbCreateLocalConnection ;
    int  mVideoFormat ;
    CpMediaNetTask* mpMediaNetTask;

    VoiceEngineSocketFactory* mpFactory ;
	VoiceEngineLogger* mpLogger;
    VideoEngineLogger* mpVideoLogger;
    mutable MediaDeviceInfo mAudioDeviceInput ;
    mutable MediaDeviceInfo mAudioDeviceOutput ;

#ifdef _WIN32
    HBITMAP mhNoCameraBitmap ;
    HBITMAP mhConnectingBitmap ;
#endif
};

/* ============================ INLINE METHODS ============================ */

#endif  // _VoiceEngineFactoryImpl_h_
