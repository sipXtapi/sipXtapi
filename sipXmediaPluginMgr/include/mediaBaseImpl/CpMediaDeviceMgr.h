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
// Copyright (C) 2005-2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
/////////////////////////////////////////////////////////////////////////////

#ifndef _CpMediaDeviceMgr_h_
#define _CpMediaDeviceMgr_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "mediaInterface/IMediaDeviceMgr.h"
#include "CpMediaInterface.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsConfigDb.h"
#include "sdp/SdpCodec.h"
#include "net/ProxyDescriptor.h"
#include "tapi/sipXtapi.h"


// DEFINES
#define SIPXTAPI_OVERRIDE_KEY             "SOFTWARE\\sipXtapi\\Override"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class SdpCodec ;


class CpMediaDeviceMgr : public IMediaDeviceMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    CpMediaDeviceMgr() :     mlockList(OsMutex::Q_FIFO)
    {
    }
    virtual ~CpMediaDeviceMgr()
    {
    }

/* ============================ CREATORS ================================== */
    virtual void initialize(  OsConfigDb* pConfigDb, 
                         uint32_t fameSizeMs = 0, 
                         uint32_t maxSamplesPerSec = 0,
                         uint32_t defaultSamplesPerSec = 0) = 0;

    void freeDeviceList(char* deviceList[]);

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
    virtual OsStatus enableSpeakerVolumeAdjustment(bool bEnable) 
    {
        return OS_NOT_SUPPORTED ;
    }

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
   virtual OsStatus setAudioAECMode(const MEDIA_AEC_MODE mode)
   {
      return OS_NOT_SUPPORTED ;
   }


     /// @brief Enable/Disable Noise Reduction
   virtual OsStatus setAudioNoiseReductionMode(const MEDIA_NOISE_REDUCTION_MODE mode)
   {
      return OS_NOT_SUPPORTED ;
   }


     /// @brief Enable AGC Status
   virtual OsStatus enableAGC(UtlBoolean bEnable)
   {
       return OS_NOT_SUPPORTED ;
   }


     /// @brief Enable/Disable sending DTMF tones inband
   virtual OsStatus enableOutOfBandDTMF(UtlBoolean enable) 
   {
      return OS_NOT_SUPPORTED ;
   }

  	  /// @brief Enable/Disable sending DTMF tones inband
  	virtual OsStatus enableInBandDTMF(UtlBoolean enable)
   {
      return OS_NOT_SUPPORTED ;
   }

  	 
     /// @brief Enable/Disable RTCP reports
   virtual OsStatus enableRTCP(UtlBoolean bEnable)
   {
      return OS_NOT_SUPPORTED ;
   }

     /// @brief Set name send as part of RTCP reports.
   virtual OsStatus setRTCPName(const char* szName)
   {
      return OS_NOT_SUPPORTED ;
   }

     /// @brief Populate the codec factory, return number of rejected codecs
   virtual OsStatus buildCodecFactory(SdpCodecList *pFactory, 
                                      const UtlString& sAudioPreferences,
                                      const UtlString& sVideoPreferences,
                                      int videoFormat,
                                      int* iRejected) = 0;

     /// @brief Set the global video preview window 
   virtual OsStatus setVideoPreviewDisplay(void* pDisplay) 
   {
      return OS_NOT_SUPPORTED ;
   }


     /// @brief Set the global video quality 
   virtual OsStatus setVideoQuality(int quality) 
   {
      return OS_NOT_SUPPORTED ;
   }


     /// @brief Set the global video parameters 
   virtual OsStatus setVideoParameters(int bitRate, int frameRate) 
   {
      return OS_NOT_SUPPORTED ;
   }

   virtual OsStatus setVideoBitrate(int bitrate)
   {
      return OS_NOT_SUPPORTED ;
   }

   virtual OsStatus setVideoFramerate(int framerate)
   {
      return OS_NOT_SUPPORTED ;
   }

     /// @brief Set the global CPU usage
   virtual OsStatus setVideoCpuValue(int cpuValue) 
   {
      return OS_NOT_SUPPORTED ;
   }

     /// @brief Update the video preview window given the specified display context.
   virtual OsStatus updateVideoPreviewWindow(void* displayContext) 
   {
      return OS_NOT_SUPPORTED ;
   }

     /// @brief Release the rtp port back to the pool of available RTP ports
   virtual OsStatus releaseRtpPort(const int rtpPort) ;

     /// @brief Sets the RTP port range for this factory
   virtual void setRtpPortRange(int startRtpPort, int lastRtpPort) ;

     /// Gets the next available rtp port
   virtual OsStatus getNextRtpPort(int &rtpPort) ;


     /// @brief Set the connection idle timeout
   virtual OsStatus setConnectionIdleTimeout(const int idleTimeout) 
   {
      return OS_NOT_SUPPORTED ;
   }

   virtual OsStatus translateToneId(const SIPX_TONE_ID toneId,
                                SIPX_TONE_ID&      xlateId ) const;
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
   virtual OsStatus getVideoCaptureDevices(UtlSList& videoDevices) const 
   {
      return OS_NOT_SUPPORTED ;
   }

    
     /// @brief Gets the current video device string.
   virtual OsStatus getVideoCaptureDevice(UtlString& videoDevice) 
   {
      return OS_NOT_SUPPORTED ;
   }

    
     /// @brief Sets the video capture device, given its string name.
   virtual OsStatus setVideoCaptureDevice(const UtlString& videoDevice) 
   {
      return OS_NOT_SUPPORTED ;
   }

    
     /// @brief Get video quality
   virtual OsStatus getVideoQuality(int& quality) const 
   {
      return OS_NOT_SUPPORTED ;
   }


     /// @brief Get video bit rate
   virtual OsStatus getVideoBitRate(int& bitRate) const 
   {
      return OS_NOT_SUPPORTED ;
   }


     /// @brief Get video frame rate
   virtual OsStatus getVideoFrameRate(int& frameRate) const 
   {
      return OS_NOT_SUPPORTED ;
   }


     /// @brief Get cpu usage
   virtual OsStatus getVideoCpuValue(int& cpuValue) const = 0;

     /// @brief Get the connection idle timeout
   virtual OsStatus getConnectionIdleTimeout(int& idleTimeout) const 
   {
      return OS_NOT_SUPPORTED ;
   }


/* ============================ INQUIRY =================================== */

     /// @brief Return status of echo cancellation
   virtual OsStatus getAudioAECMode(MEDIA_AEC_MODE& mode) const 
   {
      return OS_NOT_SUPPORTED ;
   }
        

     /// @brief Return status of noise reduction
   virtual OsStatus getAudioNoiseReductionMode(MEDIA_NOISE_REDUCTION_MODE& mode) const 
   {
      return OS_NOT_SUPPORTED ;
   }


     /// @brief Return status of AGC
   virtual OsStatus isAGCEnabled(UtlBoolean& bEnable) const
   {
      return OS_NOT_SUPPORTED ;
   }


     /// @brief Return status of out-of-band DTMF
   virtual OsStatus isOutOfBandDTMFEnabled(UtlBoolean& enabled) const 
   {
      return OS_NOT_SUPPORTED ;
   }


     /// @brief Return status of in band DTMF
   virtual OsStatus isInBandDTMFEnabled(UtlBoolean& enabled) const 
   {
      return OS_NOT_SUPPORTED ;
   }



     /// @brief GIPS-specific trace setting
    virtual void setGipsTracing(const bool bTracing) { return ; }


    virtual void* const getAudioEnginePtr() const { return NULL; } 

    virtual void* const getVideoEnginePtr() const { return NULL; }
    virtual OsStatus createLocalAudioConnection() { return OS_NOT_SUPPORTED; }
    virtual OsStatus destroyLocalAudioConnection() { return OS_NOT_SUPPORTED; }


    virtual int getNumAudioInputDevices();
    virtual void getAudioInputDevices(char* deviceNameArray[], const int arraySize);
    virtual int getNumAudioOutputDevices();
    virtual void getAudioOutputDevices(char* deviceNameArray[], const int arraySize);
    virtual void applyMediaContactTypeOverride(SIPX_CONTACT_TYPE& eType) const ;


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   int      miGain ;          /**< Gain value stored for unmuting */
   int      miStartRtpPort ;  /**< Requested starting rtp port */
   int      miLastRtpPort ;   /**< Requested ending rtp port */
   int      miNextRtpPort ;   /**< Next available rtp port */
   UtlSList mlistFreePorts ;  /**< List of recently freed ports */
   UtlSList mlistBusyPorts ;  /**< List of busy ports */
   OsMutex  mlockList ;       /**< Lock for port allocation */

   static size_t mnAllocCodecPaths; ///< Number of slots allocated in codec path array.
   static size_t mnCodecPaths;      ///< Number of codec paths stored.
   static UtlString* mpCodecPaths;  ///< dynamic array of paths to use when loading codecs.


     /// @brief Bind the the specified port and see if any data is ready to 
     /// read for the designated check time.
   virtual UtlBoolean isPortBusy(int iPort, int checkTimeMS) ;
     /**<
     *  @param iPort Port number to check
     *  @param checkTimeMS Number of ms to wait for data.
     */

     /// @brief Ensure that there is enough capacity in our codec paths array
   static OsStatus ensureCapacityCodecPaths(size_t newSize);
     /**<
     *  Ensure that there is enough capacity in our codec paths array for 
     *  \p newSize elements.
     *  If there is not enough capacity, then the maximum capacity is increased
     *  above newSize, unless there are so many elements that \p newSize exceeds
     *  1/2 the maximum value of a size_t, in which case, it is just allocated
     *  to fit newSize, and no greater.
     *
     *  @param[in] newSize - the size to check the codec path array allocation size against.
     *  @retval OS_SUCCESS - If everything went ok, and there is enough capacity.
     *  @retval OS_NO_MEMORY - If there was an allocation error.
     */

    void applyVideoQualityOverride(int& quality) const;
    void applyVideoBitRateOverride(int& bitRate) const;
    void applyVideoFrameRateOverride(int& frameRate) const;
    void applyVideoCpuValueOverride(int& cpuValue) const;
    void applyIdleTimeoutOverride(int& idleTimeout) const ;
    void applyAudioCodecListOverride(UtlString& codecs) const ;
    void applyVideoCodecListOverride(UtlString& codecs) const ;
    void applyVideoFormatOverride(int& videoFormat) const ;
    void applyAudioCodecOrderOverride(UtlString& codecOrder) const ;
    void applyVideoCodecOrderOverride(UtlString& codecOrder) const ;    
    void applyIgnoreCameraCapsOverride(bool& bIgnoreCameraCaps) const ;
    void applyEnableRTCPOverride(UtlBoolean& bEnableRtcp) const ;
    void getConsoleTraceOverride(bool& bEnable) const ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Disabled copy constructor
   CpMediaDeviceMgr(const CpMediaDeviceMgr& 
                               rCpMediaDeviceMgr);

     /// Disabled equals operator
   CpMediaDeviceMgr& operator=(const CpMediaDeviceMgr& rhs);   

};

#endif 

