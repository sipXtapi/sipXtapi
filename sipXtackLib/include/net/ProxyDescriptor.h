// Copyright 2007 AOL LLC.
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
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
// 

#ifndef _PROXYDESCRIPTOR_H_
#define _PROXYDESCRIPTOR_H_

#include "utl/UtlString.h"

//////////////////////////////////////////////////////////////////////////////

class ProxyDescriptor
{
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

    ProxyDescriptor() ;

    ProxyDescriptor(const char* szAddress,
                    int         iPort,
                    int         iKeepAliveSecs=0);

    ProxyDescriptor(const char* szAddress,
                    int         iPort,
                    const char* szUsername,
                    const char* szPassword,
                    int         iKeepAliveSecs=0) ;

    ProxyDescriptor(const ProxyDescriptor& ref);

    ProxyDescriptor& operator=(const ProxyDescriptor& ref) ;


/* ============================ MANIPULATORS ============================== */

    void setAddress(const char* szAddress) ;
    void setPort(int iPort);
    void setUsername(const char* szUsername) ;
    void setPassword(const char* szPassword) ;
    void setKeepalive(int iKeepaliveSecs);
    void setIntParam(int param) ;

/* ============================ ACCESSORS ================================= */

    const UtlString& getAddress() const ;
    int  getPort() const ;
    const UtlString& getUsername() const ;
    const UtlString& getPassword() const ;
    int getKeepalive() const;
    int getIntParam() const;

/* ============================ INQUIRY =================================== */

    bool isValid() const ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */

protected:
    UtlString mProxyAddress ;
    int       miProxyPort ;
    UtlString mProxyUsername ;
    UtlString mProxyPassword ;
    int       miKeepAliveSecs;
    int       miParam;
} ;

#endif

//////////////////////////////////////////////////////////////////////////////

