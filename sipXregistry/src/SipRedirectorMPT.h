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

#ifndef SIPREDIRECTORMPT_H
#define SIPREDIRECTORMPT_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "SipRedirector.h"
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
class MPTWriterTask : public OsTask {
  public:

   MPTWriterTask::MPTWriterTask(void* pArg);

   virtual int run(void* pArg);
};

/**
 * SipRedirectorMPT is a class whose object returns contacts for the
 * Multi-Party Test system.
 */

class SipRedirectorMPT : public SipRedirector
{
  public:

   SipRedirectorMPT();

   ~SipRedirectorMPT();

   /**
    * Requires the following parameters:
    *
    * mediaServer - the URI of the Media Server.
    *
    * voicemailServer - the URI of the voicemail server.
    *
    * localDomainHost - the SIP domain name
    *
    * mappingRulesFilename - the base name of the mapping rules file
    * to load (e.g., "mappingrules.xml")
    */
   virtual OsStatus initialize(const UtlHashMap& configParameters,
                               OsConfigDb& configDb,
                               SipUserAgent* pSipUserAgent,
                               int redirectorNo);

   virtual void finalize();

   virtual SipRedirector::LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      SipMessage& response,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage);

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
   MPTWriterTask writerTask;

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

#endif // SIPREDIRECTORMPT_H
