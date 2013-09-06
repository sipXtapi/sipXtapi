//
// Copyright (C) 2005-2013 SIPez LLC.  All rights reserved.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _CpMediaInterfaceFactoryImpl_h_
#define _CpMediaInterfaceFactoryImpl_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "sdp/SdpCodecList.h"
#include "utl/UtlSList.h"
#include "os/OsMutex.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

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
class CpMediaInterface ;
class SdpCodec ;
class OsMsgDispatcher ;

/**
 *
 */
class CpMediaInterfaceFactoryImpl
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

     /// @brief Default constructor
   CpMediaInterfaceFactoryImpl();    

/* =========================== DESTRUCTORS ================================ */

protected:
     /// @brief Destructor
   virtual ~CpMediaInterfaceFactoryImpl();
public:

     /// @brief Public interface for destroying this media interface
   virtual void release();

/* ============================ MANIPULATORS ============================== */

     /// @brief Create a media interface given the designated parameters.
   virtual CpMediaInterface* createMediaInterface(const char* publicAddress,
                                                  const char* localAddress,
                                                  int numCodecs,
                                                  SdpCodec* sdpCodecArray[],
                                                  const char* locale,
                                                  int expeditedIpTos,
                                                  const char* szStunServer,
                                                  int iStunPort,
                                                  int iStunKeepAliveSecs,
                                                  const char* szTurnServer,
                                                  int iTurnPort,
                                                  const char* szTurnUsername,
                                                  const char* szTurnPassword,
                                                  int iTurnKeepAliveSecs,
                                                  UtlBoolean bEnableICE, 
                                                  uint32_t samplesPerSec,
                                                  OsMsgDispatcher* pDispatcher
                                                 ) = 0 ;


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


     /// @brief Sets the RTP port range for this factory
   virtual void setRtpPortRange(int startRtpPort, int lastRtpPort) ;

     /// Gets the next available rtp port
   virtual OsStatus getNextRtpPort(const char* bindAddress, int &rtpPort);
   /**
    * @param bindAddress - address on which to find available port, may be NULL
    * @param rtpPort - next available RTP port to attempt to bind do
    */

     /// @brief Release the rtp port back to the pool of available RTP ports
   virtual OsStatus releaseRtpPort(const int rtpPort) ;

     /// @brief Set the connection idle timeout
   virtual OsStatus setConnectionIdleTimeout(const int idleTimeout) 
   {
      return OS_NOT_SUPPORTED ;
   }

     /// @copydoc CpMediaInterfaceFactory::addCodecPaths()
   static OsStatus addCodecPaths(const size_t nCodecPaths, 
                                 const UtlString codecPaths[]);

     /// @copydoc CpMediaInterfaceFactory::clearCodecPaths()
   static void clearCodecPaths();


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

#ifdef VIDEO
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
#endif // VIDEO

     /// @brief Get the connection idle timeout
   virtual OsStatus getConnectionIdleTimeout(int& idleTimeout) const 
   {
      return OS_NOT_SUPPORTED ;
   }

   /// @brief Set IP address to use in SDP (spoofed address)
   virtual void setConfiguredIpAddress(const UtlString& configuredIpAddress);

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


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   int      miInterfaceId ;   /**< Id for the next media interface */
   int      miGain ;          /**< Gain value stored for unmuting */
   int      miStartRtpPort ;  /**< Requested starting rtp port */
   int      miLastRtpPort ;   /**< Requested ending rtp port */
   int      miNextRtpPort ;   /**< Next available rtp port */
   UtlSList mlistFreePorts ;  /**< List of recently freed ports */
   UtlSList mlistBusyPorts ;  /**< List of busy ports */
   OsMutex  mlockList ;       /**< Lock for port allocation */
   UtlString mConfiguredIpAddress; /**< IP address to use for SDP c line */

   static size_t mnAllocCodecPaths; ///< Number of slots allocated in codec path array.
   static size_t mnCodecPaths;      ///< Number of codec paths stored.
   static UtlString* mpCodecPaths;  ///< dynamic array of paths to use when loading codecs.


     /// @brief Bind the the specified port and see if any data is ready to 
     /// read for the designated check time.
   UtlBoolean isAddressPortBusy(const char* bindAddress, int iPort, int checkTimeMS) ;
     /**<
     *  @param bindAddress - IP address on which to check port availablity
     *  @param iPort Port number to check
     *  @param checkTimeMS Number of ms to wait for data.
     *  Note: had a problem on x86_64 where this was not properly invoked without
     *  specifying explicitly class CpMediaInterfaceFactoryImpl scope.  So for
     *  now made this a non-virtual function. 
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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Disabled copy constructor
   CpMediaInterfaceFactoryImpl(const CpMediaInterfaceFactoryImpl& 
                               rCpMediaInterfaceFactoryImpl);

     /// Disabled equals operator
   CpMediaInterfaceFactoryImpl& operator=(const CpMediaInterfaceFactoryImpl& rhs);   
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterfaceFactoryImpl_h_
