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


// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "mi/CpMediaInterfaceFactory.h"
#include "mi/CpMediaInterfaceFactoryImpl.h"
#include "tapi/sipXtapi.h"

#ifdef _WIN32
    #include <mmsystem.h>
#endif 

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInterfaceFactory::CpMediaInterfaceFactory()
    : mpFactoryImpl(NULL)
{
}

// Destructor
CpMediaInterfaceFactory::~CpMediaInterfaceFactory()
{
}

/* ============================ MANIPULATORS ============================== */


// Create a media interface via the specified factory
IMediaInterface* CpMediaInterfaceFactory::createMediaInterface(const char* publicAddress,
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
                                          uint32_t samplesPerSec
                                         )
{
    CpMediaInterface* pInterface = NULL ;

    if (mpFactoryImpl) 
    {
        pInterface = mpFactoryImpl->createMediaInterface(publicAddress,
                localAddress,
                numCodecs,
                sdpCodecArray,
                locale, 
                expeditedIpTos,
                stunServer,
                turnProxy,
                arsProxy,
                arsHttpProxy,
                pMediaPacketCallback,
                bEnableICE,
                samplesPerSec);
    }

    return pInterface ;
}

// Static method to add codec paths
OsStatus CpMediaInterfaceFactory::addCodecPaths(const size_t nCodecPaths, const UtlString codecPaths[])
{
   return CpMediaInterfaceFactoryImpl::addCodecPaths(nCodecPaths, codecPaths);
}

void CpMediaInterfaceFactory::clearCodecPaths()
{
   CpMediaInterfaceFactoryImpl::clearCodecPaths();
}

int CpMediaInterfaceFactory::getNumAudioInputDevices()
{
    int num = 0;
#ifdef _WIN32
    num = waveInGetNumDevs();
#endif 
    return num;
}

void CpMediaInterfaceFactory::getAudioInputDevices(char* deviceNameArray[], const int arraySize)
{
    if (deviceNameArray)
    {
#  if defined (_WIN32)
        WAVEINCAPS  incaps ;
        int num = getNumAudioInputDevices();
        for (int i=0; i < num && i < arraySize; i++)
        {
            waveInGetDevCaps(i, &incaps, sizeof(WAVEINCAPS)) ;
            deviceNameArray[i] = strdup(incaps.szPname) ;
        }
#elif defined (__MACH__)
        // mac os code goes here
#else
        // linux code goes here
#endif
    }
    else
    {
        assert(false);
    }
}

int CpMediaInterfaceFactory::getNumAudioOutputDevices()
{
    int num = 0;
#ifdef WIN32
    num = waveOutGetNumDevs();
#endif 
    return num;
}

void CpMediaInterfaceFactory::getAudioOutputDevices(char* deviceNameArray[], const int arraySize)
{
    if (deviceNameArray)
    {
#  if defined (_WIN32)
        WAVEOUTCAPS  outcaps ;
        int num = getNumAudioOutputDevices();
        for (int i=0; i < num && i < arraySize; i++)
        {
            waveOutGetDevCaps(i, &outcaps, sizeof(WAVEOUTCAPS)) ;
            deviceNameArray[i] = strdup(outcaps.szPname) ;
        }
#elif defined (__MACH__)
        // mac os code goes here
#else
    // linux code goes here
#endif
    }
    else
    {
        assert(false);
    }
}

/* ============================ ACCESSORS ================================= */

CpMediaInterfaceFactoryImpl* 
CpMediaInterfaceFactory::getFactoryImplementation()
{
    return mpFactoryImpl ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



