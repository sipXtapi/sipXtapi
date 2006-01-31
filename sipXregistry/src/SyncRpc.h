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
#ifndef _SYNCRPC_H_
#define _SYNCRPC_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/XmlRpcMethod.h"
#include "RegistrarPeer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;
class XmlRpcDispatch;

/// Base class for all registerSync XML-RPC Methods
class SyncRpcMethod : public XmlRpcMethod
{
public:
   static XmlRpcMethod* get();

   /// destructor
   virtual ~SyncRpcMethod() 
      {
      }
   
   /// Get the name of the XML-RPC method.
   virtual const char* name() = 0;

   /// Register this method with the XmlRpcDispatch object so it can be called.
   static void registerSelf(SipRegistrar&   registrar);

protected:
   // Method name
   static const char* METHOD_NAME;
   
   /// constructor 
   SyncRpcMethod();

   /// Common method for registration with the XML-RPC dispatcher
   static void registerMethod(const char*        methodName,
                              XmlRpcMethod::Get  getMethod,
                              SipRegistrar&      registrar
                              );

   /// The execute method called by XmlRpcDispatch
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        ) = 0;

   /// Common method to do peer authentication
   ExecutionStatus authenticateCaller(
      const HttpRequestContext& requestContext, ///< request context
      const UtlString&          peerName,       ///< name of the peer who is calling
      XmlRpcResponse&           response,       ///< response to put fault in
      SipRegistrar&             registrar,      ///< registrar
      RegistrarPeer**           peer = NULL     ///< optional output arg: look up peer by name
                                      );

private:
   /// no copy constructor
   SyncRpcMethod(const SyncRpcMethod& nocopy);

   /// no assignment operator
   SyncRpcMethod& operator=(const SyncRpcMethod& noassignment);
};


/// the registerSync.reset XML-RPC method.
class SyncRpcReset : public SyncRpcMethod
{
  public:

   static XmlRpcMethod* get();

   /// destructor
   virtual ~SyncRpcReset() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipRegistrar&   registrar);

   /// Reset the SynchronizationState and update numbers with respect to some peer.
   static RegistrarPeer::SynchronizationState
      invoke( const char*    myName   ///< primary name of this registrar
             ,RegistrarPeer& peer     ///< the peer to invoke reset on
             );
   /**<
    * On success,
    *   - updates the sent update number for the peer
    *   - marks the peer as Reachable
    *
    * On any failure, the peer is marked UnReachable.
    */

  protected:
   /// constructor 
   SyncRpcReset();

   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        );
   
   static const char* METHOD_NAME;

  private:

   /// no copy constructor
   SyncRpcReset(const SyncRpcReset& nocopy);

   /// no assignment operator
   SyncRpcReset& operator=(const SyncRpcReset& noassignment);
};



/// the registerSync.pullUpdates XML-RPC method.
/**
 * Retrieve all updates for primaryRegistrar whose update number is greater than updateNumber.
 * 
 * Parameters  Type          Name                    Description
 *  Inputs:
 *             string        callingRegistrar        Calling registrar name
 *             string        primaryRegistrar        Primary registrar name
 *             intll         updateNumber
 *  Outputs:
 *             struct
 *               int         numUpdates
 *               array       updates
 *                 struct    row
 *                   string  uri
 *                   string  callid
 *                   int     cseq
 *                   string  contact
 *                   int     expires
 *                   string  qvalue
 *                   string  instanceId
 *                   string  gruu
 *                   string  primary
 *                   intll   updateNumber
 */
class SyncRpcPullUpdates : public SyncRpcMethod
{
  public:

   static XmlRpcMethod* get();

   /// destructor
   virtual ~SyncRpcPullUpdates() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipRegistrar&   registrar);

   /// pull all missing updates for a given primary from a peer
   static RegistrarPeer::SynchronizationState
      invoke(RegistrarPeer* source,       ///< peer to pull from
             const char*    myName,       ///< name of this registrar
             const char*    primaryName,  ///< name of registrar whose updates we want
             intll          updateNumber, ///< pull updates starting after this number
             UtlSList*      bindings      ///< list of RegistrationBinding 
             );
   /**<
    * On success, the bindings are returned.
    * 
    * On any failure, the source is marked UnReachable.
    */
   
  protected:
   /// constructor 
   SyncRpcPullUpdates();

   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        );
   
   static const char* METHOD_NAME;
   static const UtlString NUM_UPDATES;
   static const UtlString UPDATES;
   
  private:

   /// no copy constructor
   SyncRpcPullUpdates(const SyncRpcPullUpdates& nocopy);

   /// no assignment operator
   SyncRpcPullUpdates& operator=(const SyncRpcPullUpdates& noassignment);
};




/// the registerSync.pushUpdates XML-RPC method.
/**
 * Send registry updates to a peer registrar
 * 
 * Parameters  Type        Name                    Description
 *  Inputs:
 *             string      callingRegistrar        Calling registrar name
 *             i8          lastSentUpdateNumber    Number of last update sent
 *             array       updates
 *               struct
 *                 string  uri
 *                 string  callid
 *                 int     cseq
 *                 string  contact
 *                 int     expires
 *                 string  qvalue
 *                 string  instanceId
 *                 string  gruu
 *                 string  primary
 *                 intll   updateNumber
 *  Outputs:
 *             intll       updateNumber
 */
class SyncRpcPushUpdates : public SyncRpcMethod
{
public:
   static XmlRpcMethod* get();

   /// destructor
   virtual ~SyncRpcPushUpdates() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipRegistrar&   registrar);

   /// Push one update to a given peer
   static RegistrarPeer::SynchronizationState
      invoke(RegistrarPeer* replicated, ///< peer to push to
             const char*    myName,     ///< primary name of this registrar
             UtlSList*      bindings    ///< list of RegistrationBinding 
             );
   /**<
    * On success, this updates the sent update number for the peer.
    *
    * On any failure, the peer is marked UnReachable.
    */

protected:
   /// constructor 
   SyncRpcPushUpdates();

   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        );
   
   static const char* METHOD_NAME;

private:
   /// Check lastSentUpdateNumber <= PeerReceivedDbUpdateNumber, otherwise updates are missing
   bool checkLastSentUpdateNumber(intll lastSentUpdateNumber,
                                  RegistrarPeer& peer,
                                  XmlRpcResponse& response);
   /**<
    * If everything is OK, return true.  Otherwise mark the response and return false.
    */

   // Compare the binding's updateNumber with the expected number.
   // Return true if they match and false if they don't.
   // If there is a mismatch, then set up fault info in the RPC reponse.
   bool checkUpdateNumber(const RegistrationBinding& reg,
                          intll updateNumber,
                          XmlRpcResponse& response,
                          ExecutionStatus& status
                          );

   /// no copy constructor
   SyncRpcPushUpdates(const SyncRpcPushUpdates& nocopy);

   /// no assignment operator
   SyncRpcPushUpdates& operator=(const SyncRpcPushUpdates& noassignment);
};



#endif // _SYNCRPC_H_
