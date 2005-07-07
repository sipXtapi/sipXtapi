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
#include <time.h>

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

#define HUNT_GROUP_MAX_Q 800

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
            // iterate through the list of contacts in the
            // sip message, assigning Q values starting at 0.800
            // working our way down in steps of 0.001 and bypassing
            // any contacts that already have qvalues
            int numContacts = response.getCountHeaderFields( SIP_CONTACT_FIELD );

            OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipHuntGroupFilter::reorderContacts - "
                          "applying Hunt Group Algorithm, number of contacts %d\n",numContacts);

            double* qv;
            if ((qv = new double[numContacts]))
            {
                memset(qv, 0, numContacts);
                int count;

                if ((count = getQValues(numContacts, response, &qv)) > 0)  // no contacts without q-value
                {
                    int *idx;
                    if ((idx = new int[numContacts]))
                    {
                        if ((result = randomizeIndex(count, &idx)) == OS_SUCCESS)
                        {
                            result = resetQValues(numContacts, response, idx, qv);
                        }
                        delete idx;
                    }
                    else
                    {
                        result = OS_NO_MEMORY;
                    }
                }
                else
                {
                    result = OS_SUCCESS; // no contacts without q-value
                }
                delete qv;
            }
        }
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

int SipHuntGroupFilter::getQValues(int numContacts,
                                        SipMessage& response,
                                        double** qv)
{
   UtlString nextContact, qValue;
   double nextQValue = 0.80;
   int count = 0;
   for ( int i = 0; i<numContacts; i++ )
   {
       // the response objects contains all the collected
       // contacts from mapping rules etc
       if ( response.getContactField( i, nextContact ) )
       {
           Url contactUri( nextContact );
           if ( !contactUri.getFieldParameter(SIP_Q_FIELD, qValue) )
           {
               (*qv)[count++] = nextQValue;
               nextQValue -= 0.01;
           }
       }
   }

   return count;
}

OsStatus SipHuntGroupFilter::randomizeIndex(int count, int** index)
{
   int *rnd1, *rnd2;
   rnd1 = new int[count];
   rnd2 = new int[count];

   if (!rnd1 || !rnd2) return OS_NO_MEMORY; // :TBD: leak if only one worked!

   int i, j;

   srand( (unsigned)time(NULL) );
   for (i = 0; i < count; i++)
      rnd1[i] = rnd2[i] = rand();

   int tmp;
   for (i = 0; i < count; i++)
   {
      tmp = rnd1[i];
      for (j = i + 1; j < count; j++)
      {
         if (rnd1[j] < tmp)
         {
            rnd1[i] = rnd1[j];
            rnd1[j] = tmp;
            tmp = rnd1[i];
         }
      }
   }

  for (i = 0; i < count; i++)
   {
      for (j = 0; j < count; j++)
      {
         if (rnd1[i] == rnd2[j])
         {
            (*index)[i] = j;
            break;
         }
      }
   }

   delete rnd1;
   delete rnd2;
   return OS_SUCCESS;
}

OsStatus SipHuntGroupFilter::resetQValues(int numContacts,
                                          SipMessage& response,
                                          int* idx,
                                          double* qv)
{
   UtlString nextContact, qValue;
   char temp[10];
   int j = 0;
   for ( int i = 0; i<numContacts; i++ )
   {
       // the response objects contains all the collected
       // contacts from mapping rules etc
       if ( response.getContactField( i, nextContact ) )
       {
           Url contactUri( nextContact );
           if ( !contactUri.getFieldParameter(SIP_Q_FIELD, qValue) )
           {
               UtlString oldUri;
               contactUri.toString(oldUri);
               
               sprintf(temp, "%3.2f", qv[idx[j++]]);
               contactUri.setFieldParameter(SIP_Q_FIELD, temp);
               response.setContactField(contactUri.toString(), i);

	       // Temporary fix for 2.6 -- lower logging level to INFO.
               OsSysLog::add( FAC_SIP, PRI_INFO, "SipHuntGroupFilter::resetQValues() - "
                             "modify URI MAP '%s' to '%s'\n",
                             oldUri.data(), contactUri.toString().data());
           }
       }
   }
   return OS_SUCCESS;
}
