//
// Copyright (C) 2004-2006 SIPfoundry Inc.
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
#include "net/SdpCodecFactory.h"
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
    MEDIA_AEC_CANCEL_AUTO,
} MEDIA_AEC_MODE ;

typedef enum MEDIA_NOISE_REDUCTION_MODE
{
    MEDIA_NOISE_REDUCTION_DISABLED,
    MEDIA_NOISE_REDUCTION_LOW,
    MEDIA_NOISE_REDUCTION_MEDIUM,
    MEDIA_NOISE_REDUCTION_HIGH,
} MEDIA_NOISE_REDUCTION_MODE ;

// FORWARD DECLARATIONS
class CpMediaInterface ;
class SdpCodec ;

/**
 *
 */
class CpMediaInterfaceFactoryImpl
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   CpMediaInterfaceFactoryImpl();    

/* =========================== DESTRUCTORS ================================ */

protected:
   /**
    * Destructor
    */
   virtual ~CpMediaInterfaceFactoryImpl();
public:

   /**
    * public interface for destroying this media interface
    */ 
   virtual void CpMediaInterfaceFactoryImpl::release();

/* ============================ MANIPULATORS ============================== */

    /**
     * Create a media interface given the designated parameters.
     */
    virtual CpMediaInterface* createMediaInterface( const char* publicAddress,
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
                                                    bool bEnableICE
                                                  ) = 0 ;


    /**
     * Set the speaker volume.  Valid range includes 0 to 50.
     */
    virtual OsStatus setSpeakerVolume(int iVolume) = 0 ;

    /**
     * Set the speaker device.
     */
    virtual OsStatus setSpeakerDevice(const UtlString& device) = 0 ;

    /**
     * Set the microphone gain.  Valid range includes 0 to 10.
     */ 
    virtual OsStatus setMicrophoneGain(int iGain) = 0 ;

    /**
     * Set the Microphone device
     */
    virtual OsStatus setMicrophoneDevice(const UtlString& device) = 0 ;    


    /**
     * Mute the microphone
     */ 
    virtual OsStatus muteMicrophone(UtlBoolean bMute) = 0 ;

    /**
     * Enable/Disable echo cancellation
     */
    virtual OsStatus setAudioAECMode(const MEDIA_AEC_MODE mode)
    {
        return OS_NOT_SUPPORTED ;
    }


    /**
     * Enable/Disable Noise Reduction
     */
    virtual OsStatus setAudioNoiseReductionMode(const MEDIA_NOISE_REDUCTION_MODE mode)
    {
        return OS_NOT_SUPPORTED ;
    }


    /**
     * Enable AGC Status
     */
    virtual OsStatus enableAGC(UtlBoolean bEnable)
    {
        return OS_NOT_SUPPORTED ;
    }


    /**
     * Enable/Disable sending DTMF tones inband
     */
    virtual OsStatus enableOutOfBandDTMF(UtlBoolean enable) 
    {
        return OS_NOT_SUPPORTED ;
    }

    /**
  	 * Enable/Disable sending DTMF tones inband
  	 */
  	virtual OsStatus enableInBandDTMF(UtlBoolean enable)
    {
        return OS_NOT_SUPPORTED ;
    }

  	 
    /**
     * Enable/Disable RTCP reports
     */
    virtual OsStatus enableRTCP(UtlBoolean bEnable)
    {
        return OS_NOT_SUPPORTED ;
    }

  	 

    /**
     * Populate the codec factory, return number of rejected codecs
     */
    virtual OsStatus buildCodecFactory(SdpCodecFactory *pFactory, 
                                       const UtlString& sAudioPreferences,
                                       const UtlString& sVideoPreferences,
                                       int videoFormat,
                                       int* iRejected) = 0;

    /**
     * Set the global video preview window 
     */ 
    virtual OsStatus setVideoPreviewDisplay(void* pDisplay) 
    {
        return OS_NOT_SUPPORTED ;
    }


    /**
     * Set the global video quality 
     */ 
    virtual OsStatus setVideoQuality(int quality) 
    {
        return OS_NOT_SUPPORTED ;
    }

        
    /**
     * Set the global video parameters 
     */ 
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

    /**
    * Set the global CPU usage
    */
    virtual OsStatus setVideoCpuValue(int cpuValue) 
    {
        return OS_NOT_SUPPORTED ;
    }

    /**
     * Update the video preview window given the specified display context.
     */ 
    virtual OsStatus updateVideoPreviewWindow(void* displayContext) 
    {
        return OS_NOT_SUPPORTED ;
    }


    /**
     * Sets the RTP port range for this factory
     */     
    virtual void setRtpPortRange(int startRtpPort, int lastRtpPort) ;

    /** 
     * Gets the next available rtp port
     */
    virtual OsStatus getNextRtpPort(int &rtpPort) ;

    /**
     * Release the rtp port back to the pool of available RTP ports
     */
    virtual OsStatus releaseRtpPort(const int rtpPort) ;

    /*
     * Set the connection idle timeout
     */
    virtual OsStatus setConnectionIdleTimeout(const int idleTimeout) 
    {
        return OS_NOT_SUPPORTED ;
    }


/* ============================ ACCESSORS ================================= */

    /**
     * Get the speaker volume
     */ 
    virtual OsStatus getSpeakerVolume(int& iVolume) const = 0 ;

    /**
     * Get the speaker device
     */ 
    virtual OsStatus getSpeakerDevice(UtlString& device) const = 0 ;

    /**
     * Get the microphone gain
     */
    virtual OsStatus getMicrophoneGain(int& iVolume) const = 0 ;

    /**
     * Get the microphone device
     */
    virtual OsStatus getMicrophoneDevice(UtlString& device) const = 0 ;

    /* 
     * Get specific codec identified by iCodec
     */
    virtual OsStatus getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const = 0;

    /* 
     * Get the connection id for the local audio connection
     */
    virtual OsStatus getLocalAudioConnectionId(int& connectionId) const = 0;

#ifdef VIDEO
    /*
     * Gets a list of UtlStrings representing available video capture devices.
     */
    virtual OsStatus getVideoCaptureDevices(UtlSList& videoDevices) const 
    {
        return OS_NOT_SUPPORTED ;
    }

    
    /*
     * Gets the current video device string.
     */
    virtual OsStatus getVideoCaptureDevice(UtlString& videoDevice) 
    {
        return OS_NOT_SUPPORTED ;
    }

    
    /* 
     * Sets the video capture device, given its string name.
     */
    virtual OsStatus setVideoCaptureDevice(const UtlString& videoDevice) 
    {
        return OS_NOT_SUPPORTED ;
    }

    
    /* 
     * Get video quality
     */
    virtual OsStatus getVideoQuality(int& quality) const 
    {
        return OS_NOT_SUPPORTED ;
    }


    /* 
     * Get video bit rate
     */
    virtual OsStatus getVideoBitRate(int& bitRate) const 
    {
        return OS_NOT_SUPPORTED ;
    }


    /* 
     * Get video frame rate
     */
    virtual OsStatus getVideoFrameRate(int& frameRate) const 
    {
        return OS_NOT_SUPPORTED ;
    }


    /* 
     * Get cpu usage
     */  
    virtual OsStatus getVideoCpuValue(int& cpuValue) const = 0;
#endif // VIDEO
    /*
     * Get the connection idle timeout
     */
    virtual OsStatus getConnectionIdleTimeout(int& idleTimeout) const 
    {
        return OS_NOT_SUPPORTED ;
    }


/* ============================ INQUIRY =================================== */

    /**
     * Return status of echo cancellation
     */
    virtual OsStatus getAudioAECMode(MEDIA_AEC_MODE& mode) const 
    {
        return OS_NOT_SUPPORTED ;
    }
        

    /**
     * Return status of noise reduction
     */
    virtual OsStatus getAudioNoiseReductionMode(MEDIA_NOISE_REDUCTION_MODE& mode) const 
    {
        return OS_NOT_SUPPORTED ;
    }


    /**
     * Return status of AGC
     */ 
    virtual OsStatus isAGCEnabled(UtlBoolean& bEnable) const
    {
        return OS_NOT_SUPPORTED ;
    }


    /**
     * Return status of out-of-band DTMF
     */
    virtual OsStatus isOutOfBandDTMFEnabled(UtlBoolean& enabled) const 
    {
        return OS_NOT_SUPPORTED ;
    }


    /**
  	 * Return status of in band DTMF
  	 */
    virtual OsStatus isInBandDTMFEnabled(UtlBoolean& enabled) const 
    {
        return OS_NOT_SUPPORTED ;
    }


/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
    int      miGain ;          /**< Gain value stored for unmuting */
    int      miStartRtpPort ;  /**< Requested starting rtp port */
    int      miLastRtpPort ;   /**< Requested ending rtp port */
    int      miNextRtpPort ;   /**< Next available rtp port */
    UtlSList mlistFreePorts ;  /**< List of recently freed ports */
    UtlSList mlistBusyPorts ;  /**< List of busy ports */
    OsMutex  mlockList ;       /**< Lock for port allocation */


    /**
     * Bind the the specified port and see if any data is ready to read for
     * the designated check time.
     *
     * @param iPort Port number to check
     * @param checkTimeMS Number of ms to wait for data.
     */
    virtual bool isPortBusy(int iPort, int checkTimeMS) ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

    /**
     * Disabled copy constructor
     */
    CpMediaInterfaceFactoryImpl(const CpMediaInterfaceFactoryImpl& 
            rCpMediaInterfaceFactoryImpl);

   /** 
    * Disabled equals operator
    */
   CpMediaInterfaceFactoryImpl& operator=(
            const CpMediaInterfaceFactoryImpl& rhs);   
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterfaceFactoryImpl_h_
