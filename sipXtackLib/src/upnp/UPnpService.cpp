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
#include "os/OsSysLog.h"
#include "upnp/UPnpService.h"
#include "net/HttpMessage.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* UPnpService::WANIPConnection = "urn:schemas-upnp-org:service:WANIPConnection:1";
// MACROS

//////////////////////////////////////////////////////////////////////////////
UPnpService::UPnpService()
{
}

//////////////////////////////////////////////////////////////////////////////
UPnpService::UPnpService(const UtlString xml,
                         const UtlString serviceType) :
    mServiceType(serviceType)
{
    parseXml(xml);
}

UPnpService::UPnpService(const Url rootXmlUrl,
                         const UtlString serviceType) :
    mServiceType(serviceType)
{
    HttpMessage document;
    Url nonConstUrl_butWhy(rootXmlUrl);

    int status_code = document.get(nonConstUrl_butWhy, 10000, false, false);

    const HttpBody* pBody = document.getBody();
    UtlString actualBytes;
    int len = 0;
    pBody->getBytes(&actualBytes, &len);
    if (pBody == NULL)
    {
        // TODO: error reporting
    }
    else
    {
        parseXml(actualBytes);
    }
}

//////////////////////////////////////////////////////////////////////////////
UPnpService::UPnpService(const UPnpService& src)
{
    this->mControlUrl = src.mControlUrl;
    this->mEventSubUrl = src.mEventSubUrl;
    this->mScpdUrl = src.mScpdUrl;
    this->mServiceId = src.mServiceId;
    this->mServiceType = src.mServiceType;
}

//////////////////////////////////////////////////////////////////////////////
UPnpService::~UPnpService()
{
}

//////////////////////////////////////////////////////////////////////////////
UPnpService& UPnpService::operator= (const UPnpService& src)
{
    if (&src == this)
    {
        return *this;
    }
    this->mControlUrl = src.mControlUrl;
    this->mEventSubUrl = src.mEventSubUrl;
    this->mScpdUrl = src.mScpdUrl;
    this->mServiceId = src.mServiceId;
    this->mServiceType = src.mServiceType;
    return *this;
}

//////////////////////////////////////////////////////////////////////////////
const UtlString UPnpService::makeUrl(const char* path, const char* base) const
{
    UtlString url;
    if (UtlString(path).contains("://"))
    {
        return UtlString(path);
    }
    if (path && strlen(path) > 0)
    {
        if (base  && strlen(base) > 0)
        {
            url = UtlString(base) + UtlString(path);
        }
        else
        {
            url = UtlString(path);
        }
    }
    else
    {
        if (base  && strlen(base) > 0)
        {
            url = UtlString(base);
        }
    }
    return url;
}

//////////////////////////////////////////////////////////////////////////////
bool UPnpService::parseXml(const UtlString& xml)
{
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "UPnpService::parseXml begin parse%s\n", 
                    xml.data());

      TiXmlDocument doc;
      
      doc.Parse(xml.data());
      UtlString urlBase;
      if (!doc.Error())
      {
         TiXmlNode* curNode = doc.FirstChildElement();

         TiXmlNode* urlBaseNode = curNode->FirstChildElement("URLBase");
         if (urlBaseNode)
         {
             urlBaseNode = urlBaseNode->FirstChild();
             if (urlBaseNode)
                urlBase = urlBaseNode->Value();
         }

         curNode = UPnpXmlNavigator::getFirstDescendant(curNode, "serviceType", mServiceType.data());
         if (curNode)
         {
             curNode = curNode->NextSibling();
             while (curNode)
             {
                 const char* elemName = curNode->Value();

                 /*  
                     for example: 
                <serviceId>urn:upnp-org:serviceId:WANIPConn1</serviceId>
                <controlURL>http://10.1.10.1:4444/wipconn</controlURL>
                <eventSubURL>http://10.1.10.1:9393/wipconn</eventSubURL>
                <SCPDURL>http://10.1.10.1/get/WANIPConn1.xml</SCPDURL>
                */
                 if (strcmp(elemName, "serviceId") == 0)
                 {
                     if (curNode->FirstChild())
                        mServiceId = curNode->FirstChild()->Value();
                 }
                 else if (strcmp(elemName, "controlURL") == 0)
                 {
                     if (curNode->FirstChild())
                        mControlUrl = makeUrl(curNode->FirstChild()->Value(), urlBase.data());
                 }
                 else if (strcmp(elemName, "eventSubURL") == 0)
                 {
                     if (curNode->FirstChild())
                        mEventSubUrl = makeUrl(curNode->FirstChild()->Value(), urlBase.data());
                 }
                 else if (strcmp(elemName, "SCPDURL") == 0)
                 {
                     if (curNode->FirstChild())
                        mScpdUrl = makeUrl(curNode->FirstChild()->Value(), urlBase.data());
                 }
                 curNode = curNode->NextSibling();
             }
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogEvent::parseBody xml parsing error");
      }

      return true;
}

TiXmlNode* UPnpXmlNavigator::getFirstDescendant(TiXmlNode* root, const char* elemName, const char* text)
{
    bool bFound = false;
    TiXmlNode* curNode = root;
    while (curNode)
    {
        if (strcmp(curNode->Value(), elemName) == 0)
        {
            if (text != NULL)
            {
                if (strcmp(curNode->FirstChild()->Value(), text) == 0)
                {
                    bFound = true;
                    break;
                }
            }
            else
            {
                bFound = true;
                break;
            }
        }
        if (curNode->FirstChildElement())
        {
            curNode = curNode->FirstChildElement();
        }
        else
        {
            TiXmlNode* next = curNode->NextSiblingElement();
            if (next != NULL)
            {
                    curNode = next;
            }
            else
            {
                    TiXmlNode* parent = curNode->Parent();
                    if (parent == NULL)
                    {
                        break;
                    }
                    TiXmlNode* parentNext = parent->NextSiblingElement();
                    while (parentNext == NULL)
                    {
                        parent = parent->Parent();
                        if (parent == NULL)
                        {
                            break;
                        }
                        parentNext = parent->NextSiblingElement();
                    }
                    curNode = parentNext;
            }
        }
    }
    if (!bFound)
    {
        curNode = NULL;
    }
    return curNode;
}

//////////////////////////////////////////////////////////////////////////////

