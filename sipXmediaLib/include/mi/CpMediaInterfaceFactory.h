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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
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
///////////////////////////////////////////////////////////////////////////////


#ifndef _CpMediaInterfaceFactory_h_
#define _CpMediaInterfaceFactory_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <utl/UtlDefs.h>
#include <utl/UtlString.h>
#include "net/ProxyDescriptor.h"
#include "tapi/sipXtapi.h"

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

/* ============================ MANIPULATORS ============================== */

    /**
     * Create a media interface using the designated 
     * CpMediaIntefaceFactoryInterface.
     */ 
   IMediaInterface* createMediaInterface(const char* publicAddress,
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
                                          UtlBoolean bEnableICE,
                                          uint32_t samplesPerSec = 0 ///< Zero takes default
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

/* ============================ INQUIRY =================================== */
    int getNumAudioInputDevices();
    void getAudioInputDevices(char* deviceNameArray[], const int arraySize);
    int getNumAudioOutputDevices();
    void getAudioOutputDevices(char* deviceNameArray[], const int arraySize);


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
