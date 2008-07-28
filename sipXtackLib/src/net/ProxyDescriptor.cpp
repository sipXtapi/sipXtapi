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
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
// 

#include "net/ProxyDescriptor.h"

//////////////////////////////////////////////////////////////////////////////

ProxyDescriptor::ProxyDescriptor() 
{
    miProxyPort = 0 ;
    miKeepAliveSecs = 0 ;
    miParam = 0 ;
}

//////////////////////////////////////////////////////////////////////////////

ProxyDescriptor::ProxyDescriptor(const char* szAddress,
                                 int         iPort,
                                 int         iKeepAliveSecs)
{
    mProxyAddress = szAddress ;
    miProxyPort = iPort ;
    miKeepAliveSecs = iKeepAliveSecs ;
    miParam = 0 ;
}

//////////////////////////////////////////////////////////////////////////////

ProxyDescriptor::ProxyDescriptor(const char* szAddress,
                                 int         iPort,
                                 const char* szUsername,
                                 const char* szPassword,
                                 int         iKeepAliveSecs)
{
    mProxyAddress = szAddress ;
    miProxyPort = iPort ;
    mProxyUsername = szUsername ;
    mProxyPassword = szPassword ;
    miKeepAliveSecs = iKeepAliveSecs ;
    miParam = 0 ;
}

//////////////////////////////////////////////////////////////////////////////

ProxyDescriptor::ProxyDescriptor(const ProxyDescriptor& ref)
{
    if (this != &ref)
    {
        mProxyAddress = ref.mProxyAddress ;
        miProxyPort = ref.miProxyPort ;
        mProxyUsername = ref.mProxyUsername ;
        mProxyPassword = ref.mProxyPassword ;
        miKeepAliveSecs = ref.miKeepAliveSecs;
        miParam = ref.miParam;
    }
}

//////////////////////////////////////////////////////////////////////////////

ProxyDescriptor& ProxyDescriptor::operator=(const ProxyDescriptor& ref) 
{
    if (this != &ref)
    {
        mProxyAddress = ref.mProxyAddress ;
        miProxyPort = ref.miProxyPort ;
        mProxyUsername = ref.mProxyUsername ;
        mProxyPassword = ref.mProxyPassword ;
        miKeepAliveSecs = ref.miKeepAliveSecs;
        miParam = ref.miParam;
    }

    return *this;
}

//////////////////////////////////////////////////////////////////////////////

void ProxyDescriptor::setAddress(const char* szAddress) 
{
    mProxyAddress = szAddress ;
}

//////////////////////////////////////////////////////////////////////////////

void ProxyDescriptor::setPort(int iPort)
{
    miProxyPort = iPort ;
}

//////////////////////////////////////////////////////////////////////////////

void ProxyDescriptor::setUsername(const char* szUsername) 
{
    mProxyUsername = szUsername ;
}

//////////////////////////////////////////////////////////////////////////////

void ProxyDescriptor::setPassword(const char* szPassword) 
{
    mProxyPassword = szPassword ;
}

//////////////////////////////////////////////////////////////////////////////

void ProxyDescriptor::setKeepalive(int iKeepaliveSecs) 
{ 
    miKeepAliveSecs = iKeepaliveSecs ;
}

//////////////////////////////////////////////////////////////////////////////

void ProxyDescriptor::setIntParam(int iParam) 
{
    miParam = iParam ;
}

//////////////////////////////////////////////////////////////////////////////

const UtlString&  ProxyDescriptor::getAddress() const 
{
    return mProxyAddress ;
}

//////////////////////////////////////////////////////////////////////////////

int ProxyDescriptor::getPort() const 
{
    return miProxyPort ;
}

//////////////////////////////////////////////////////////////////////////////

const UtlString& ProxyDescriptor::getUsername() const 
{
    return mProxyUsername ;
}

//////////////////////////////////////////////////////////////////////////////

const UtlString& ProxyDescriptor::getPassword() const 
{
    return mProxyPassword ;
}

//////////////////////////////////////////////////////////////////////////////

int ProxyDescriptor::getKeepalive() const
{
    return miKeepAliveSecs ;
}

//////////////////////////////////////////////////////////////////////////////

int ProxyDescriptor::getIntParam() const
{
    return miParam ;
}

//////////////////////////////////////////////////////////////////////////////

bool ProxyDescriptor::isValid() const 
{
    return (mProxyAddress.length() > 0 && portIsValid(miProxyPort)) ;

}

//////////////////////////////////////////////////////////////////////////////
