// 
// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "registry/SipRegistrar.h"
#include "SipRedirectorCallerAlias.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorCallerAlias(instanceName);
}

// Constructor
SipRedirectorCallerAlias::SipRedirectorCallerAlias(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
}

// Destructor
SipRedirectorCallerAlias::~SipRedirectorCallerAlias()
{
}

// Read config information.
void SipRedirectorCallerAlias::readConfig(OsConfigDb& configDb)
{
}

// Initializer
OsStatus
SipRedirectorCallerAlias::initialize(OsConfigDb& configDb,
                                     SipUserAgent* pSipUserAgent,
                                     int redirectorNo,
                                     const UtlString& localDomainHost)
{
   mpCallerAliasDB = CallerAliasDB::getInstance();

   if (mpCallerAliasDB)
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "SipRedirectorCallerAlias initialized");
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING, "SipRedirectorCallerAlias - no CallerAliasDB");
   }
   
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorCallerAlias::finalize()
{
   if (mpCallerAliasDB)
   {
      mpCallerAliasDB->releaseInstance();
      mpCallerAliasDB = NULL;
   }
}

RedirectPlugin::LookUpStatus SipRedirectorCallerAlias::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   SipMessage& response,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage)
{
   RedirectPlugin::LookUpStatus lookupStatus = RedirectPlugin::LOOKUP_SUCCESS; // always, so far
   
   // Do the cheap global tests first
   //   Is there a caller alias database? (should always be true)
   //   Is the request method INVITE? (This operates only on initial invites)
   //   Does the response have any Contacts? (if not, there's nothing to do)
   if (mpCallerAliasDB)
   {
      int contacts = response.getCountHeaderFields(SIP_CONTACT_FIELD);
      if (   (method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase) == 0)
          && (contacts)
          )
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectorCallerAlias::lookUp "
                       "checking for To tag"
                       );
         
         // see if there is a To tag to decide if this is a new dialog
         Url toUrl;
         message.getToUrl(toUrl);
         UtlString ignoreValue;      
         if (! toUrl.getUrlParameter("tag", ignoreValue, 0)) // no To tag on the request?
         {
            // no tag on the To field - this is an initial INVITE.

            /*
             * Get the callers identity by getting the caller URI and:
             *    remove all parameters
             *    remove the scheme name
             */
            UtlString callerIdentity;

            // start with the From header field (someday we should use the Identity if present)
            Url fromUrl;
            message.getFromUrl(fromUrl);
            fromUrl.removeParameters(); // parameters are not relevant for this 
         
            Url::Scheme fromUrlScheme = fromUrl.getScheme();
            switch (fromUrlScheme)
            {
            case Url::SipsUrlScheme:
               // sips and sip are equivalent for identity purposes,
               //   so just set to sip 
               fromUrl.setScheme(Url::SipUrlScheme);
               //   and fall through to extract the identity...

            case Url::SipUrlScheme:
               // case Url::TelUrlScheme: will go here, since 'tel' and 'sip' are the same length
               fromUrl.getUri(callerIdentity);
               callerIdentity.remove(0,4 /* strlen("sip:") */); // strip off the scheme name
               break;

            default:
               // for all other schemes, treat identity as null
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "SipRedirectorCallerAlias::lookUp From uses unsupported scheme '%s'"
                             " - using null identity",
                             fromUrl.schemeName(fromUrlScheme)
                             );
               break;
            }

            /* determine whether the identity is one for which this registrar
             * is authoritative; if not, we will not use wildcard matches. */
            bool identityIsLocal = SipRegistrar::getInstance()->isValidDomain(fromUrl);
            
            // now we have callerIdentity set; use for looking up each contact.
            
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRedirectorCallerAlias::lookUp "
                          "\n  caller '%s' %s",
                          callerIdentity.data(),
                          identityIsLocal ? "is local" : "is not local"
                          );

            /*
             * Loop through each contact in the response,
             *   checking for a caller alias set for its domain with callerIdentity
             */
            UtlString contact;
            for (int contactNumber = 0;
                 response.getContactEntry(contactNumber, &contact);
                 contactNumber++
                 )
            {
               Url contactUri(contact);
            
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRedirectorCallerAlias::lookUp "
                             "\n  contact %d '%s'",
                             contactNumber, contact.data()
                             );

               // is there already a From header parameter on this?
               if (!contactUri.getHeaderParameter(SIP_FROM_FIELD, ignoreValue))
               {
                  // there is no From header parameter, so we can add one if we find an alias

                  // extract the domain value from the contact
                  UtlString contactDomain;
                  contactUri.getHostWithPort(contactDomain);

                  // look up any caller alias for this identity and contact domain
                  UtlString callerAlias;
                  UtlString nullId; // empty string for wildcard matches
                  if (mpCallerAliasDB->getCallerAlias(callerIdentity, contactDomain, callerAlias)
                      || (   identityIsLocal
                          && mpCallerAliasDB->getCallerAlias(nullId, contactDomain, callerAlias)
                          )
                      )
                  {
                     // found a caller alias, so add it as a header parameter to the contact
                     contactUri.setHeaderParameter(SIP_FROM_FIELD, callerAlias);

                     // and put the modified contact back into the message
                     UtlString contactWithFrom;
                     contactUri.toString(contactWithFrom);
                     response.setContactField(contactWithFrom, contactNumber);

                     OsSysLog::add( FAC_SIP, PRI_DEBUG,
                                   "SipRedirectorCallerAlias::lookUp set caller alias "
                                   "'%s' for caller '%s' to '%s'\n",
                                   callerAlias.data(), callerIdentity.data(), contactDomain.data()
                                   );
                  }
               }
               else
               {
                  // this contact had a From header parameter set - do not modify it.
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "SipRedirectorCallerAlias::lookUp "
                                "using existing From header '%s'",
                                ignoreValue.data()
                                );

               }
            } // loop over all contacts
         }
         else
         {
            // there is a To tag, so this is a reINVITE; do not alias it.
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRedirectorCallerAlias::lookUp "
                          "found To tag '%s' - not rewriting",
                          ignoreValue.data()                          
                          );
         
         }
      }
      else
      {
         // request is not an INVITE, or no Contact headers in the response
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectorCallerAlias::lookUp "
                       "'%s' request is not an INVITE or has no response contacts (%d) - ignored.",
                       method.data(), contacts
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectorCallerAlias::lookup No CallerAliasDB");
   }

   return lookupStatus;
}
