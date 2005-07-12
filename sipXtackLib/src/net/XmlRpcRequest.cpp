// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <os/OsDateTime.h>
#include <utl/UtlInt.h>
#include <utl/UtlBool.h>
#include <utl/UtlDateTime.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMapIterator.h>
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

   result = addValue(value);
   
   mpRequestBody->append(END_PARAM);
        
   return result;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
bool XmlRpcRequest::addValue(UtlContainable* value)
{
   bool result = false;

   UtlString paramType(value->getContainableType());
   UtlString paramValue; 

   // UtlInt
   char temp[10];
   if (paramType.compareTo("UtlInt") == 0)
   {
      UtlInt* pValue = (UtlInt *)value;
      sprintf(temp, "%d", pValue->getValue());
      paramValue = BEGIN_INT + UtlString(temp) + END_INT;
      result = true;
   }
   else
   {
      // UtlBool
      if (paramType.compareTo("UtlBool") == 0)
      {
         UtlBool* pValue = (UtlBool *)value;
         if (pValue->getValue())
         {
            sprintf(temp, "1");
         }
         else
         {
            sprintf(temp, "0");
         }
               
         paramValue = BEGIN_BOOLEAN + UtlString(temp) + END_BOOLEAN;
         result = true;
      }
      else
      {
         // UtlString
         if (paramType.compareTo("UtlString") == 0)
         {
            UtlString* pValue = (UtlString *)value;
            paramValue = BEGIN_STRING + *pValue + END_STRING;
            result = true;
         }
         else
         {
            // UtlDateTime
            if (paramType.compareTo("UtlDateTime") == 0)
            {
               UtlDateTime* pTime = (UtlDateTime *)value;
               OsDateTime time;
               pTime->getTime(time);
               UtlString isoTime;
               time.getIsoTimeStringZ(isoTime);               
               paramValue = BEGIN_TIME + isoTime + END_TIME;
               result = true;
            }
            else
            {
               // UtlHashMap
               if (paramType.compareTo("UtlHashMap") == 0)
               {
                  result = addStruct((UtlHashMap *)value);
               }
               else
               {
                  // UtlSList
                  if (paramType.compareTo("UtlSList") == 0)
                  {
                     result = addArray((UtlSList *)value);
                  }
                  else
                  {
                     OsSysLog::add(FAC_SIP, PRI_WARNING,
                                   "XmlRpcRequest::addValue unspported type = %s\n", paramType.data());                     
                  }                     
               }
            }
         }
      }
   }
            
   mpRequestBody->append(paramValue);
   return result;
}


bool XmlRpcRequest::addArray(UtlSList* array)
{
   bool result = false;
   mpRequestBody->append(BEGIN_ARRAY);
   
   UtlSListIterator iterator(*array);
   UtlContainable* pObject;
   while (pObject = iterator())
   {
      result = addValue(pObject);
      if (!result)
      {
         break;
      }
   }
   
   mpRequestBody->append(END_ARRAY);
   return result;
}

bool XmlRpcRequest::addStruct(UtlHashMap* members)
{
   bool result = false;
   mpRequestBody->append(BEGIN_STRUCT);
   
   UtlHashMapIterator iterator(*members);
   UtlString* pName;
   UtlContainable* pObject;
   UtlString structName;
   while (pName = (UtlString *)iterator())
   {
      mpRequestBody->append(BEGIN_MEMBER);

      structName = BEGIN_NAME + *pName + END_NAME;
      mpRequestBody->append(structName); 
      
      pObject = members->findValue(pName);
      result = addValue(pObject);
      if (!result)
      {
         break;
      }
      
      mpRequestBody->append(END_MEMBER);
   }
   
   mpRequestBody->append(END_STRUCT);
   return result;
}



/* ============================ FUNCTIONS ================================= */

