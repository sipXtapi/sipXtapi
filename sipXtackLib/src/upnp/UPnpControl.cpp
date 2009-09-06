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

// SYSTEM INCLUDES
#include <stdio.h>

// APPLICATION INCLUDES
#include "upnp/UPnpControl.h"
#include "net/HttpMessage.h"
#include <xmlparser/tinyxml.h>
#include "upnp/UPnpService.h" // for UPnpXmlNavigator
#include "os/OsSysLog.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
#ifdef _WIN32
#define strcasecmp stricmp
#endif

//////////////////////////////////////////////////////////////////////////////
PortMapping::PortMapping() :
    mExternalPort(0),
    mInternalPort(0)
{
}

//////////////////////////////////////////////////////////////////////////////
PortMapping::PortMapping(const PortMapping& src)
{
    copy(src);
}

//////////////////////////////////////////////////////////////////////////////
PortMapping& PortMapping::operator= (const PortMapping& src)
{
    if (&src == this)
    {
        return *this;
    }
    return copy(src);
}

//////////////////////////////////////////////////////////////////////////////
PortMapping::~PortMapping()
{
}

//////////////////////////////////////////////////////////////////////////////
PortMapping& PortMapping::copy(const PortMapping& src)
{
    this->mExternalPort = src.mExternalPort;
    this->mInternalPort = src.mInternalPort;
    this->mClientIp = src.mClientIp;
    return *this;
}

//////////////////////////////////////////////////////////////////////////////
UPnpControl::UPnpControl(const Url controlUrl) :
    mControlUrl(controlUrl),
    nErrorCode(0),
    sErrorText(""),
    sErrorDetail(""),
    mpRand(NULL)
{
}

//////////////////////////////////////////////////////////////////////////////
UPnpControl::UPnpControl(const UPnpControl& src)
{
}

//////////////////////////////////////////////////////////////////////////////
UPnpControl::~UPnpControl()
{
}

//////////////////////////////////////////////////////////////////////////////
UPnpControl& UPnpControl::operator= (const UPnpControl& src)
{
    if (&src == this)
    {
        return *this;
    }
    return *this;
}

//////////////////////////////////////////////////////////////////////////////
bool UPnpControl::requestConnection() const
{
    bool bRet = false;
    
    UtlString envelope = createRequestConnectionSoapEnvelope();
    HttpMessage request = createRequestConnectionRequest(envelope.data(),
                                                   envelope.length());
    
    HttpMessage response;
    nErrorCode = makeRequest(request, response, 0);
    if (200 == nErrorCode)
    {
        bRet = true;
    }
    return bRet;
}

//////////////////////////////////////////////////////////////////////////////
bool UPnpControl::addRandomPortMapping(int& externalPortResult,
                        const int internalPort,
                        const char* szClientAddress,
                        const int maxRequestTime,
                        const char* descriptionBase,
                        const char* protocol,
                        const bool enabled) const
{
    if (mpRand == NULL)
    {
        mpRand = new UtlRandom(OsDateTime::getCurTimeInMS());
    }
    
    unsigned short randPort = (((unsigned int)mpRand->rand()) % 64551) + 1024;
    externalPortResult = randPort;
    return addPortMapping((const int)randPort,
                          internalPort,
                          szClientAddress,
                          maxRequestTime,
                          descriptionBase,
                          protocol,
                          enabled);
}

//////////////////////////////////////////////////////////////////////////////
bool UPnpControl::addPortMapping(const int externalPort,
                        const int internalPort,
                        const char* szClientAddress,
                        const int maxRequestTime,
                        const char* descriptionBase,
                        const char* protocol,
                        const bool enabled) const
{
    bool bRet = false;
    
    UtlString envelope = createAddPortMappingSoapEnvelope(externalPort,
                                                       internalPort,
                                                       szClientAddress,
                                                       descriptionBase,
                                                       protocol,
                                                       enabled);
    HttpMessage request = createAddPortMappingRequest(envelope.data(),
                                                   envelope.length());

    HttpMessage response;
    nErrorCode = makeRequest(request, response, maxRequestTime);
    UtlString responseString;
    int len;
    response.getBytes(&responseString, &len);
    if (200 == nErrorCode)
    {
        bRet = true;
        OsSysLog::add(FAC_NAT, PRI_INFO,
            "UPnpControl::addPortMapping - success.\n");
    }
    else
    {
        OsSysLog::add(FAC_NAT, PRI_INFO,
            "UPnpControl::addPortMapping - failure.\n");
    }
    
    return bRet;
}

//////////////////////////////////////////////////////////////////////////////
bool UPnpControl::getExternalIp(UtlString& externalIp,
                                int maxRequestTime) const
{
    UtlString envelope = createGetExternalIpSoapEnvelope();
    HttpMessage request = createGetExternalIpRequest(envelope.data(),
                                                     envelope.length());

    HttpMessage response;
    nErrorCode = makeRequest(request, response, maxRequestTime);
    if (200 == nErrorCode)
    {
        externalIp = parseGetExternalIp(response);
        OsSysLog::add(FAC_NAT, PRI_INFO,
            "UPnpControl::getExternalIp - success: %s.\n", externalIp);
    }
    else
    {
        OsSysLog::add(FAC_NAT, PRI_INFO,
            "UPnpControl::getExternalIp - failure.\n");
    }
    return (200 == nErrorCode);
}

//////////////////////////////////////////////////////////////////////////////
bool UPnpControl::deletePortMapping(const int externalPort,
                                    const int maxRequestTime,
                                    const char* protocol
                             ) const
{
    bool bRet = false;
    
    UtlString envelope = createDeletePortMappingSoapEnvelope(externalPort,
                                                       protocol);
    HttpMessage request = createDeletePortMappingRequest(envelope.data(),
                                                   envelope.length());

    HttpMessage response;
    nErrorCode = makeRequest(request, response, maxRequestTime);
    if (200 == nErrorCode)
    {
        bRet = true;
        OsSysLog::add(FAC_NAT, PRI_INFO,
            "UPnpControl::deletePortMapping - %d : success.\n", externalPort);
    }
    else
    {
        OsSysLog::add(FAC_NAT, PRI_INFO,
            "UPnpControl::deletePortMapping - %d : failure.\n", externalPort);
    }
    return bRet;
}

//////////////////////////////////////////////////////////////////////////////
bool UPnpControl::getPortMappingEntry(const int externalPort,
                                            PortMapping* const portMapping,
                                            const int maxRequestTime,
                                            const char* protocol) const
{
    bool bExists = false;
    UtlString envelope = createGetPortMappingSoapEnvelope(externalPort,
                                                       protocol);
    HttpMessage request = createGetPortMappingRequest(envelope.data(),
                                                   envelope.length());

    HttpMessage response;
    nErrorCode = makeRequest(request, response, maxRequestTime);
    if (200 == nErrorCode)
    {
        if (portMapping)
        {
            portMapping->setExternalPort(externalPort);
        }
        if (true == parseGetPortMappingResponse(portMapping, response))
        {
            bExists = true;
        }
    }
    else
    {
        OsSysLog::add(FAC_NAT, PRI_INFO,
            "UPnpControl::getPortMappingEntry - failure.\n");
    }    
    return bExists;
}

//////////////////////////////////////////////////////////////////////////////
const HttpMessage UPnpControl::createGenericRequest(
    const char* content,
    const int contentLength) const
{
/*
POST /wipconn HTTP/1.1
Content-Type: text/xml; charset="utf-8"
User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows 9x)
Content-Length: 327
Connection: close
Pragma: no-cache
*/
    HttpMessage request;
    request.setFirstHeaderLine(HTTP_POST_METHOD, mControlUrl.toString().data(), HTTP_PROTOCOL_VERSION_1_1);
    request.setBody(new HttpBody(content, contentLength));
    request.setContentLength(contentLength);
    request.addHeaderField("Content-Type",  "text/xml; charset=\"utf-8\"");
    request.addHeaderField("Connection", "close");
    request.addHeaderField("Pragma", "no-cache");

    UtlString sPath;
    mControlUrl.getPath(sPath);
    request.changeRequestUri(sPath.data());

    return request;
}

//////////////////////////////////////////////////////////////////////////////
const UtlString UPnpControl::createAddPortMappingSoapEnvelope(
                    const int externalPort,
                    const int internalPort,
                    const char* szClientAddress,
                    const char* descriptionBase,
                    const char* protocol,
                    const bool enabled) const
{
    UtlString xml;
    char szExternalPort[32];
    char szInternalPort[32];
    char szEnabled[2];
    char szDescription[256];

    snprintf(szExternalPort, sizeof(szExternalPort), "%d", externalPort);
    snprintf(szInternalPort, sizeof(szInternalPort), "%d", internalPort);
    snprintf(szDescription, sizeof(szDescription), "%s%d", descriptionBase, externalPort);

    if (enabled)
    {
        strncpy(szEnabled, "1", sizeof(szEnabled));
    }
    else
    {
        strncpy(szEnabled, "0", sizeof(szEnabled));
    }

    xml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    xml.append("<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">");
    xml.append("<SOAP-ENV:Body>");
    xml.append("<m:AddPortMapping xmlns:m=\"urn:schemas-upnp-org:service:WANIPConnection:1\">");
    xml.append("<NewRemoteHost xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"string\"/>");
    xml.append("<NewExternalPort xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"ui2\">" + UtlString(szExternalPort) + "</NewExternalPort>");
    xml.append("<NewProtocol xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"string\">" + UtlString(protocol) + "</NewProtocol>");
    xml.append("<NewInternalPort xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"ui2\">" + UtlString(szInternalPort) + "</NewInternalPort>");
    xml.append("<NewInternalClient xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"string\">" + UtlString(szClientAddress) + "</NewInternalClient>");
    xml.append("<NewEnabled xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"boolean\">" + UtlString(szEnabled) + "</NewEnabled>");
    xml.append("<NewPortMappingDescription xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"string\">" + UtlString(szDescription) + "</NewPortMappingDescription>");
    xml.append("<NewLeaseDuration xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"ui4\">0</NewLeaseDuration>");
    xml.append("</m:AddPortMapping>");
    xml.append("</SOAP-ENV:Body>");
    xml.append("</SOAP-ENV:Envelope>");

    return xml;
}

//////////////////////////////////////////////////////////////////////////////
const HttpMessage UPnpControl::createAddPortMappingRequest(const char* content,
                                                        const int contentLength) const
{
/*
POST /wipconn HTTP/1.1
Content-Type: text/xml; charset="utf-8"
SOAPAction: "urn:schemas-upnp-org:service:WANIPConnection:1#AddPortMapping"
User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows 9x)
Host: 10.1.10.1:4444
Content-Length: 1117
Connection: close
Pragma: no-cache
*/
    HttpMessage request = createGenericRequest(content, contentLength);
    request.addHeaderField("SOAPAction", "\"urn:schemas-upnp-org:service:WANIPConnection:1#AddPortMapping\"");

    return request;
}

//////////////////////////////////////////////////////////////////////////////
const UtlString UPnpControl::createGetPortMappingSoapEnvelope(
    const int externalPort,
    const char* protocol) const
{
    char szExternalPort[32];
    snprintf(szExternalPort, sizeof(szExternalPort), "%d", externalPort);

    UtlString xml;
    xml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    xml.append("<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">");
    xml.append("<SOAP-ENV:Body>");
    xml.append("<m:GetSpecificPortMappingEntry xmlns:m=\"urn:schemas-upnp-org:service:WANIPConnection:1\">");
    xml.append("<NewRemoteHost xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"string\"/>");
    xml.append("<NewExternalPort xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"ui2\">" + UtlString(szExternalPort) + "</NewExternalPort>");
    xml.append("<NewProtocol xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"string\">" + UtlString(protocol) + "</NewProtocol>");
    xml.append("</m:GetSpecificPortMappingEntry>");
    xml.append("</SOAP-ENV:Body>");
    xml.append("</SOAP-ENV:Envelope>");
    return xml;
}

//////////////////////////////////////////////////////////////////////////////
const HttpMessage UPnpControl::createGetPortMappingRequest(
    const char* content,
    const int contentLength) const
{
/*
POST /wipconn HTTP/1.1
Content-Type: text/xml; charset="utf-8"
SOAPAction: "urn:schemas-upnp-org:service:WANIPConnection:1#GetSpecificPortMappingEntry"
User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows 9x)
Host: 10.1.10.1:4444
Content-Length: 624
Connection: close
Pragma: no-cache
*/
    HttpMessage request = createGenericRequest(content, contentLength);
    request.addHeaderField("SOAPAction", "\"urn:schemas-upnp-org:service:WANIPConnection:1#GetSpecificPortMappingEntry\"");

    return request;
}

//////////////////////////////////////////////////////////////////////////////
const UtlString UPnpControl::createGetExternalIpSoapEnvelope() const
{
    UtlString xml;

    xml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    xml.append("<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">");
    xml.append("<SOAP-ENV:Body>");
    xml.append("<m:GetExternalIPAddress xmlns:m=\"urn:schemas-upnp-org:service:WANIPConnection:1\"/>");
    xml.append("</SOAP-ENV:Body>");
    xml.append("</SOAP-ENV:Envelope>");

    return xml;
}

//////////////////////////////////////////////////////////////////////////////
const HttpMessage UPnpControl::createGetExternalIpRequest(
    const char* content,
    const int contentLength) const
{
/*
POST /wipconn HTTP/1.1
Content-Type: text/xml; charset="utf-8"
SOAPAction: "urn:schemas-upnp-org:service:WANIPConnection:1#GetExternalIPAddress"
User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows 9x)
Host: 10.1.10.1:4444
Content-Length: 327
Connection: close
Pragma: no-cache
*/
    HttpMessage request = createGenericRequest(content, contentLength);
    request.addHeaderField("SOAPAction", "\"urn:schemas-upnp-org:service:WANIPConnection:1#GetExternalIPAddress\"");

    return request;
}

//////////////////////////////////////////////////////////////////////////////
const UtlString UPnpControl::createRequestConnectionSoapEnvelope() const
{
    UtlString xml;

    xml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    xml.append("<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">");
    xml.append("<SOAP-ENV:Body>");
    xml.append("<m:RequestConnection xmlns:m=\"urn:schemas-upnp-org:service:WANIPConnection:1\"/>");
    xml.append("</SOAP-ENV:Body>");
    xml.append("</SOAP-ENV:Envelope>");

    return xml;
}

//////////////////////////////////////////////////////////////////////////////
const HttpMessage UPnpControl::createRequestConnectionRequest(
    const char* content,
    const int contentLength) const
{
    HttpMessage request = createGenericRequest(content, contentLength);
    request.addHeaderField("SOAPAction", "\"urn:schemas-upnp-org:service:WANIPConnection:1#RequestConnection\"");

    return request;
}


//////////////////////////////////////////////////////////////////////////////
const UtlString UPnpControl::createDeletePortMappingSoapEnvelope(
                    const int externalPort,
                    const char* protocol) const
{
    UtlString xml;
    char szExternalPort[32];

    snprintf(szExternalPort, sizeof(szExternalPort), "%d", externalPort);

    xml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    xml.append("<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">");
    xml.append("<SOAP-ENV:Body>");
    xml.append("<m:DeletePortMapping xmlns:m=\"urn:schemas-upnp-org:service:WANIPConnection:1\">");
    xml.append("<NewRemoteHost xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"string\"/>");
    xml.append("<NewExternalPort xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"ui2\">" + UtlString(szExternalPort) + "</NewExternalPort>");
    xml.append("<NewProtocol xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"string\">" + UtlString(protocol) + "</NewProtocol>");
    xml.append("</m:DeletePortMapping>");
    xml.append("</SOAP-ENV:Body>");
    xml.append("</SOAP-ENV:Envelope>");

    return xml;
}

//////////////////////////////////////////////////////////////////////////////
const HttpMessage UPnpControl::createDeletePortMappingRequest(
    const char* content,
    const int contentLength) const
{
/*
POST /wipconn HTTP/1.1
Content-Type: text/xml; charset="utf-8"
SOAPAction: "urn:schemas-upnp-org:service:WANIPConnection:1#GetExternalIPAddress"
User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows 9x)
Host: 10.1.10.1:4444
Content-Length: 327
Connection: close
Pragma: no-cache
*/
    HttpMessage request = createGenericRequest(content, contentLength);
    request.addHeaderField("SOAPAction", "\"urn:schemas-upnp-org:service:WANIPConnection:1#DeletePortMappping\"");

    return request;
}

const int UPnpControl::makeRequest(HttpMessage& request,
                                   HttpMessage& response,
                                   const int maxRequestTime) const
{
    int timeoutSecondsToUse = maxRequestTime;
    if (timeoutSecondsToUse == 0)
        timeoutSecondsToUse = 4;
    nErrorCode = response.get((Url&)mControlUrl, (HttpMessage&)request, timeoutSecondsToUse * 1000);
    if (nErrorCode != 200)
    {
        response.getFirstHeaderLinePart(2, &sErrorText);
        if (response.getBody())
        {
            int len;
            response.getBody()->getBytes(&sErrorDetail, &len);
        }
    }
    else
    {
        sErrorText = "";
        sErrorDetail = "";
    }
    return nErrorCode;
}
//////////////////////////////////////////////////////////////////////////////
const UtlString UPnpControl::parseGetExternalIp(const HttpMessage response) const
{
    UtlString externalIp;
    UtlString xml;
    int length;
    if (response.getBody())
    {
        response.getBody()->getBytes(&xml, &length);

        int docBegin = xml.first("<?xml");
        if (docBegin > 0)
            xml = UtlString(&xml.data()[docBegin]);
        TiXmlDocument doc;
          
        doc.Parse(xml.data());
        TiXmlNode * curNode = doc.FirstChildElement();
        if (curNode)
        {
            curNode = UPnpXmlNavigator::getFirstDescendant(curNode,
                                                        "NewExternalIPAddress",
                                                            NULL);
            if (curNode)
            {
                curNode = curNode->FirstChild();
                if (curNode)
                {
                    externalIp = curNode->Value();
                }
            }
        }
    }
    return externalIp;
}

bool UPnpControl::parseGetPortMappingResponse(PortMapping* const portMapping,
                                        const HttpMessage response) const
{
    bool bRet = false;
    UtlString xml;
    int length;
    response.getBody()->getBytes(&xml, &length);
    int docBegin = xml.first("<?xml");
    if (docBegin > 0)
        xml = UtlString(&xml.data()[docBegin]);

    TiXmlDocument doc;
      
    doc.Parse(xml.data());
    TiXmlNode* curNode = UPnpXmlNavigator::getFirstDescendant(doc.FirstChildElement(),
                                                              "m:GetSpecificPortMappingEntryResponse",
                                                              NULL);
    if (curNode == NULL) // try "u:"
    {
        curNode = UPnpXmlNavigator::getFirstDescendant(doc.FirstChildElement(),
                                                              "u:GetSpecificPortMappingEntryResponse",
                                                              NULL);
    }
    if (curNode)
    {
        TiXmlNode* childElem = curNode->FirstChildElement();
        while (childElem)
        {
            bRet = true;
            const char* elemName = childElem->Value();
            if (strcasecmp(elemName, "NewInternalPort") == 0)
            {
                TiXmlNode* textChild = childElem->FirstChild();
                if (textChild)
                {
                    portMapping->setInternalPort(atoi(textChild->Value()));
                }
            }
            else if (strcasecmp(elemName, "NewInternalClient") == 0)
            {
                TiXmlNode* textChild = childElem->FirstChild();
                if (textChild)
                {
                    portMapping->setClientIp(textChild->Value());
                }
            }
            childElem = childElem->NextSiblingElement();
        }
    }
    return bRet;
}

