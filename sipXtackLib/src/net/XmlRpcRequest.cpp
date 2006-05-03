//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include "net/XmlRpcRequest.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
XmlRpcRequest::XmlRpcRequest(Url& uri, const char* methodName)
{
   mUrl = uri;
   
   // Start to contruct the HTTP message
   mpHttpRequest = new HttpMessage();
   
   mpHttpRequest->setFirstHeaderLine(HTTP_POST_METHOD, "/RPC2", HTTP_PROTOCOL_VERSION_1_1);
   mpHttpRequest->addHeaderField("Accept", "text/xml");
   mpHttpRequest->setUserAgentField("XML-RPC client");
   
   // Start to construct the XML-RPC body
   mpRequestBody = new XmlRpcBody();
   mpRequestBody->append(BEGIN_METHOD_CALL);
   
   UtlString methodCall = BEGIN_METHOD_NAME + UtlString(methodName) + END_METHOD_NAME;
   mpRequestBody->append(methodCall);
   
   mpRequestBody->append(BEGIN_PARAMS);   
}

// Copy constructor
XmlRpcRequest::XmlRpcRequest(const XmlRpcRequest& rXmlRpcRequest)
{
}

// Destructor
XmlRpcRequest::~XmlRpcRequest()
{
   if (mpHttpRequest)
   {
      delete mpHttpRequest;
   }
}


bool XmlRpcRequest::execute(XmlRpcResponse& response)
{
   bool result = false;
   
   // End of constructing the XML-RPC body
   mpRequestBody->append(END_PARAMS);   
   mpRequestBody->append(END_METHOD_CALL);
   
   UtlString bodyString;
   int bodyLength;
   mpRequestBody->getBytes(&bodyString, &bodyLength);
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "XmlRpcRequest::execute XML-RPC request message = \n%s\n", bodyString.data());
   
   mpHttpRequest->setBody(mpRequestBody);
   mpHttpRequest->setContentType(CONTENT_TYPE_TEXT_XML);
   mpHttpRequest->setContentLength(bodyLength);

   // Create an empty response object and sent the built up request
   // to the XML-RPC server
   HttpMessage *pResponse =
      new HttpMessage(static_cast< const HttpMessage& >(*mpHttpRequest));

   pResponse->get(mUrl, *mpHttpRequest, XML_RPC_TIMEOUT);

   UtlString status;
   
   pResponse->getResponseStatusText(&status);
      
   if (status.compareTo("OK") == 0)
   {
      const HttpBody* pResponseBody = pResponse->getBody();
      pResponseBody->getBytes(&bodyString, &bodyLength);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "XmlRpcRequest::execute XML-RPC message = %s\n", bodyString.data());
         
      if (response.parseXmlRpcResponse(bodyString))
      {
         result = true;
      }
      else
      {
         result = false;
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "XmlRpcRequest::execute failed with status = %s\n", status.data());
   }

   delete pResponse;
   
   return result;
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
XmlRpcRequest&
XmlRpcRequest::operator=(const XmlRpcRequest& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
}

/* ============================ ACCESSORS ================================= */

bool XmlRpcRequest::addParam(UtlContainable* value)
{
   bool result = false;
   mpRequestBody->append(BEGIN_PARAM);  

   result = mpRequestBody->addValue(value);
   
   mpRequestBody->append(END_PARAM);
        
   return result;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

