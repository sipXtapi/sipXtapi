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
// Author: Mike Cohen
// 

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsHostPort.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS

//////////////////////////////////////////////////////////////////////////////
OsHostPort::OsHostPort(const UtlString host,
                       const int port) :
    mHost(host),
    mPort(port)
{
}

//////////////////////////////////////////////////////////////////////////////
OsHostPort::OsHostPort(const OsHostPort& src) :
    mHost(src.mHost),
        mPort(src.mPort)
{
}

//////////////////////////////////////////////////////////////////////////////
OsHostPort::~OsHostPort()
{
}

//////////////////////////////////////////////////////////////////////////////
OsHostPort& OsHostPort::operator= (const OsHostPort& src)
{
    if (&src == this)
    {
        return *this;
    }
    this->mHost = src.mHost;
    this->mPort = src.mPort;
    return *this;
}

//////////////////////////////////////////////////////////////////////////////
const UtlString& OsHostPort::getHost() const
{
    return mHost;
}

//////////////////////////////////////////////////////////////////////////////
const int OsHostPort::getPort() const
{
    return mPort;
}

