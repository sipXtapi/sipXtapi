//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _XMLRPCRESPPONSE_H_
#define _XMLRPCRESPPONSE_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <xmlparser/tinyxml.h>
#include "net/Url.h"
#include "net/XmlRpcBody.h"

// DEFINES
#define ILL_FORMED_CONTENTS_FAULT_CODE -1
#define ILL_FORMED_CONTENTS_FAULT_STRING "Ill-formed XML-RPC contents"

#define METHOD_NAME_FAULT_CODE -2
#define METHOD_NAME_FAULT_STRING "Method name is missing"

#define UNREGISTERED_METHOD_FAULT_CODE -3
#define UNREGISTERED_METHOD_FAULT_STRING "Method has not been registered"

#define AUTHENTICATION_REQUIRED_FAULT_CODE -4
#define AUTHENTICATION_REQUIRED_FAULT_STRING "Authentication is required"

#define EMPTY_PARAM_VALUE_FAULT_CODE -5
#define EMPTY_PARAM_VALUE_FAULT_STRING "Empty param value"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

/**
 * This object is used to create a XML-RPC response to a XmlRpcRequest request.
 * setResponse() is used for creating the response, and getResponse() is
 * used for getting the value from the request. Furthermore, setFault() is for
 * creating a fault response, and getFault() is used for getting the fault code
 * and fault string in the fault response.
 * 
 */

class XmlRpcResponse
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Contruct a XML-RPC response
   XmlRpcResponse();

   /// Destructor
   virtual ~XmlRpcResponse();

/* ============================ MANIPULATORS ============================== */

   /// Set the XML-RPC response
   bool setResponse(UtlContainable* value); ///< value for the response

   /// Set the fault code and fault string in a fault response
   bool setFault(int faultCode, const char* faultString);
   /**<
    * This function will create a fault response
    * 
    */

   /// Get the XML-RPC response
   bool getResponse(UtlContainable*& value); ///< value for the param

   /// Get the fault code and fault string from the XML-RPC response
   void getFault(int* faultCode, UtlString& faultString);

   /// Parse the XML-RPC response
   bool parseXmlRpcResponse(UtlString& responseContent); ///< response content from XML-RPC request
   
   /// Get the content of the response
   XmlRpcBody* getBody() { return mpResponseBody; };
         
/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Parse a value in the XML-RPC response
   bool parseValue(TiXmlNode* valueNode);

   /// Parse an array in the XML-RPC response
   bool parseArray(TiXmlNode* valueNode, UtlSList* array);

   /// Parse a struct in the XML-RPC response
   bool parseStruct(TiXmlNode* valueNode, UtlHashMap* memebers);

   // Clean up the memory in a UtlContainable
   void cleanUp(UtlContainable* value);
   
   /// Clean up the memory in a struct
   void cleanUp(UtlHashMap* members);
   
   /// Clean up the memory in an array
   void cleanUp(UtlSList* array);
   
   /// XML-RPC body
   XmlRpcBody* mpResponseBody;

   /// Value for the XML-RPC response
   UtlContainable* mResponseValue;
   
   /// Fault code
   int mFaultCode;
   
   /// Fault string
   UtlString mFaultString;
     
   /// Disabled copy constructor
   XmlRpcResponse(const XmlRpcResponse& rXmlRpcResponse);

   /// Disabled assignment operator
   XmlRpcResponse& operator=(const XmlRpcResponse& rhs);   
};

/* ============================ INLINE METHODS ============================ */

#endif  // _XMLRPCRESPPONSE_H_


