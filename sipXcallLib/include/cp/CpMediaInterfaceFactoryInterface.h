// $Id$
//
// Copyright (C) 2005 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _CpMediaInterfaceFactoryInterface_h_
#define _CpMediaInterfaceFactoryInterface_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CpMediaInterface ;
class SdpCodec ;

/**
 *
 */
class CpMediaInterfaceFactoryInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   CpMediaInterfaceFactoryInterface();
     

   /**
    * Destructor
    */
   virtual ~CpMediaInterfaceFactoryInterface();

/* ============================ MANIPULATORS ============================== */

    /**
     * Create a media interface given the designated parameters.
     */
    virtual CpMediaInterface* createMediaInterface( int startRtpPort, 
                                                    int lastRtpPort,
                                                    const char* publicAddress,
                                                    const char* localAddress,
                                                    int numCodecs,
                                                    SdpCodec* sdpCodecArray[],
                                                    const char* locale,
                                                    int expeditedIpTos,
                                                    const char* szStunServer,
                                                    int iStunKeepAliveSecs 
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
     * Enable or disable Acoustic echo cancellation
     */
    virtual OsStatus enableAEC(UtlBoolean bEnable) = 0 ;

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

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

    /**
     * Disabled copy constructor
     */
    CpMediaInterfaceFactoryInterface(const CpMediaInterfaceFactoryInterface& 
            rCpMediaInterfaceFactoryInterface);

   /** 
    * Disabled equals operator
    */
   CpMediaInterfaceFactoryInterface& operator=(
            const CpMediaInterfaceFactoryInterface& rhs);  

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterfaceFactoryInterface_h_
