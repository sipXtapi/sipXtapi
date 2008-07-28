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
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
/////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include "os/OsDefs.h"

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <mmsystem.h>
#else
#   define DEFAULT_AUDIO_CODEC_LIST ""
#   define DEFAULT_VIDEO_CODEC_LIST ""
#endif

// APPLICATION INCLUDES
#include "mediaBaseImpl/CpMediaDeviceMgr.h"
#include "os/OsServerSocket.h"
#include "os/OsDatagramSocket.h"
#include "os/OsLock.h"

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

int CpMediaDeviceMgr::getNumAudioInputDevices()
{
    int num = 0;
#ifdef _WIN32
    num = waveInGetNumDevs();
#endif 
    return num;
}

void CpMediaDeviceMgr::getAudioInputDevices(char* deviceNameArray[], const int arraySize)
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

int CpMediaDeviceMgr::getNumAudioOutputDevices()
{
    int num = 0;
#ifdef WIN32
    num = waveOutGetNumDevs();
#endif 
    return num;
}

void CpMediaDeviceMgr::getAudioOutputDevices(char* deviceNameArray[], const int arraySize)
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
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwValue ;
        DWORD dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoQuality", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            quality = (int) dwValue ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}


void CpMediaDeviceMgr::applyVideoBitRateOverride(int& bitRate) const
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwValue ;
        DWORD dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoBitrate", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            bitRate = (int) dwValue ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}


void CpMediaDeviceMgr::applyVideoFrameRateOverride(int& frameRate) const
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwValue ;
        DWORD dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoFramerate", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            frameRate = (int) dwValue ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}


void CpMediaDeviceMgr::applyVideoCpuValueOverride(int& cpuValue) const
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwValue ;
        DWORD dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoCPU", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            cpuValue = (int) dwValue ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}


void CpMediaDeviceMgr::applyIdleTimeoutOverride(int& idleTimeout) const 
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwValue ;
        DWORD dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "audioTimeout", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            idleTimeout = (int) dwValue ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}


void CpMediaDeviceMgr::applyAudioCodecListOverride(UtlString& codecs) const 
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        char cTemp[256] ;
        DWORD dwSize = sizeof(cTemp) ;
        if (RegQueryValueEx(hKey, "audioCodecs", 0, NULL, (LPBYTE) &cTemp, &dwSize) == ERROR_SUCCESS)
        {
            codecs = cTemp ;
        }
        RegCloseKey(hKey) ;
    }
#else
    codecs = DEFAULT_AUDIO_CODEC_LIST ;
#endif

    // Adjust order
    UtlString codecOrder ;
    applyAudioCodecOrderOverride(codecOrder) ;
    SdpCodecList::applyCodecListOrdering(codecOrder, codecs) ;
}


void CpMediaDeviceMgr::applyVideoCodecListOverride(UtlString& codecs) const 
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        char cTemp[256] ;
        DWORD dwSize = sizeof(cTemp) ;
        if (RegQueryValueEx(hKey, "videoCodecs", 0, NULL, (LPBYTE) &cTemp, &dwSize) == ERROR_SUCCESS)
        {
            codecs = cTemp ;
        }
        RegCloseKey(hKey) ;
    }
#else
    codecs = DEFAULT_VIDEO_CODEC_LIST ;
#endif

    // Adjust order
    UtlString codecOrder ;
    applyVideoCodecOrderOverride(codecOrder) ;
    SdpCodecList::applyCodecListOrdering(codecOrder, codecs) ;

}

void CpMediaDeviceMgr::applyVideoFormatOverride(int& videoFormat) const 
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwValue ;
        DWORD dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoFormat", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            videoFormat = (int) dwValue ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}


void CpMediaDeviceMgr::applyAudioCodecOrderOverride(UtlString& codecOrder) const 
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        char cTemp[256] ;
        DWORD dwSize = sizeof(cTemp) ;
        if (RegQueryValueEx(hKey, "audioCodecOrder", 0, NULL, (LPBYTE) &cTemp, &dwSize) == ERROR_SUCCESS)
        {
            codecOrder = cTemp ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}

void CpMediaDeviceMgr::applyVideoCodecOrderOverride(UtlString& codecOrder) const 
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        char cTemp[256] ;
        DWORD dwSize = sizeof(cTemp) ;
        if (RegQueryValueEx(hKey, "videoCodecOrder", 0, NULL, (LPBYTE) &cTemp, &dwSize) == ERROR_SUCCESS)
        {
            codecOrder = cTemp ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}

void CpMediaDeviceMgr::applyEnableRTCPOverride(UtlBoolean& bEnableRtcp) const 
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwValue ;
        DWORD dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "enableRtcp", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            bEnableRtcp = (dwValue != 0) ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}

void CpMediaDeviceMgr::applyMediaContactTypeOverride(SIPX_CONTACT_TYPE& eType) const
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwValue ;
        DWORD dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "mediaContactType", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            eType = (SIPX_CONTACT_TYPE) dwValue ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}

void CpMediaDeviceMgr::applyIgnoreCameraCapsOverride(bool& bIgnoreCameraCaps) const 
{
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwValue ;
        DWORD dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "ignoreCameraCaps", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            bIgnoreCameraCaps = (dwValue != 0) ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}


void CpMediaDeviceMgr::getConsoleTraceOverride(bool& bEnable) const 
{
    bEnable = false ;
#ifdef _WIN32
    HKEY hKey ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwValue ;
        DWORD dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "consoleTrace", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            bEnable = (dwValue != 0) ;
        }
        RegCloseKey(hKey) ;
    }
#endif
}



