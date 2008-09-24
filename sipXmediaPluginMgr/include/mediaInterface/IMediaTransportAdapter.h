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
// Copyright (C) 2005-2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.//
//
// $$
/////////////////////////////////////////////////////////////////////////////

#ifndef _IMediaTransportAdapter_h_
#define _IMediaTransportAdapter_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "ARS/ArsConnectionSocket.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

class IMediaTransportAdapter
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    virtual ~IMediaTransportAdapter() { }
    virtual void setRtpSocket(IOsNatSocket* pSocket) = 0;
    virtual void setRtcpSocket(IOsNatSocket* pSocket) = 0;
    virtual IOsNatSocket* getRtpSocket() = 0;
    virtual IOsNatSocket* getRtcpSocket() = 0;
    virtual int doSendPacket(int channel, const void *data, int len) = 0;
    virtual int doSendRTCPPacket(int channel, const void *data, int len) = 0;
    virtual void setRtpDestination(int iDestinationPort, const UtlString& destinationAddress) = 0;
    virtual void setRtcpDestination(int iDestinationPort, const UtlString& destinationAddress) = 0;

    // Sets Ars socket and assumes ars mode
    virtual void setArsSocket(ArsConnectionSocket* pArsSocket) = 0;
};

#endif 

