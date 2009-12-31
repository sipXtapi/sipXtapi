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
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
//
// $$
/////////////////////////////////////////////////////////////////////////////

#ifndef _IMediaDeviceMgr_h_
#define _IMediaDeviceMgr_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "IMediaInterface.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsConfigDb.h"
#include "sdp/SdpCodec.h"
#include "net/ProxyDescriptor.h"
#include "tapi/sipXtapi.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STRUCTS
// TYPEDEFS
typedef void (*MICDATAHOOK)(const int nLength, short* samples) ;
typedef void (*SPEAKERDATAHOOK)(const int nLength, short* samples) ;

typedef enum MEDIA_AEC_MODE
{
   MEDIA_AEC_DISABLED, 
   MEDIA_AEC_SUPPRESS,
   MEDIA_AEC_CANCEL,
   MEDIA_AEC_CANCEL_AUTO
} MEDIA_AEC_MODE ;

typedef enum MEDIA_NOISE_REDUCTION_MODE
{
   MEDIA_NOISE_REDUCTION_DISABLED,
   MEDIA_NOISE_REDUCTION_LOW,
   MEDIA_NOISE_REDUCTION_MEDIUM,
   MEDIA_NOISE_REDUCTION_HIGH
} MEDIA_NOISE_REDUCTION_MODE ;

// FORWARD DECLARATIONS
class IMediaInterface ;
class SdpCodec ;


class IMediaDeviceMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
    virtual ~IMediaDeviceMgr()
    {
    }
    virtual void initialize(  OsConfigDb* pConfigDb, 
                         uint32_t fameSizeMs = 0, 
                         uint32_t maxSamplesPerSec = 0,
                         uint32_t defaultSamplesPerSec = 0) = 0;

    virtual void setSysLogHandler(OsSysLogHandler sysLogHandler) = 0 ;

    virtual void destroy() = 0;
    virtual OsMutex* getLock() = 0;
    virtual void freeDeviceList(char* deviceList[]) = 0;
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
                                            SIPX_MEDIA_PACKET_CALLBACK pCallback,
                                            UtlBoolean enableIce,
                                            uint32_t samplesPerSec = 0) = 0;

    /**
     * Allow changing the actual speaker volume as opposed to scaling the output signal
     */
    virtual OsStatus enableSpeakerVolumeAdjustment(bool bEnable) = 0;

     /// @brief Set the speaker volume.  Valid range includes 0 to 50.
   virtual OsStatus setSpeakerVolume(int iVolume) = 0 ;

     /// @brief Set the speaker device.
   virtual OsStatus setSpeakerDevice(const UtlString& device) = 0 ;

     /// @brief Set the microphone gain.  Valid range includes 0 to 10.
   virtual OsStatus setMicrophoneGain(int iGain) = 0 ;

     /// @brief Set the Microphone device
   virtual OsStatus setMicrophoneDevice(const UtlString& device) = 0 ;    


     /// @brief Mute the microphone
   virtual OsStatus muteMicrophone(UtlBoolean bMute) = 0 ;

     /// @brief Enable/Disable echo cancellation
   virtual OsStatus setAudioAECMode(const MEDIA_AEC_MODE mode) = 0;


     /// @brief Enable/Disable Noise Reduction
   virtual OsStatus setAudioNoiseReductionMode(const MEDIA_NOISE_REDUCTION_MODE mode) = 0;

     /// @brief Enable AGC Status
   virtual OsStatus enableAGC(UtlBoolean bEnable) = 0;

     /// @brief Enable/Disable sending DTMF tones inband
   virtual OsStatus enableOutOfBandDTMF(UtlBoolean enable) = 0;

  	  /// @brief Enable/Disable sending DTMF tones inband
   virtual OsStatus enableInBandDTMF(UtlBoolean enable) = 0;

     /// @brief Enable/Disable RTCP reports
   virtual OsStatus enableRTCP(UtlBoolean bEnable) = 0;

     /// @brief Set name send as part of RTCP reports.
   virtual OsStatus setRTCPName(const char* szName) = 0;

     /// @brief Populate the codec factory, return number of rejected codecs
   virtual OsStatus buildCodecFactory(SdpCodecList *pFactory, 
                                      const UtlString& sAudioPreferences,
                                      const UtlString& sVideoPreferences,
                                      int videoFormat,
                                      int* iRejected) = 0;

   virtual OsStatus setMicDataHook(MICDATAHOOK pFunc) = 0;
   virtual OsStatus setSpeakerDataHook(SPEAKERDATAHOOK pFunc) = 0;

     /// @brief Set the global video preview window 
   virtual OsStatus setVideoPreviewDisplay(void* pDisplay) = 0;


     /// @brief Set the global video quality 
   virtual OsStatus setVideoQuality(int quality) = 0;


     /// @brief Set the global video parameters 
   virtual OsStatus setVideoParameters(int bitRate, int frameRate) = 0;

   virtual OsStatus setVideoBitrate(int bitrate) = 0;

   virtual OsStatus setVideoFramerate(int framerate) = 0;

     /// @brief Set the global CPU usage
   virtual OsStatus setVideoCpuValue(int cpuValue) = 0;

     /// @brief GIPS-specific trace setting
   virtual void setGipsTracing(const bool bTracing) = 0;

     /// @brief Update the video preview window given the specified display context.
   virtual OsStatus updateVideoPreviewWindow(void* displayContext) = 0;

     /// @brief Sets the RTP port range for this factory
   virtual void setRtpPortRange(int startRtpPort, int lastRtpPort) = 0;

     /// Gets the next available rtp port
   virtual OsStatus getNextRtpPort(int &rtpPort) = 0;

     /// @brief Release the rtp port back to the pool of available RTP ports
   virtual OsStatus releaseRtpPort(const int rtpPort) = 0;

     /// @brief Set the connection idle timeout
   virtual OsStatus setConnectionIdleTimeout(const int idleTimeout) = 0;

   virtual OsStatus translateToneId(const SIPX_TONE_ID toneId,
                                SIPX_TONE_ID&      xlateId ) const = 0;

/* ============================ ACCESSORS ================================= */

     /// @brief Get the speaker volume
   virtual OsStatus getSpeakerVolume(int& iVolume) const = 0 ;

     /// @brief Get the speaker device
   virtual OsStatus getSpeakerDevice(UtlString& device) const = 0 ;

     /// @brief Get the microphone gain
   virtual OsStatus getMicrophoneGain(int& iVolume) const = 0 ;

     /// @brief Get the microphone device
   virtual OsStatus getMicrophoneDevice(UtlString& device) const = 0 ;

     /// @brief Get the connection id for the local audio connection
   virtual OsStatus getLocalAudioConnectionId(int& connectionId) const = 0;

     /// @brief Gets a list of UtlStrings representing available video capture devices.
   virtual OsStatus getVideoCaptureDevices(UtlSList& videoDevices) const = 0;
    
     /// @brief Gets the current video device string.
   virtual OsStatus getVideoCaptureDevice(UtlString& videoDevice) = 0;
    
     /// @brief Sets the video capture device, given its string name.
   virtual OsStatus setVideoCaptureDevice(const UtlString& videoDevice) = 0;

     /// @brief Get video quality
   virtual OsStatus getVideoQuality(int& quality) const = 0; 

     /// @brief Get video bit rate
   virtual OsStatus getVideoBitRate(int& bitRate) const = 0;

     /// @brief Get video frame rate
   virtual OsStatus getVideoFrameRate(int& frameRate) const = 0;

     /// @brief Get cpu usage
   virtual OsStatus getVideoCpuValue(int& cpuValue) const = 0;

     /// @brief Get the connection idle timeout
   virtual OsStatus getConnectionIdleTimeout(int& idleTimeout) const = 0;


/* ============================ INQUIRY =================================== */

     /// @brief Return status of echo cancellation
   virtual OsStatus getAudioAECMode(MEDIA_AEC_MODE& mode) const = 0;
        

     /// @brief Return status of noise reduction
   virtual OsStatus getAudioNoiseReductionMode(MEDIA_NOISE_REDUCTION_MODE& mode) const = 0;


   virtual OsStatus getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const = 0;

     /// @brief Return status of AGC
   virtual OsStatus isAGCEnabled(UtlBoolean& bEnable) const = 0;


     /// @brief Return status of out-of-band DTMF
   virtual OsStatus isOutOfBandDTMFEnabled(UtlBoolean& enabled) const = 0;


     /// @brief Return status of in band DTMF
   virtual OsStatus isInBandDTMFEnabled(UtlBoolean& enabled) const = 0;

    virtual int getNumAudioInputDevices() const = 0;
    virtual void getAudioInputDevices(char* deviceNameArray[], void* deviceHandleArray[], const int arraySize) const = 0;
    virtual int getNumAudioOutputDevices() const = 0;
    virtual void getAudioOutputDevices(char* deviceNameArray[], void* deviceHandleArray[], const int arraySize) const = 0;

    virtual void applyMediaContactTypeOverride(SIPX_CONTACT_TYPE& eType) const = 0;
    virtual void* const getAudioEnginePtr() const = 0;
    virtual void* const getVideoEnginePtr() const = 0;
    virtual OsStatus createLocalAudioConnection() = 0;
    virtual OsStatus destroyLocalAudioConnection() = 0;




/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif 

