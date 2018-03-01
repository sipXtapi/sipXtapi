//
// Copyright (C) 2005-2018 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _CpMediaInterfaceFactory_h_
#define _CpMediaInterfaceFactory_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <utl/UtlDefs.h>
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CpMediaInterfaceFactoryImpl ;
class CpMediaInterface ;
class SdpCodec ;
class OsMsgDispatcher ;

/**
 * The CpMediaInterfaceFactory is responsible for create media interfaces.  
 * Developers must install factory implementation by invoking the 
 * setFactoryImplementation(...) method.
 *
 * The destructor of this factory will automatically destroy the 
 * CpMediaIntefaceFactoryInterface.
 *
 * @nosubgrouping
 */
class CpMediaInterfaceFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   CpMediaInterfaceFactory();
     

   /**
    * Destructor
    */
   virtual ~CpMediaInterfaceFactory();

   static int (*sGetInputDeviceListFunction)(UtlContainer&);
   static int (*sGetOutputDeviceListFunction)(UtlContainer&);

/* ============================ MANIPULATORS ============================== */

    /**
     * Set the factory implementation for creating media processing 
     * connections. If an existing factory was set, it will be deleted and the
     * new factory will be used for future calls to createMediaInterface.
     *
     * The factory will automatically be deleted when the destructor of 
     * CpMediaInterfaceFactory is invoked.
     */
   void setFactoryImplementation(CpMediaInterfaceFactoryImpl* pFactoryInterface) ;


    /**
     * Create a media interface using the designated 
     * CpMediaIntefaceFactoryInterface.
     */ 
   CpMediaInterface* createMediaInterface(const char* publicAddress,
                                          const char* localAddress,
                                          int numCodecs,
                                          SdpCodec* sdpCodecArray[],
                                          const char* locale,
                                          int expeditedIpTos,
                                          const char* szStunServer,
                                          int iStunPort,
                                          int iStunKeepAlivePeriodSecs,
                                          const char* szTurnServer,
                                          int iTurnPort,
                                          const char* szTurnUsername,
                                          const char* szTurnPassword,
                                          int iTurnKeepAlivePeriodSecs,
                                          UtlBoolean bEnableICE,
                                          uint32_t samplesPerSec = 0, ///< Zero takes default
                                          OsMsgDispatcher* pDispatcher = NULL
                                         );

     /// Add directory paths to the codec search path.
   static OsStatus addCodecPaths(const size_t nCodecPaths, const UtlString codecPaths[]);
     /**<
     *  Specify directories that will be added to the list of paths
     *  used when searching for codecs to load.
     *
     *  @note The paths passed in should only contain syntactically valid 
     *        directory paths.  
     *        TODO: Syntactically invalid paths will be rejected returning OS_FAILED.
     *
     *  @param[in] codecPaths - array of paths to load. 
     *  @retval OS_SUCCESS - if all of the paths passed in are properly stored.
     */

     /// Clear the static codec path list.
   static void clearCodecPaths();


/* ============================ ACCESSORS ================================= */

   CpMediaInterfaceFactoryImpl* getFactoryImplementation() ;

   /// @brief Get list of audio input device names available
   static int getInputDeviceList(UtlContainer& deviceNames);
   
   /// @brief Get list of audio output device names available
   static int getOutputDeviceList(UtlContainer& deviceNames);
    
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   CpMediaInterfaceFactoryImpl* mpFactoryImpl ; 

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /**
    * Disabled equals operator
    */
   CpMediaInterfaceFactory& operator=(const CpMediaInterfaceFactory& rhs);  

   /**
    * Disabled copy constructor
    */
   CpMediaInterfaceFactory(const CpMediaInterfaceFactory& rCpMediaInterfaceFactory);     
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterfaceFactory_h_
