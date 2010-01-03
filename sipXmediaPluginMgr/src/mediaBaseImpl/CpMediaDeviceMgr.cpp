// Copyright 2008-2009 AOL LLC.
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
// Copyright (C) 2008-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
/////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <mmsystem.h>
#else
#   define DEFAULT_AUDIO_CODEC_LIST ""
#   define DEFAULT_VIDEO_CODEC_LIST ""
#endif

#ifdef __MACH__
#   include <CoreAudio/CoreAudio.h>
#   include <CoreAudio/CoreAudioTypes.h>
#endif

// APPLICATION INCLUDES
#include "mediaBaseImpl/CpMediaDeviceMgr.h"
#include "os/OsServerSocket.h"
#include "os/OsDatagramSocket.h"
#include "os/OsLock.h"
#include "os/OsRegistry.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

void CpMediaDeviceMgr::freeDeviceList(char* deviceList[])
{
    for (int i=0; i<MAX_AUDIO_DEVICES; i++)
    {
        if (deviceList[i])
        {
            free(deviceList[i]) ;
            deviceList[i] = NULL ;
        }
    }
}

int CpMediaDeviceMgr::getNumAudioInputDevices() const
{
    int num = 0;
#if defined (_WIN32)
    num = waveOutGetNumDevs();
#elif defined (__MACH__)
    num = getNumAudioDevices(true);
#endif
    return num;
}

void CpMediaDeviceMgr::getAudioInputDevices(char* deviceNameArray[], void* deviceHandleArray[], const int arraySize) const
{
    if (deviceNameArray)
    {
#  if defined (_WIN32)
        WAVEINCAPS  incaps ;
        int num = getNumAudioInputDevices();
        for (int i=0; i < num && i < arraySize; i++)
        {
            waveInGetDevCaps(i, &incaps, sizeof(WAVEINCAPS)) ;
            if (deviceNameArray)
            {
                deviceNameArray[i] = strdup(incaps.szPname) ;
            }
            if (deviceHandleArray)
            {
                deviceHandleArray[i] = (void*) i ;
            }
        }
#elif defined (__MACH__)
        getAudioDevices(true, deviceNameArray, deviceHandleArray, arraySize);
#else
        // linux code goes here
#endif
    }
    else
    {
        assert(false);
    }
}

int CpMediaDeviceMgr::getNumAudioOutputDevices() const
{
    int num = 0;
#if defined (_WIN32)
    num = waveOutGetNumDevs();
#elif defined (__MACH__)
    num = getNumAudioDevices(false);
#endif
    return num;
}

#ifdef __MACH__
void CpMediaDeviceMgr::getAudioDevices(bool bIsInput, char* deviceNameArray[], void* deviceHandleArray[], const int arraySize) const
{
	UInt32 nDevices = 0;
//		CFStringRef	devUID;
	AudioDeviceID* pDeviceList;
	char szName[1024];
	OSStatus rc;
	UInt32 nSize;

	// get the property info and size for kAudioHardwarePropertyDevices
	rc = AudioHardwareGetPropertyInfo (kAudioHardwarePropertyDevices,
										  &nSize,
										  NULL);
	if (rc)
	{
        OsSysLog::add(FAC_MP,
        		PRI_ERR,
        		"Failed to retrieve property info for kAudioHardwarePropertyDevices, errno=%d",
                rc) ;
        return;
	}

	// determine number of hardware devices from nSize
	nDevices = nSize / sizeof(AudioDeviceID);
	// allocate device list
	pDeviceList = (AudioDeviceID*) malloc(nDevices * sizeof(AudioDeviceID));
	memset(pDeviceList, 0, nDevices * sizeof(AudioDeviceID));
	nSize = nDevices * sizeof(AudioDeviceID);

    // get the device list
	rc = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &nSize, pDeviceList );
	if (rc)
	{
        OsSysLog::add(FAC_MP,
        		PRI_ERR,
        		"Failed to retrieve property:  kAudioHardwarePropertyDevices, errno=%d",
                rc) ;
        return;
	}

	int nDeviceCount = 0;  // count of devices that have the indicated channel type (input or output)
	for (unsigned int i = 0; i < nDevices; i++)
	{
		int nChannelCount = 0;
		rc = AudioDeviceGetPropertyInfo(
			pDeviceList[i],
			0,
			bIsInput,
			kAudioDevicePropertyStreamConfiguration,
			&nSize,
			NULL);
		if (rc)
		{
	        OsSysLog::add(FAC_MP,
	        		PRI_ERR,
	        		"Failed to retrieve property info for:  kAudioDevicePropertyStreamConfiguration, errno=%d",
	                rc) ;
	        continue;
		}

		AudioBufferList* pBufferList = (AudioBufferList *) malloc(nSize);
        memset(pBufferList, 0, nSize);
		rc = AudioDeviceGetProperty(
			pDeviceList[i],
			0,
			bIsInput,
			kAudioDevicePropertyStreamConfiguration,
			&nSize,
			pBufferList);

		if (rc)
		{
	        OsSysLog::add(FAC_MP,
	        		PRI_ERR,
	        		"Failed to retrieve property:  kAudioDevicePropertyStreamConfiguration, errno=%d",
	                rc) ;
		}
		nChannelCount = pBufferList->mNumberBuffers;

		// skip devices without any of the desired (input/output) channels
		if(rc || nChannelCount==0)
		{
			free(pBufferList);
			continue;
		}
		// output some device info
		nSize=sizeof(szName)-1;


		// get the device name
		rc = AudioDeviceGetProperty(
	        pDeviceList[i],
			0,
			bIsInput,
			kAudioDevicePropertyDeviceName,
			&nSize,
			szName);

		if (rc)
		{
	        OsSysLog::add(FAC_MP,
	        		PRI_ERR,
	        		"Failed to retrieve property:  kAudioDevicePropertyDeviceName, errno=%d",
	                rc) ;
	        return;
		}

        if (deviceNameArray)
        {
    
            UtlString cleanedName(szName) ;
            cleanedName.strip(UtlString::trailing) ;
            deviceNameArray[nDeviceCount] = strdup(cleanedName);
        }

        if (deviceHandleArray)
        {
            deviceHandleArray[nDeviceCount] = (void*) (pDeviceList[i]) ;
        }
        nDeviceCount++ ;
	}
}

int CpMediaDeviceMgr::getNumAudioDevices(bool bIsInput) const
{
	UInt32 nSize;
	OSStatus rc;
	int nDeviceCount = 0;
	rc = AudioHardwareGetPropertyInfo (kAudioHardwarePropertyDevices,
										  &nSize,
										  NULL);
	if (rc)
	{
        OsSysLog::add(FAC_MP,
        		PRI_ERR,
        		"Failed to retrieve property info:  kAudioHardwarePropertyDevices, errno=%d",
                rc) ;
        return -1;
	}

	UInt32 nDevices;
	AudioDeviceID* pDeviceList;
	nDevices = nSize / sizeof(AudioDeviceID);
	pDeviceList = (AudioDeviceID*) malloc(nDevices * sizeof(AudioDeviceID));
	memset(pDeviceList, 0, sizeof(AudioDeviceID));
	nSize = nDevices * sizeof(AudioDeviceID);

    // get the device list
    rc = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &nSize, pDeviceList );
    if (rc)
    {
        OsSysLog::add(FAC_MP,
            PRI_ERR,
            "Failed to retrieve property:  kAudioHardwarePropertyDevices, errno=%d",
            rc) ;
        return -1;
    }

	for (unsigned int i = 0; i < nDevices; i++)
	{
		rc = AudioDeviceGetPropertyInfo(
			pDeviceList[i],
			0,
			bIsInput,
			kAudioDevicePropertyStreamConfiguration,
			&nSize,
			NULL);

		if (rc)
		{
	        OsSysLog::add(FAC_MP,
	        		PRI_ERR,
	        		"Failed to retrieve property info:  kAudioDevicePropertyStreamConfiguration, errno=%d",
	                rc) ;
	        continue;
		}

		AudioBufferList *pBufferList = (AudioBufferList *) malloc(nSize);
		memset(pBufferList, 0, nSize);
		rc = AudioDeviceGetProperty(
			pDeviceList[i],
			0,
			bIsInput,
			kAudioDevicePropertyStreamConfiguration,
			&nSize,
			pBufferList
			);

		if (rc)
		{
	        OsSysLog::add(FAC_MP,
	        		PRI_ERR,
	        		"Failed to retrieve property:  kAudioDevicePropertyStreamConfiguration, errno=%d",
	                rc) ;
	        free(pBufferList);
	        continue;
		}

		if (!pBufferList->mNumberBuffers)
		{
			free(pBufferList);
			continue;
		}

		nDeviceCount++;
		free(pBufferList);
	}
	return nDeviceCount;
}
#endif


void CpMediaDeviceMgr::getAudioOutputDevices(char* deviceNameArray[], void* deviceHandleArray[], const int arraySize) const
{
    if (deviceNameArray)
    {
#  if defined (_WIN32)
        WAVEOUTCAPS  outcaps ;
        int num = getNumAudioOutputDevices();
        for (int i=0; i < num && i < arraySize; i++)
        {
            waveOutGetDevCaps(i, &outcaps, sizeof(WAVEOUTCAPS)) ;
            if (deviceNameArray)
            {
                deviceNameArray[i] = strdup(outcaps.szPname) ;
            }

            if (deviceHandleArray)
            {
                deviceHandleArray[i] = (void*) i ;
            }
        }
#elif defined (__MACH__)
        getAudioDevices(false, deviceNameArray, deviceHandleArray, arraySize);
#else
    // linux code goes here
#endif
    }
    else
    {
        assert(false);
    }
}
#define MAX_PORT_CHECK_ATTEMPTS     miLastRtpPort - miStartRtpPort
#define MAX_PORT_CHECK_WAIT_MS      50
OsStatus CpMediaDeviceMgr::getNextRtpPort(int &rtpPort) 
{
    OsLock lock(mlockList) ;
    bool bGoodPort = false ;
    int iAttempts = 0 ;

    // Re-add busy ports to end of free list
    while (mlistBusyPorts.entries())
    {
        UtlInt* pInt = (UtlInt*) mlistBusyPorts.first() ;
        mlistBusyPorts.remove(pInt) ;
        mlistFreePorts.append(pInt) ;
    }

    while (!bGoodPort && (iAttempts < MAX_PORT_CHECK_ATTEMPTS))
    {
        iAttempts++ ;

        // First attempt to get a free port for the free list, if that
        // fails, return a new one. 
        if (mlistFreePorts.entries())
        {
            UtlInt* pInt = (UtlInt*) mlistFreePorts.first() ;
            mlistFreePorts.remove(pInt) ;
            rtpPort = pInt->getValue() ;
            delete pInt ;
        }
        else
        {
            rtpPort = miNextRtpPort ;

            // Only allocate if the nextRtpPort is greater then 0 -- otherwise we
            // are allowing the system to allocate ports.
            if (miNextRtpPort > 0)
            {
                miNextRtpPort ++ ;
            }
        }

        bGoodPort = !isPortBusy(rtpPort, MAX_PORT_CHECK_WAIT_MS) ;
        if (!bGoodPort)
        {
            mlistBusyPorts.insert(new UtlInt(rtpPort)) ;
        }
    }

    // If unable to find a usable port, let the system pick one.
    if (!bGoodPort)
    {
        rtpPort = 0 ;
    }
    
    return OS_SUCCESS ;
}

void CpMediaDeviceMgr::setRtpPortRange(int startRtpPort, int lastRtpPort) 
{
    miStartRtpPort = startRtpPort ;
    if (miStartRtpPort < 0)
    {
        miStartRtpPort = 0 ;
    }
    miLastRtpPort = lastRtpPort ;
    miNextRtpPort = miStartRtpPort ;
}

UtlBoolean CpMediaDeviceMgr::isPortBusy(int iPort, int checkTimeMS) 
{
    UtlBoolean bBusy = FALSE ;

    if (iPort > 0)
    {
        OsDatagramSocket* pSocket = new OsDatagramSocket(0, NULL, iPort, NULL) ;
        if (pSocket != NULL)
        {
            if (!pSocket->isOk() || pSocket->isReadyToRead(checkTimeMS))
            {
                bBusy = true ;
            }
            pSocket->close() ;
            delete pSocket ;
        }
        
        if (!bBusy)
        {
            // also check TCP port availability
            OsServerSocket* pTcpSocket = new OsServerSocket(64, iPort, 0, 1);
            if (pTcpSocket != NULL)
            {
                if (!pTcpSocket->isOk())
                {
                    bBusy = TRUE;
                }
                pTcpSocket->close();
                delete pTcpSocket;
            }
        }
    }

    return bBusy ;
}

OsStatus CpMediaDeviceMgr::releaseRtpPort(const int rtpPort) 
{
    OsLock lock(mlockList) ;

    // Only bother noting the free port if the next port isn't 0 (OS selects 
    // port)
    if (miNextRtpPort != 0)
    {
        UtlInt key(rtpPort) ;

        // if it is not already in the list...
        if (!mlistFreePorts.find(&key))
        {
            // Release port to head of list (generally want to reuse ports)
            mlistFreePorts.insert(new UtlInt(rtpPort)) ;
        }
    }

    return OS_SUCCESS ;
}

OsStatus CpMediaDeviceMgr::translateToneId(const SIPX_TONE_ID toneId,
                                             SIPX_TONE_ID&      xlateId ) const
{
    OsStatus rc = OS_SUCCESS;
    if (toneId >= '0' && toneId <= '9')
    {
        xlateId = (SIPX_TONE_ID)(toneId - '0');
    } 
    else if (toneId == ID_DTMF_STAR)
    {
        xlateId = (SIPX_TONE_ID)10;
    }
    else if (toneId == ID_DTMF_POUND)
    {
        xlateId = (SIPX_TONE_ID)11;
    }
    else if (toneId == ID_DTMF_FLASH)
    {
        xlateId = (SIPX_TONE_ID)16;
    }
    else
    {
        rc = OS_FAILED;
    }
    return rc;
}

void CpMediaDeviceMgr::applyVideoQualityOverride(int& quality) const
{
    OsRegistry reg;
    reg.readInteger(SIPXTAPI_OVERRIDE_KEY, "videoQuality", quality);
}


void CpMediaDeviceMgr::applyVideoBitRateOverride(int& bitRate) const
{
    OsRegistry reg;
    reg.readInteger(SIPXTAPI_OVERRIDE_KEY, "videoBitrate", bitRate);
}


void CpMediaDeviceMgr::applyVideoFrameRateOverride(int& frameRate) const
{
    OsRegistry reg;
    reg.readInteger(SIPXTAPI_OVERRIDE_KEY, "videoFramerate", frameRate);
}


void CpMediaDeviceMgr::applyVideoCpuValueOverride(int& cpuValue) const
{
    OsRegistry reg;
    reg.readInteger(SIPXTAPI_OVERRIDE_KEY, "videoCPU", cpuValue);
}

void CpMediaDeviceMgr::applyIdleTimeoutOverride(int& idleTimeout) const 
{
    OsRegistry reg;
    reg.readInteger(SIPXTAPI_OVERRIDE_KEY, "audioTimeout", idleTimeout);
}


void CpMediaDeviceMgr::applyAudioCodecListOverride(UtlString& codecs) const 
{
    OsRegistry reg;
    reg.readString(SIPXTAPI_OVERRIDE_KEY, "audioCodecs", codecs);

    // Adjust order
    UtlString codecOrder ;
    applyAudioCodecOrderOverride(codecOrder) ;
    SdpCodecList::applyCodecListOrdering(codecOrder, codecs) ;
}


void CpMediaDeviceMgr::applyVideoCodecListOverride(UtlString& codecs) const 
{
    OsRegistry reg;
    reg.readString(SIPXTAPI_OVERRIDE_KEY, "videoCodecs", codecs);

    // Adjust order
    UtlString codecOrder ;
    applyVideoCodecOrderOverride(codecOrder) ;
    SdpCodecList::applyCodecListOrdering(codecOrder, codecs) ;

}

void CpMediaDeviceMgr::applyVideoFormatOverride(int& videoFormat) const 
{
    OsRegistry reg;
    reg.readInteger(SIPXTAPI_OVERRIDE_KEY, "videoFormat", videoFormat);
}


void CpMediaDeviceMgr::applyAudioCodecOrderOverride(UtlString& codecOrder) const 
{
    OsRegistry reg;
    reg.readString(SIPXTAPI_OVERRIDE_KEY, "audioCodecOrder", codecOrder);
}

void CpMediaDeviceMgr::applyVideoCodecOrderOverride(UtlString& codecOrder) const 
{
    OsRegistry reg;
    reg.readString(SIPXTAPI_OVERRIDE_KEY, "videoCodecOrder", codecOrder);
}

void CpMediaDeviceMgr::applyEnableRTCPOverride(UtlBoolean& bEnableRtcp) const 
{
    OsRegistry reg;
    reg.readInteger(SIPXTAPI_OVERRIDE_KEY, "enableRtcp", bEnableRtcp);
}

void CpMediaDeviceMgr::applyMediaContactTypeOverride(SIPX_CONTACT_TYPE& eType) const
{
    OsRegistry reg;
    reg.readInteger(SIPXTAPI_OVERRIDE_KEY, "mediaContactType", (int&)eType);
}

void CpMediaDeviceMgr::applyIgnoreCameraCapsOverride(bool& bIgnoreCameraCaps) const 
{
    OsRegistry reg;
    UtlBoolean temp = 0;
    reg.readInteger(SIPXTAPI_OVERRIDE_KEY, "ignoreCameraCaps", temp);
    if (temp)
    {
        bIgnoreCameraCaps = true;
    }
}


void CpMediaDeviceMgr::getConsoleTraceOverride(bool& bEnable) const 
{
    bEnable = false ;
    OsRegistry reg;
    UtlBoolean temp = 0;
    reg.readInteger(SIPXTAPI_OVERRIDE_KEY, "consoleTrace", temp);
    if (temp)
    {
        bEnable = true;
    }
}




