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
#ifndef _UPnpService_h_
#define _UPnpService_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/Url.h"
#include "utl/UtlString.h"
#include <xmlparser/tinyxml.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//////////////////////////////////////////////////////////////////////////////
class UPnpXmlNavigator
{
public:
    static TiXmlNode* getFirstDescendant(TiXmlNode* root,
                                         const char* elemName,
                                         const char* text);
};

class UPnpService
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    UPnpService();
    UPnpService(const UtlString xml,
                const UtlString serviceType);
    UPnpService(const Url rootXmlUrl,
                const UtlString serviceType);
    UPnpService(const UPnpService& src);
    virtual ~UPnpService();

    UPnpService& operator= (const UPnpService& src);

    static const char* WANIPConnection;

    UtlString getControlUrl() { return mControlUrl; }

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    bool parseXml(const UtlString& xml);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlString mServiceType;
    UtlString mServiceId;
    UtlString mControlUrl;
    UtlString mEventSubUrl;
    UtlString mScpdUrl;
    const UtlString makeUrl(const char* path, const char* base) const;
};

#endif
