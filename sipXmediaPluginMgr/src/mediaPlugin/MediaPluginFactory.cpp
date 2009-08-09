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
// APPLICATION INCLUDES
#include "mediaPlugin/MediaPluginFactory.h"
#include "os/OsSharedLibMgr.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
#if defined(_WIN32)
const char* MediaPluginFactory::spLibrary = "sipXmediaLib.dll";
#elif defined(__MACH__)
const char* MediaPluginFactory::spLibrary = "libsipXmedia.dylib";
#else
const char* MediaPluginFactory::spLibrary = "libsipXmedia.so";
#endif
bool MediaPluginFactory::mbLoaded = false;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
MediaPluginFactory::MediaPluginFactory()
{
}

MediaPluginFactory::~MediaPluginFactory()
{
}

/* ============================ MANIPULATORS ============================== */
bool MediaPluginFactory::load(const char* szLibrary)
{
    bool bRet = false;

    if (NULL == szLibrary)
    {
        szLibrary = spLibrary;
    }

    OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();
    OsStatus res = pShrMgr->loadSharedLib(szLibrary);
    if (res == OS_SUCCESS)
    {
        mbLoaded = true;
        bRet = true;
    }
    return bRet;
}

bool MediaPluginFactory::unload(const char* szLibrary)
{
    bool bRet = false;
    if (NULL == szLibrary)
    {
        szLibrary = spLibrary;
    }
    OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();
    OsStatus res = pShrMgr->unloadSharedLib(szLibrary);

    if (res == OS_SUCCESS)
    {
        mbLoaded = false;
        bRet = true;
    }
    return bRet;
}

IMediaDeviceMgr* const MediaPluginFactory::createMediaDeviceMgr(const char* szLibrary) const
{
    IMediaDeviceMgr* pDeviceMgr = NULL;

    if (NULL == szLibrary)
    {
        szLibrary = spLibrary;
    }
    
    if (mbLoaded)
    {
        DEVICE_MGR_FACTORY_FUNC pFactoryFunc = NULL;

        OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();
        pShrMgr->getSharedLibSymbol(szLibrary, "createMediaDeviceMgr", (void*&)pFactoryFunc);
        if (pFactoryFunc)
        {
            pDeviceMgr = pFactoryFunc();
            if (pDeviceMgr)
                pDeviceMgr->setSysLogHandler(OsSysLog::pluginSysLogHandler) ;
        }
    }
    return pDeviceMgr;
}

void MediaPluginFactory::releaseMediaDeviceMgr(IMediaDeviceMgr* pDevice, const char* szLibrary) const
{
    IMediaDeviceMgr* pDeviceMgr = NULL;

    if (NULL == szLibrary)
    {
        szLibrary = spLibrary;
    }
    
    if (mbLoaded)
    {
        DEVICE_MGR_DESTROY_FUNC pDestroyFunc = NULL;

        OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();
        pShrMgr->getSharedLibSymbol(szLibrary, "releaseMediaDeviceMgr", (void*&)pDestroyFunc);
        if (pDestroyFunc)
        {
            pDestroyFunc(pDevice);
        }
    }
    return;
}


 int MediaPluginFactory::getReferenceCount(const char* szLibrary) const
{
    int count = 0;
    if (NULL == szLibrary)
    {
        szLibrary = spLibrary;
    }    
    if (mbLoaded)
    {
        DEVICE_MGR_REFCOUNT_FUNC pFunc = NULL;

        OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();
        pShrMgr->getSharedLibSymbol(szLibrary, "getReferenceCount", (void*&)pFunc);
        if (pFunc)
        {
            count = pFunc();
        }
    }
    return count;
}


/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */
