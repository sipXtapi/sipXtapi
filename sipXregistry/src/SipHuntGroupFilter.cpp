// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "sipdb/HuntgroupDB.h"
#include "sipdb/ResultSet.h"
#include "SipRegistrar.h"
#include "SipHuntGroupFilter.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
SipHuntGroupFilter* SipHuntGroupFilter::spInstance = NULL;
OsMutex     SipHuntGroupFilter::sLockMutex (OsMutex::Q_FIFO);
UtlBoolean   SipHuntGroupFilter::sHuntGroupsDefined (FALSE);

/* Hunt group q values are in the range 0.400 <= q < 0.600 */
#define HUNT_GROUP_MAX_Q 600
#define HUNT_RANGE_SIZE  200
#define HUNT_GROUP_MAX_CONTACTS 40

SipHuntGroupFilter::SipHuntGroupFilter()
{
    // Determine whether we have huntgroup supported
    // if the XML file contains 0 rows or the file
    // does not exist then disable huntgroup support
    ResultSet resultSet;
    HuntgroupDB::getInstance()->getAllRows(resultSet);
    if (resultSet.getSize() > 0)
    {
        sHuntGroupsDefined = TRUE;
    }
}

SipHuntGroupFilter::~SipHuntGroupFilter()
{}

OsStatus
SipHuntGroupFilter::reorderContacts (
   const SipMessage& message,
   SipMessage& response )
{
   OsStatus result = OS_SUCCESS;
   if ( sHuntGroupsDefined )
   {
      UtlString uri;
      message.getRequestUri(&uri);
      Url requestUri(uri);

      if ( HuntgroupDB::getInstance()->isHuntGroup(requestUri) )
      {
         int numContacts = response.getCountHeaderFields(SIP_CONTACT_FIELD);

         numContacts = ( (numContacts <= HUNT_GROUP_MAX_CONTACTS)
                        ? numContacts
                        : HUNT_GROUP_MAX_CONTACTS);
         
         int* qDeltas = new int[numContacts]; // records random deltas already used.
         int deltasSet = 0; // count of entries filled in qDeltas 

         UtlString thisContact;
         for (int contactNum = 0; response.getContactField(contactNum, thisContact); contactNum++ )
         {
            Url contactUri( thisContact );
            UtlString qValue;
            
            if (!contactUri.getFieldParameter(SIP_Q_FIELD, qValue))
            {
               // this contact is not explicitly ordered, so generate a q value for it

               if (deltasSet < HUNT_GROUP_MAX_CONTACTS) // we only randomize this many
               {
                  // pick a random delta, ensure it has not been used already
                  bool duplicate=false;
                  do 
                  {
                     // The rand man page says not to use low order bits this way, but
                     // we're not doing security here, just sorting, so this is good enough,
                     // and it's much faster than the floating point they suggest.
                     int thisDelta = 1 + (rand() % HUNT_RANGE_SIZE); // 0 < thisDelta <= HUNT_RANGE_SIZE

                     // check to see that thisDelta is unique in qDeltas
                     int i;
                     for (i = 0, duplicate=false; (!duplicate) && (i < deltasSet); i++)
                     {
                        duplicate = (thisDelta == qDeltas[i]);
                     }

                     if (!duplicate)
                     {
                        // it was unique, so use it
                        qDeltas[deltasSet] = thisDelta;
                        deltasSet++;

                        char temp[6];
                        sprintf(temp, "0.%03d", HUNT_GROUP_MAX_Q - thisDelta);
                        contactUri.setFieldParameter(SIP_Q_FIELD, temp);
                        response.setContactField(contactUri.toString(), contactNum);

                        OsSysLog::add( FAC_SIP, PRI_NOTICE, "hunt URI MAP set '%s' to q=%s\n",
                                      thisContact.data(), temp);
                     }
                  } while(duplicate);
               }
               else
               {
                  // We've randomized HUNT_GROUP_MAX_CONTACTS,
                  // so from here on just set the q value to 0.0
                  contactUri.setFieldParameter(SIP_Q_FIELD, "0.0");
                  response.setContactField(contactUri.toString(), contactNum);
                  OsSysLog::add( FAC_SIP, PRI_WARNING,
                                "hunt URI MAP overflow - set '%s' to q=0.0\n",
                                thisContact.data());
               }
            }
            else
            {
               // thisContact had a q value set - do not modify it
            }
         } // for all contacts

         if (qDeltas)
         {
             delete[] qDeltas;
             qDeltas = NULL;
         }
      }
      else
      {
         // the request uri was not a hunt group - do not modify any contacts
      }
   }
   else
   {
      // no hunt groups configured - do not modify any contacts
   }
   return result;
}


SipHuntGroupFilter*
SipHuntGroupFilter::getInstance()
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new SipHuntGroupFilter ();
    }
    return spInstance;
}
