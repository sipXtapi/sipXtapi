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

#ifndef SIPREDIRECTORGATEWAY_H
#define SIPREDIRECTORGATEWAY_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"
#include "digitmaps/UrlMapping.h"
#include "net/HttpServer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Task type to write the mappings file to disk when needed.
class GatewayWriterTask : public OsTask {
  public:

   GatewayWriterTask(void* pArg);

   virtual int run(void* pArg);
};

/**
 * SipRedirectorGateway is a class whose object maps dialing prefixes to
 * host addresses (presumably of SIP gateways).  It also runs an HTTP server
 * by which the mappings can be configured.
 */

class SipRedirectorGateway : public RedirectPlugin
{
  public:

   SipRedirectorGateway(const UtlString& instanceName);

   ~SipRedirectorGateway();

   virtual void readConfig(OsConfigDb& configDb);

   /**
    * Requires the following parameters:
    *
    * MAPPING_FILE - full file name containing the mappings.
    * PREFIX - fixed part of routing prefix
    * DIGITS - number of digits in variable portion of routing prefix
    * PORT - HTTP listening port
    */
   virtual OsStatus initialize(const UtlHashMap& configParameters,
                               OsConfigDb& configDb,
                               SipUserAgent* pSipUserAgent,
                               int redirectorNo);

   virtual void finalize();

   virtual RedirectPlugin::LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      SipMessage& response,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage);

   // File to read/write mappings to.
   UtlString mMappingFileName;

   // Semaphore to lock addess to the maps.
   OsBSem mMapLock;

   // Maps (full) user names into lists of contacts (each of which is ended
   // with newline).
   UtlHashMap mMapUserToContacts;
   // The reverse map from contact strings to user names.
   UtlHashMap mMapContactsToUser;

   // True if the maps have been modified but not written out.
   UtlBoolean mMapsModified;

   // HTTP server for creating further mappings.
   HttpServer* mpServer;

   // File to read/write the mappings.
   UtlString configFileName;

   // Host name for this redirector.
   UtlString mDomainName;

   // Helper task to write the mappings back to disk periodically.
   GatewayWriterTask writerTask;

   void loadMappings(UtlString* file_name,
                     UtlHashMap* mapUserToContacts,
                     UtlHashMap* mapContactsToUser);
   void writeMappings(UtlString* file_name,
                      UtlHashMap* mapUserToContacts);
   // Add a mapping.  Returns the user assigned.
   UtlString* addMapping(const char* contacts,
                         int length);

   static void displayForm(const HttpRequestContext& requestContext,
                           const HttpMessage& request,
                           HttpMessage*& response);
   static void processForm(const HttpRequestContext& requestContext,
                           const HttpMessage& request,
                           HttpMessage*& response);
   static UtlBoolean addMappings(const char* value,
                                 int length,
                                 UtlString*& user,
                                 const char*& error_msg,
                                 int& location);
};

#endif // SIPREDIRECTORGATEWAY_H
