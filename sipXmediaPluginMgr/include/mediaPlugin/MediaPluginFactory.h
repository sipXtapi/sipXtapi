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
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
//
// $$
/////////////////////////////////////////////////////////////////////////////

#ifndef _MediaPluginFactory_h_
#define _MediaPluginFactory_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mediaInterface/IMediaDeviceMgr.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STRUCTS
// TYPEDEFS
typedef IMediaDeviceMgr* (*DEVICE_MGR_FACTORY_FUNC)();
typedef void (*DEVICE_MGR_DESTROY_FUNC)(void*);
typedef int  (*DEVICE_MGR_REFCOUNT_FUNC)();

// FORWARD DECLARATIONS

class MediaPluginFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
    MediaPluginFactory();
    virtual ~MediaPluginFactory();

/* ============================ MANIPULATORS ============================== */
    bool load(const char* szLibrary = NULL);
    bool unload(const char* szLibrary = NULL);

    IMediaDeviceMgr* const createMediaDeviceMgr(const char* szLibrary = NULL) const;
    void releaseMediaDeviceMgr(IMediaDeviceMgr* pDevice, const char* szLibrary = NULL) const;
    int getReferenceCount(const char* szLibrary = NULL) const;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    MediaPluginFactory(const MediaPluginFactory& rMediaPluginFactory);

    MediaPluginFactory& operator=(const MediaPluginFactory& rhs);

    static bool mbLoaded;
    static const char* spLibrary;
};

#endif 
