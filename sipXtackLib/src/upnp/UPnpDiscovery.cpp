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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "upnp/UPnpDiscovery.h"
#include "upnp/UPnpService.h"
#include "net/HttpMessage.h"
#include "net/Url.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS

//////////////////////////////////////////////////////////////////////////////
UPnpDiscovery::UPnpDiscovery(const int timeout) :
    mSrcIp("0.0.0.0"),
    mSrcPort(0),
    mpSocket(NULL),
    mTimeout(timeout)
{
}

//////////////////////////////////////////////////////////////////////////////
UPnpDiscovery::UPnpDiscovery(const UtlString srcIp,
                             const int srcPort,
                             const int timeout) :
    mSrcIp(srcIp),
    mSrcPort(srcPort),
    mpSocket(NULL),
    mTimeout(timeout)
{
}

//////////////////////////////////////////////////////////////////////////////
UPnpDiscovery::UPnpDiscovery(const UPnpDiscovery& src)
{
}

//////////////////////////////////////////////////////////////////////////////
UPnpDiscovery::~UPnpDiscovery()
{
    delete mpSocket;
}

//////////////////////////////////////////////////////////////////////////////
UPnpDiscovery& UPnpDiscovery::operator= (const UPnpDiscovery& src)
{
    if (&src == this)
    {
        return *this;
    }

    return *this;
}

//////////////////////////////////////////////////////////////////////////////
UtlString UPnpDiscovery::discoverRootLocation() const
{
    UtlString serviceType("upnp:rootdevice");
    UtlString location("");

    HttpMessage response = sendDiscoveryRequest(createDiscoveryRequest(serviceType), mTimeout);
    location = getLocation(response);

    return location;
}

//////////////////////////////////////////////////////////////////////////////
UtlString UPnpDiscovery::discoverGatewayLocation() const
{
    UtlString serviceType("urn:schemas-upnp-org:device:InternetGatewayDevice:1");
    UtlString location("");

    HttpMessage response = sendDiscoveryRequest(createDiscoveryRequest(serviceType), mTimeout);
    location = getLocation(response);

    return location;
}

//////////////////////////////////////////////////////////////////////////////
UtlString UPnpDiscovery::discoverWANIPConnectionLocation() const
{
    OsSysLog::add(FAC_SIP, PRI_INFO,
        "UPnpDiscovery::discoverWANIPConnectionLocation() begin");

    UtlString serviceType(UPnpService::WANIPConnection);
    UtlString location("");

    HttpMessage response = sendDiscoveryRequest(createDiscoveryRequest(serviceType), mTimeout);
    location = getLocation(response);

    OsSysLog::add(FAC_SIP, PRI_INFO,
        "UPnpDiscovery::discoverWANIPConnectionLocation() location = %s", location.data() );

    return location;
}

//////////////////////////////////////////////////////////////////////////////
const HttpMessage UPnpDiscovery::sendDiscoveryRequest(const HttpMessage& message,
                                                      const int timeout) const
{
    HttpMessage response;
    Url url("http://239.255.255.250:1900/*", true);
    int status_code = response.get(url, (HttpMessage&)message, timeout, true);
    return response;
}

//////////////////////////////////////////////////////////////////////////////
const HttpMessage UPnpDiscovery::createDiscoveryRequest(const UtlString serviceType) const
{
    HttpMessage msg;

    msg.setFirstHeaderLine("M-SEARCH", "*", HTTP_PROTOCOL_VERSION_1_1);
    msg.setSendAddress(mSrcIp.data(), mSrcPort);
    msg.addHeaderField("host", "239.255.255.250:1900");
    msg.addHeaderField("ST", serviceType.data());
    msg.addHeaderField("Man", "\"ssdp:discover\"");
    msg.addHeaderField("MX", "3");

    return msg;
}

//////////////////////////////////////////////////////////////////////////////
const UtlString UPnpDiscovery::getLocation(const HttpMessage response) const
{
    UtlString location = response.getHeaderValue(0, "LOCATION");
    return location;
}
