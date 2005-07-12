// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _XMLRPCDISPATCH_H_
#define _XMLRPCDISPATCH_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsBSem.h>
#include <xmlparser/tinyxml.h>
#include "net/HttpService.h"
#include "net/HttpServer.h"
#include "net/XmlRpcMethod.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

/**
 * A XmlRpcDispatch is a object that monitors the incoming
 * XML-RPC requests, parses XmlRpcRequest messages, invokes the correct
 * XmlRpcMethod calls, and sends back the corresponding XmlRpcResponse responses.
 * If the correspnding method does not exit, it will send back a 404 response.
 * Otherwise, it will always send back a 200 OK response with XmlRpcResponse
 * content.
 * 
 * For each XML-RPC server, it needs to instantiate a XmlRpcDispatch object first,
 * and then register each service method using addMethod() or remove the method
 * using removeMethod().
 */

class XmlRpcDispatch : public HttpService
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Create a dispatch object.
   XmlRpcDispatch(int httpServerPort,           ///< port number for HttpServer
                  bool isSecureServer,          ///< option for HTTP or HTTPS
                  const char* uriPath = "/RPC2" ///< uri path
                  ); 

   /// Destructor.
   virtual ~XmlRpcDispatch();

/* ============================ MANIPULATORS ============================== */

   /// Handler for XML-RPC requests
   void processRequest(const HttpRequestContext& requestContext,
                       const HttpMessage& request,
                       HttpMessage*& response );

/* ============================ ACCESSORS ================================= */

   /// Add a method to the RPC dispatch
   void addMethod(const char* methodName, XmlRpcMethod::Get* method);

   /// Remove a method from the RPC dispatch by name
   void removeMethod(const char* methodName);
   
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   friend class XmlRpcTest;
   
   /// Parse the XML-RPC request
   bool parseXmlRpcRequest(UtlString& requestContent, XmlRpcMethod*& method, XmlRpcResponse& response);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Parse a value in the XML-RPC request
   bool parseValue(XmlRpcMethod*& method, TiXmlNode* valueNode, int index, XmlRpcResponse& response);

   /// Parse an array in the XML-RPC request
   bool parseArray(TiXmlNode* valueNode, UtlSList*& array);

   /// Parse a struct in the XML-RPC request
   bool parseStruct(TiXmlNode* valueNode, UtlHashMap*& memebers);

   /// Http server for handling the HTTP POST request  
   HttpServer* mpHttpServer;
   
   /// hash map for holding all registered XML-RPC methods
   UtlHashMap  mMethods;
   
   /// reader/writer lock for synchronization
   OsBSem mLock;

   /// Disabled copy constructor
   XmlRpcDispatch(const XmlRpcDispatch& rXmlRpcDispatch);

   /// Disabled assignment operator
   XmlRpcDispatch& operator=(const XmlRpcDispatch& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _XMLRPCDISPATCH_H_


