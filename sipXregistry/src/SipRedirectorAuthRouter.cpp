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
#include "SipRegistrar.h"
#include "SipRedirectorAuthRouter.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CONFIG_SETTING_AUTH_PROXY "SIP_REGISTRAR_AUTH_PROXY"

// STATIC VARIABLE INITIALIZATIONS

// Constructor
SipRedirectorAuthRouter::SipRedirectorAuthRouter()
{
}

// Destructor
SipRedirectorAuthRouter::~SipRedirectorAuthRouter()
{
}

// Initializer
OsStatus
SipRedirectorAuthRouter::initialize(const UtlHashMap& configParameters,
                                    OsConfigDb& configDb,
                                    SipUserAgent* pSipUserAgent,
                                    int redirectorNo)
{
   UtlString authProxyConfig;
   if (   (OS_SUCCESS == configDb.get(CONFIG_SETTING_AUTH_PROXY, authProxyConfig) )
       && ! authProxyConfig.isNull()
       )
   {
      Url authUrl(authProxyConfig);
      if ( Url::SipUrlScheme == authUrl.getScheme() )
      {
         authUrl.setUserId(NULL);
         authUrl.setDisplayName(NULL);
         authUrl.removeFieldParameters();
         authUrl.removeHeaderParameters();
         authUrl.setUrlParameter("lr",NULL);

         authUrl.toString(mAuthUrl);
         
         OsSysLog::add(FAC_SIP,
                       (authProxyConfig.compareTo(mAuthUrl, UtlString::ignoreCase)
                        ? PRI_INFO : PRI_NOTICE ),
                       "SipRedirectorAuthRouter::initialize "
                       "authorization proxy route '%s'", mAuthUrl.data()
                       );
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "SipRedirectorAuthRouter::initialize "
                       "invalid route '%s'", authProxyConfig.data());
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "SipRedirectorAuthRouter::initialize "
                    "No authorization proxy specified");
   }

   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorAuthRouter::finalize()
{
}

SipRedirector::LookUpStatus SipRedirectorAuthRouter::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   SipMessage& response,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage)
{
   SipRedirector::LookUpStatus lookupStatus = SipRedirector::LOOKUP_SUCCESS; // always, so far
   
   // Do the cheap global tests first
   //   Is there an authorization proxy route?
   //   Is the request method INVITE? (This operates only on initial invites)
   //   Does the response have any Contacts? (if not, there's nothing to do)
   if (!mAuthUrl.isNull())
   {
      int contacts = response.getCountHeaderFields(SIP_CONTACT_FIELD);
      if (   (method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase) == 0)
          && (contacts)
          )
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectorAuthRouter::lookUp "
                       "checking for To tag"
                       );
         
         /*
          * Loop through each contact in the response,
          *   checking to see if the contact includes a route set
          */
         UtlString contact;
         for (int contactNumber = 0;
              response.getContactEntry(contactNumber, &contact);
              contactNumber++
              )
         {
            Url contactUri(contact);
            
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRedirectorAuthRouter::lookUp "
                          "\n  contact %d '%s'",
                          contactNumber, contact.data()
                          );

            // is there a route header parameter in the contact?
            UtlString routeValue;
            if (contactUri.getHeaderParameter(SIP_ROUTE_FIELD, routeValue))
            {
               // there is a route in the contact
               // prepend the authproxy route to ensure it is checked
               UtlString checkedRoute(mAuthUrl);
               checkedRoute.append(SIP_MULTIFIELD_SEPARATOR);

               checkedRoute.append(routeValue);

               contactUri.setHeaderParameter(SIP_ROUTE_FIELD, checkedRoute);

               // and put the modified contact back into the message
               UtlString modifiedContact;
               contactUri.toString(modifiedContact);
               response.setContactField(modifiedContact, contactNumber);

               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRedirectorAuthRouter::lookUp modified:\n"
                             "   '%s'\n"
                             "in '%s'\n"
                             "to '%s'\n"
                             "in '%s'\n",
                             routeValue.data(), contact.data(),
                             checkedRoute.data(), modifiedContact.data()
                             );
            }
         } // loop over all contacts
      }
      else
      {
         // request is not an INVITE, or no Contact headers in the response
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectorAuthRouter::lookUp "
                       "'%s' request is not an INVITE or has no response contacts (%d) - ignored.",
                       method.data(), contacts
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectorAuthRouter::lookup No authproxy configured");
   }

   return lookupStatus;
}
