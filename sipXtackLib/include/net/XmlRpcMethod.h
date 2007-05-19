//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

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
 * This class is the abstract base from which all XML-RPC methods must inherit. Two
 * methods must be implemented by the subclasses:
 * 
 * - get() is for XmlRpcDispatch to instantiate the subclass during the runtime. It
 * is not a singleton class. The XmlRpcMethod derived objects will be deleted after
 * each use in XmlRpcDispatch.
 * 
 * - execute() is for XmlRpcDispatch to execute the XML-RPC request and send back
 * the XmlRpcResponse to the client side.
 * 
 * All the params in the XML-RPC request are stored in a UtlSList and passed to
 * the service in execute(). All the param values are stored in UtlContainable
 * types. The mapping between XML-RPC value types and UtlContainable types are:
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

   /// Execute the method. Subclasses must provide a definition for this method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params, ///< request param list
                        void* userData, ///< user data
                        XmlRpcResponse& response, ///< request response
                        ExecutionStatus& status) = 0; ///< execution status

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


