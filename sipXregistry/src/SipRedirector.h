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

#ifndef SIPREDIRECTOR_H
#define SIPREDIRECTOR_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "net/SipMessage.h"
#include "utl/UtlString.h"
#include "net/Url.h"
#include "os/OsConfigDb.h"
#include "net/SipUserAgent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/**
 * The type of the request sequence numbers.
 *
 * Must be a type for which '++' will increment with rollover when
 * the end of its range is reached.
 */
typedef unsigned int RequestSeqNo;

// FORWARD DECLARATIONS

class SipRedirectorPrivateStorage;

/**
 * SipRedirector is a virtual class.  Instances of its subclasses are the
 * processing agents which construct and manipulate lists of contact points
 * for addresses of record.
 */

class SipRedirector
{
  public:

   /**
    * The destructor does almost nothing, as the primary finalization
    * work should be done by ::finalize().
    */
   virtual ~SipRedirector();

   /**
    * All initialization should be done in ::initialize() in preference
    * to in the constructor, so that we have finer control over when
    * it happens.
    *
    * configParameters is a UtlHashMap that gives various
    * configuration parameters that ::initialize() may want to use.
    *
    * configDb is the OsConfigDb containing the configuration
    * parameters from the file registrar-config.
    *
    * pSipUserAgent is a pointer to the SipUserAgent to use for
    * communication.
    *
    * redirectorNo is the number this redirector is assigned.
    *
    * @return ::initialize() returns OS_SUCCESS if it has successfully
    * initialized and wishes to process requests, and OS_FAILED if it
    * does not wish to process requests.  If it has detected an error,
    * it must output an error message on its own, as OS_FAILED per se isn't
    * an error signal.
    */
   virtual OsStatus initialize(const UtlHashMap& configParameters,
                               OsConfigDb& configDb,
                               SipUserAgent* pSipUserAgent,
                               int redirectorNo) = 0;

   /**
    * All finalization should be done in ::finalize() in preference
    * to in the destructor, so that we have finer control over when
    * it happens.
    */
   virtual void finalize() = 0;

   /**
    * Look up redirections and add them to the response.
    *
    * message is the incoming SIP message.
    *
    * requestString is the request URI from the SIP message as a UtlString.
    *
    * requestUri is the request URI from the SIP message as a Uri, for
    * use in debugging messages.
    *
    * response is the response SIP message that we are in the process
    * of building.
    *
    * privateStorage is the cell containing the pointer to the private
    * storage object for this redirector.
    *
    * @return the success/error/suspend indication showing the outcome
    * of processing:
    * LOOKUP_SUCCESS means the redirector successfully finished its processing.
    * LOOKUP_SUSPEND means the redirector needs the request to be suspended
    * for asynchronous processing.
    * LOOKUP_ERROR_REQUEST means that the request was invalid in some way
    * (e.g., a call retrieve from an invalid orbit number) and the request
    * should get a 403 Forbidden response.
    * LOOKUP_ERROR_SERVER means that the redirector encountered an internal
    * error and the request should get a 500 response
    * If it returns LOOKUP_ERROR, the redirector should most likely have
    * logged message(s) at ERR level giving the details of the problem.
    *
    * The SipRedirectServer will be holding mMutex while lookUp is called.
    * See ../doc/Redirection.txt for more details on how lookUp is called.
    */
   typedef enum LookUpStatus
   {
      // Start numbering status values from 1 so 0 is invalid.
      LOOKUP_SUCCESS = 1,
      LOOKUP_ERROR_REQUEST,
      LOOKUP_ERROR_SERVER,
      LOOKUP_SUSPEND
   } LookUpStatus;
   virtual LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      SipMessage& response,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      class SipRedirectorPrivateStorage*& privateStorage) = 0;

   /**
    * Cancel processing of a request.
    *
    * If lookUp has ever returned LOOKUP_SUSPEND for this request, the
    * redirect server guarantees to call cancel after the moment when
    * lookUp will never be called again for the request.
    *
    * The private storage for the redirector that is managed by
    * SipRedirectServer will be deleted after cancel() is called.
    *
    * request is the sequence number of the request to be canceled.
    *
    * The SipRedirectServer will be holding mMutex while cancel() is called.
    *
    * This call must not block.
    */
   virtual void cancel(RequestSeqNo request);

   /**
    * Add a contact to the redirection list.
    *
    * response is the response SIP message we are constructing, for
    * use in debugging messages.
    *
    * contact is the contact URI to be added to the redirection list.
    *
    * label is a string that described this redirector, for use in
    * debugging messages.
    *
    * requestString is the request URI as a UtlString 
    */
   static void addContact(SipMessage& response,
                          const UtlString& requestString,
                          const Url& contact,
                          const char* label);

   /**
    * Remove all contacts from the redirection list.
    */
   static void removeAllContacts(SipMessage& response);

  protected:

   /**
    * Declare that a redirector is ready to reprocess a request.
    *
    * This method may be called from any context and does not block,
    * because all it does is queue a message on the redirect server's
    * queue.  It should not be called more than once following each
    * invocation of lookUp that returns LOOKUP_SUSPEND.
    *
    * request is the sequence number of the request (obtained from the
    * invocation of lookUp).
    *
    * redirector is the identifier of this redirector (ditto)
    */
   static void resumeRedirection(RequestSeqNo request,
                                 int redirector);
};

/**
 * SipRedirectorPrivateStorage is a virtual class.  Instances of its
 * subclasses can be saved by redirectors on the master list of
 * suspended requests.
 *
 * SipRedirectorPrivateStorage is a subclass of UtlContainable, so
 * pointers to it can be returned by iterators.  But its hash and comparison
 * functions are dummies, and should be overridden if one wants to use
 * these objects as indexes.
 */

class SipRedirectorPrivateStorage : public UtlContainable
{
  public:
   
   virtual ~SipRedirectorPrivateStorage();

   unsigned int hash() const;

   virtual const char* const getContainableType() const;

   virtual int compareTo(const UtlContainable*) const;
};

#endif // SIPREDIRECTOR_H
