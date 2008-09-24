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
// 
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Mike Cohen
// 
#ifndef _UPnpControl_h_
#define _UPnpControl_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "net/Url.h"
#include "net/HttpMessage.h"
#include "utl/UtlRandom.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//////////////////////////////////////////////////////////////////////////////
class PortMapping
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    PortMapping();
    PortMapping(const PortMapping& src);
    virtual ~PortMapping();

    PortMapping& operator= (const PortMapping& src);

/* ============================ MANIPULATORS ============================== */
    void setInternalPort(const int internalPort) { mInternalPort = internalPort; }
    void setExternalPort(const int externalPort) { mExternalPort = externalPort; }
    void setClientIp(const UtlString clientIp) { mClientIp = clientIp; }

/* ============================ ACCESSORS ================================= */
    const int getInternalPort() const { return mInternalPort; }
    const int getExternalPort() const { return mExternalPort; }
    const UtlString getClientIp() const { return mClientIp; }

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */

private:
    PortMapping& copy(const PortMapping& src);
    int mInternalPort;
    int mExternalPort;
    UtlString mClientIp;
};

//////////////////////////////////////////////////////////////////////////////
class UPnpControl
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    UPnpControl(const Url controlUrl);
    UPnpControl(const UPnpControl& src);
    virtual ~UPnpControl();

    UPnpControl& operator= (const UPnpControl& src);

/* ============================ MANIPULATORS ============================== */
    bool requestConnection() const;
    bool addRandomPortMapping(int& externalPortResult,
                        const int internalPort,
                        const char* szClientAddress,
                        const int maxRequestTime,
                        const char* descriptionBase = "sipXtapi",
                        const char* protocol = "UDP",
                        const bool enabled = true) const;
    bool addPortMapping(
                        const int externalPort,
                        const int internalPort,
                        const char* szClientAddress,
                        const int maxRequestTime,
                        const char* descriptionBase = "sipXtapi",
                        const char* protocol = "UDP",
                        const bool enabled = true) const;

    bool getExternalIp(UtlString& externalIp,
                       const int maxRequestTime) const;

    bool getPortMappingEntry( const int externalPort,
                                    PortMapping* const portMapping,
                                    const int maxRequestTime,
                                    const char* protocol = "UDP") const;
    bool deletePortMapping(
                                 const int externalPort,
                                 const int maxRequestTime,
                                 const char* protocol = "UDP"
                                 ) const;
    const int getLastErrorCode() const { return nErrorCode; }
    const UtlString getLastErrorText() const { return sErrorText; }
    const UtlString getLastErrorDetail() const { return sErrorDetail; }
                                    
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    mutable UtlRandom* mpRand;
    const Url mControlUrl;
    const char* mszUsername;
    const char* mszPassword;
    mutable int nErrorCode;
    mutable UtlString sErrorText;
    mutable UtlString sErrorDetail;

    const int makeRequest(HttpMessage& request,
        HttpMessage& response,
        const int timeoutSeconds) const;

    const HttpMessage createGenericRequest(
        const char* content,
        const int contentLength) const;

    const UtlString createAddPortMappingSoapEnvelope(
        const int externalPort,
        const int internalPort,
        const char* szClientAddress,
        const char* descriptionBase = "sipXtapi",
        const char* protocol = "UDP",
        const bool enabled = true) const;

    const UtlString createGetPortMappingSoapEnvelope(
        const int externalPort,
        const char* protocol = "UDP") const;

    const UtlString createGetExternalIpSoapEnvelope() const;

    const UtlString createRequestConnectionSoapEnvelope() const;

    const UtlString createDeletePortMappingSoapEnvelope(
        const int externalPort,
        const char* protocol = "UDP") const;

    const HttpMessage createAddPortMappingRequest(
        const char* content,
        const int contentLength) const;

    const HttpMessage createGetPortMappingRequest(
        const char* content,
        const int contentLength) const;
    
    const HttpMessage createGetExternalIpRequest(
        const char* content,
        const int contentLength) const;

    const HttpMessage createRequestConnectionRequest(
        const char* content,
        const int contentLength) const;

    const HttpMessage createDeletePortMappingRequest(
        const char* content,
        const int contentLength) const;

    const UtlString parseGetExternalIp(const HttpMessage response) const;

    bool parseGetPortMappingResponse(PortMapping* const portMapping,
                                           const HttpMessage response) const;

};

#endif
