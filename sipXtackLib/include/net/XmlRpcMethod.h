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

#ifndef _XMLRPCMETHOD_H_
#define _XMLRPCMETHOD_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/UtlHashMap.h>
#include <net/HttpRequestContext.h>
#include "net/XmlRpcResponse.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * A XmlRpcMethod is a dynamically loaded object that is invoked by the XmlRpcDispatch
 * during the runtime.
 *
 * This class is the abstract base from which all XML-RPC methods must inherit. Four
 * methods must be implemented by the subclasses:
 * 
 * - get() is for XmlRpcDispatch to instantiate the subclass during the runtime
 * 
 * - addParam() is for XmlRpcDispatch to pass the atomic param from incmoing
 * XmlRpcRequest to service.
 * 
 * All the param types must be UtlContainable. Here is the mapping between XML-RPC
 * types to UtlContainable types:
 * 
 * <i4> or <int> is UtlInt.
 * <boolean> is UtlBool.
 * <string> is UtlString.
 * <dateTime.iso8601> is UtlDateTime.
 * <array> is UtlSList.
 * <struct> is UtlHashMap.
 * 
 * <double> and <base64> are currently not supported.
 * 
 * - execute() is for XmlRpcDispatch to execute the XML-RPC request and send back
 * the XmlRpcResponse to the client side
 *
 */

class XmlRpcMethod
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum ExecutionStatus
   {
      OK,
      FAILED,
      REQUIRE_AUTHENTICATION
   };

/* ============================ CREATORS ================================== */

   typedef XmlRpcMethod* Get();
   
   /// Get the instance of this method. Subclasses must provide a definition for this method.
   static XmlRpcMethod* get();
   
   /// Destructor
   virtual ~XmlRpcMethod();

   /// Add a param. Subclasses must provide a definition for this method.
   virtual bool addParam(int index, ///< index position of the param in the request
                         UtlContainable* value, ///< value for the param
                         XmlRpcResponse& faultResponse ///< fault response if return is false
                         ) = 0;

   /// Execute the method. Subclasses must provide a definition for this method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        XmlRpcResponse& response, ///< request response
                        ExecutionStatus status) = 0; ///< execution status

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// Constructor
   XmlRpcMethod();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Disabled copy constructor
   XmlRpcMethod(const XmlRpcMethod& rXmlRpcMethod);

   /// Disabled assignment operator
   XmlRpcMethod& operator=(const XmlRpcMethod& rhs);
   
};

/* ============================ INLINE METHODS ============================ */

#endif  // _XMLRPCMETHOD_H_


