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
//////////////////////////////////////////////////////////////////////////////
// Author: Mike Cohen
// 
#ifndef _UPnpAgent_h_
#define _UPnpAgent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "upnp/UPnpDiscovery.h"
#include "upnp/UPnpService.h"
#include "upnp/UPnpControl.h"
#include "os/OsRWMutex.h"
#include "utl/UtlString.h"
#include "tapi/sipXtapi.h"

// DEFINES
#ifdef WIN32
#define WIN32_UPNP_REG_PATH "SOFTWARE\\sipxUA\\uPnpPorts"
#endif
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//////////////////////////////////////////////////////////////////////////////
class UPnpAgent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    static UPnpAgent* getInstance();
    static void release();

    virtual ~UPnpAgent();

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
    int bindToAvailablePort(const char* szClientAddress,
        const int internalPort,
        const int maxRetries = 10);
    void removeBinding(const char* szClientIp,
        const int internalPort);
    void setEnabled(const bool enabled);
    bool isEnabled();
    void setTimeoutSeconds(const int timeoutSeconds);
    int getTimeoutSeconds();
    SIPX_RESULT getLastResults(char* szInternalAddress,
                               const size_t internalAddressSize,
                               int& internalPort,
                               int& externalPort);

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UPnpAgent();
    void initialize();
    int loadPortSetting(const char* szClientAddress, const int internalPort) const;
    void savePortSetting(const char* szClientAddress, const int internalPort, const int externalPort) const;
    void setLastStatus(const char* szInternalAddress,
                       const int nInternalPort,
                       const int nExternalPort) const;
    static UPnpAgent* mpInstance;
    static OsRWMutex* spMutex;
    UPnpDiscovery* mpDiscovery;
    UPnpService* mpService;
    UtlString* mpLocation;
    UPnpControl* mpControl;
    mutable bool mLastResult;
    mutable UtlString mLastInternalAddress;
    mutable int mLastInternalPort;
    mutable int mLastExternalPort;

};

#endif
