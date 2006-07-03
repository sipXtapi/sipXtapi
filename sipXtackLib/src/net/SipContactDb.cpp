//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "net/SipContactDb.h"
#include "utl/UtlInt.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsLock.h"
#include "utl/UtlHashMapIterator.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
#ifdef WIN32
#  define strcasecmp stricmp
#  define strncasecmp strnicmp
#endif


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipContactDb::SipContactDb() : 
    mNextContactId(1),
    mLock(OsMutex::Q_FIFO),
    mbTurnEnabled(FALSE)
{
    
}

// Destructor
SipContactDb::~SipContactDb()
{
    UtlHashMapIterator iterator(mContacts);
    
    UtlInt* pKey = NULL;
    while (pKey = (UtlInt*)iterator())
    {
        UtlVoidPtr* pValue = NULL;
        pValue = (UtlVoidPtr*)iterator.value();
        if (pValue)
        {
            delete pValue->getValue();
        }
    }
    mContacts.destroyAll();
}


/* ============================ MANIPULATORS ============================== */

const bool SipContactDb::addContact(SIPX_CONTACT_ADDRESS& contact)
{
    OsLock lock(mLock);
    bool bRet = false;
    
    assert (contact.id < 1);
    
    if (!isDuplicate(contact.cIpAddress, contact.iPort, contact.eContactType, contact.eTransportType))
    {
        assignContactId(contact);

        SIPX_CONTACT_ADDRESS* pContactCopy = new SIPX_CONTACT_ADDRESS(contact);
        mContacts.insertKeyAndValue(new UtlInt(pContactCopy->id), new UtlVoidPtr(pContactCopy));

        // If turn is enabled, duplicate the contact with a relay type
        if (mbTurnEnabled && contact.eContactType != CONTACT_RELAY)
        {
            pContactCopy = new SIPX_CONTACT_ADDRESS(contact);
            pContactCopy->eContactType = CONTACT_RELAY ;
            pContactCopy->id = 0;
            addContact(*pContactCopy);
        }
        bRet = true;
    }
    else
    {
        // fill out the information in the contact,
        // to match what is already in the database
        contact = *(find(contact.cIpAddress, contact.iPort, contact.eContactType));
    }
    return bRet;
}

const bool SipContactDb::updateContact(SIPX_CONTACT_ADDRESS& contact)
{
    bool bFound = false ;
    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);
    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    UtlString replacedAddress ;
    int replacedPort ;



    // Pass one: replace the exact match
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        assert(pContact) ;
        if (    (pContact->eContactType == contact.eContactType) &&
                (pContact->eTransportType == contact.eTransportType) &&
                (strcasecmp(pContact->cInterface, contact.cInterface) == 0) &&
                (strcasecmp(pContact->cCustomTransportName, 
                contact.cCustomTransportName) == 0) &&
                (strcasecmp(pContact->cCustomRouteID, 
                contact.cCustomRouteID) == 0) )
        {
            bFound = true ;
            replacedAddress = pContact->cIpAddress ;
            replacedPort = pContact->iPort ;
            strcpy(pContact->cIpAddress, contact.cIpAddress) ;            
            pContact->iPort = contact.iPort ;
            break ;
        }
    }

    // Pass two: replace anything else that had the same IP/port combo.  
    // This pass adjusted relay and external transport duplicates.
    if (bFound)
    {
        while (pKey = (UtlInt*)iterator())
        {
            pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
            assert(pValue);
            
            pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
            assert(pContact) ;
            if ((   replacedAddress.compareTo(pContact->cIpAddress) == 0) &&
                    (replacedPort == pContact->iPort))
            {
                strcpy(pContact->cIpAddress, contact.cIpAddress) ;            
                pContact->iPort = contact.iPort ;
            }
        }
    }

    return bFound ;
}

const bool SipContactDb::deleteContact(const SIPX_CONTACT_ID id)
{
    OsLock lock(mLock);
    UtlInt idKey(id);
    return mContacts.destroy(&idKey);
}

SIPX_CONTACT_ADDRESS* SipContactDb::find(SIPX_CONTACT_ID id)
{
    OsLock lock(mLock);
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt idKey(id);
    
    UtlVoidPtr* pValue = (UtlVoidPtr*)mContacts.findValue(&idKey);
    if (pValue)
    {
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
    }
    
    return pContact;
}


// Finds the first contact by a given contact type
SIPX_CONTACT_ADDRESS* SipContactDb::findByType(SIPX_CONTACT_TYPE type, SIPX_TRANSPORT_TYPE transportType, UtlString sCustomTransport) 
{
    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);
    SIPX_CONTACT_ADDRESS* pRC = NULL ;

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        assert(pContact) ;
        if (transportType != OsSocket::UNKNOWN && transportType == pContact->eTransportType)
        {
            if (pContact->eContactType == type)
            {
				if (sCustomTransport.length() == 0)
				{
					pRC = pContact ;
					break ;
				}
				else
				{
                    if (sCustomTransport.compareTo(pContact->cCustomTransportName, UtlString::ignoreCase) == 0)
					{
						pRC = pContact;
					}
				}
            }
        }
    }
    return pRC ;
}


// Return a transport type given the specified transport name.  The name 
// could be tls, tcp, udp, or a custom transport type.
SIPX_TRANSPORT_TYPE SipContactDb::findTransportType(const char* transportName) 
{
    SIPX_TRANSPORT_TYPE eType = TRANSPORT_UDP ;

    assert(transportName) ;
    if (transportName && strlen(transportName)) 
    {
        UtlString transport(transportName) ;
 
        if (transport.compareTo("tcp", UtlString::ignoreCase) == 0)
        {
            eType = TRANSPORT_TCP ;
        }
        else if (transport.compareTo("udp", UtlString::ignoreCase)== 0)
        {
            eType = TRANSPORT_UDP ;
        }
        else if (transport.compareTo("tls", UtlString::ignoreCase)== 0)
        {
            eType = TRANSPORT_TLS ;
        }
        else
        {
            OsLock lock(mLock);
            UtlHashMapIterator iterator(mContacts);
            UtlVoidPtr* pValue;
            UtlInt* pKey;
            SIPX_CONTACT_ADDRESS* pContact = NULL;
    
            while (pKey = (UtlInt*)iterator())
            {
                pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
                assert(pValue); // Should NEVER happen
                pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
                assert(pContact) ; // Should NEVER happen

                if (    (pContact->eTransportType >= TRANSPORT_CUSTOM) && 
                        (transport.compareTo(pContact->cCustomTransportName, 
                        UtlString::ignoreCase) == 0))
                {
                    eType = pContact->eTransportType ;
                    break ;
                }
            }
        }
    }

    return eType ;
}


// Find the local contact from a contact id
SIPX_CONTACT_ADDRESS* SipContactDb::getLocalContact(SIPX_CONTACT_ID id)
{
    OsLock lock(mLock);

    SIPX_CONTACT_ADDRESS* pRC = NULL ;
    SIPX_CONTACT_ADDRESS* pOriginal = find(id) ;
    if (pOriginal)
    {
        if (pOriginal->eContactType == CONTACT_LOCAL)
        {
            pRC = pOriginal ;
        }
        else
        {
            UtlHashMapIterator iterator(mContacts);
            UtlVoidPtr* pValue = NULL;
            SIPX_CONTACT_ADDRESS* pContact = NULL;
            UtlInt* pKey;
            while (pKey = (UtlInt*)iterator())
            {
                pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
                assert(pValue);
                
                pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
                assert(pContact) ;
                if ((strcmp(pContact->cInterface, pOriginal->cInterface) == 0) && 
                    (pContact->eContactType == CONTACT_LOCAL))
                {
                    pRC = pContact ;
                    break ;
                }
            }
        }
    }

    return pRC ;
}


SIPX_CONTACT_ADDRESS* SipContactDb::find(const UtlString ipAddress, const int port, SIPX_CONTACT_TYPE type)
{
    OsLock lock(mLock);
    bool bFound = false;
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        if (    (pContact->eContactType == type) &&
                (strcmp(pContact->cIpAddress, ipAddress.data()) == 0))
        {
            if (port < 0 || port == pContact->iPort)
            {
                bFound = true;
                break;
            }
        }
    }
    
    if (!bFound)
    {
        pContact = NULL;
    }
        
    return pContact;
}

void SipContactDb::getAll(SIPX_CONTACT_ADDRESS* contacts[], int& actualNum) const
{

    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    actualNum = 0; // array index
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        contacts[actualNum] = pContact;
        actualNum++;
    }
    return;
}


const bool SipContactDb::getRecordForAdapter(SIPX_CONTACT_ADDRESS& contact,
                                             const char* szAdapter,
                                             const SIPX_CONTACT_TYPE contactFilter) const
{
    bool bRet = false;

    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        
        if (0 != strcmp(pContact->cInterface, szAdapter))
        {
            continue;
        }
        if (pContact->eContactType != contactFilter)
        {
            continue;
        }

        contact = *pContact;
        bRet = true;
        break;
    }
    return bRet;
}

const bool SipContactDb::getRecordForAdapter(SIPX_CONTACT_ADDRESS& contact,
                                             const char* szAdapter,
                                             const SIPX_CONTACT_TYPE contactFilter,
                                             const SIPX_TRANSPORT_TYPE transportFilter) const
{
    bool bRet = false;

    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        
        if (0 != strcmp(pContact->cInterface, szAdapter))
        {
            continue;
        }
        if ((pContact->eContactType != contactFilter) || 
                (pContact->eTransportType != transportFilter))
        {
            continue;
        }

        contact = *pContact;
        bRet = true;
        break;
    }
    return bRet;
}
                                             
void SipContactDb::getAllForAdapter(const SIPX_CONTACT_ADDRESS* contacts[],
                                    const char* szAdapter,
                                    int& actualNum, 
                                    const SIPX_CONTACT_TYPE contactFilter) const
{

    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    actualNum = 0; // array index
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        
        if (0 != strcmp(pContact->cInterface, szAdapter))
        {
            continue;
        }
        if (contactFilter != CONTACT_ALL && pContact->eContactType != contactFilter)
        {
            continue;
        }

        contacts[actualNum] = pContact;
        actualNum++;
    }
    
    return;
}


void SipContactDb::enableTurn(bool bEnable) 
{
    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    mbTurnEnabled = bEnable ;    

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        if (pContact)
        {
            if (mbTurnEnabled)
            {
                if (pContact->eContactType != CONTACT_RELAY)
                {
                    SIPX_CONTACT_ADDRESS* pContactCopy = new SIPX_CONTACT_ADDRESS(*pContact);
                    pContactCopy->eContactType = CONTACT_RELAY ;
                    assignContactId(*pContactCopy) ;
                    mContacts.insertKeyAndValue(new UtlInt(pContactCopy->id), new UtlVoidPtr(pContactCopy));
                }
            }
            else
            {
                if (pContact->eContactType == CONTACT_RELAY)
                {
                    deleteContact(pContact->id) ;
                }
            }
        }        
    }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */



/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */

const bool SipContactDb::isDuplicate(const SIPX_CONTACT_ID id)
{
    OsLock lock(mLock);
    bool bRet = false;
    UtlInt idKey(id);
    
    UtlVoidPtr* pValue = (UtlVoidPtr*)mContacts.findValue(&idKey);
    if (pValue)
    {
        bRet = true;
    }
    return bRet;
}

const bool SipContactDb::isDuplicate(const UtlString& ipAddress, 
                                     const int port, SIPX_CONTACT_TYPE type, 
                                     SIPX_TRANSPORT_TYPE transportType)
{
    OsLock lock(mLock);
    bool bRet = false;
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        if (    (pContact->eContactType == type) &&
                (strcmp(pContact->cIpAddress, ipAddress.data()) == 0) &&
                pContact->eTransportType == transportType)
        {
            if (port < 0 || port == pContact->iPort)
            {
                bRet = true;
                break;
            }
        }
    }
    return bRet;    
}

const bool SipContactDb::assignContactId(SIPX_CONTACT_ADDRESS& contact)
{
    OsLock lock(mLock);
    
    contact.id = mNextContactId;
    mNextContactId++;
    
    return true;
}

void SipContactDb::replicateForTransport(const SIPX_TRANSPORT_TYPE originalTransportType, 
                                         const SIPX_TRANSPORT_TYPE newTransport,
                                         const char* szTransport,
                                         const char* szRoutingID)
{
    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    int index = 0; // array index
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        if (pContact->eTransportType == originalTransportType)
        {
            SIPX_CONTACT_ADDRESS* newContact = new SIPX_CONTACT_ADDRESS(*pContact);
            newContact->id = 0;
            newContact->eTransportType = newTransport;
            memset(newContact->cCustomTransportName, 0, sizeof(newContact->cCustomTransportName)) ;
            if (szTransport)
                strncpy(newContact->cCustomTransportName, szTransport, sizeof(newContact->cCustomTransportName));                
            memset(newContact->cCustomRouteID, 0, sizeof(newContact->cCustomRouteID)) ;
            if (szRoutingID)
                strncpy(newContact->cCustomRouteID, szRoutingID, sizeof(newContact->cCustomRouteID));

            addContact(*newContact);
        }
        index++;
    }
    
    return;
}

void SipContactDb::removeForTransport(const SIPX_TRANSPORT_TYPE transport)
{
    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    int index = 0; // array index
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);
        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();
        if (pContact->eTransportType == transport)
        {
            deleteContact(pContact->id);
        }
        index++;
    }

    return;
}


void SipContactDb::dump(UtlString& output) 
{
    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    output.append("\nSipContactDB Dump:\n") ;

    UtlVoidPtr* pValue = NULL;
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    UtlInt* pKey;
    int index = 0; // array index
    while (pKey = (UtlInt*)iterator())
    {
        pValue = (UtlVoidPtr*)mContacts.findValue(pKey);
        assert(pValue);        
        pContact = (SIPX_CONTACT_ADDRESS*)pValue->getValue();

        char cFoo[512] ;
        sprintf(cFoo, "   %d %d %d %s/%s:%d %s %s\n",
                pContact->id,
                pContact->eContactType,
                pContact->eTransportType,
                pContact->cInterface,
                pContact->cIpAddress,
                pContact->iPort,
                pContact->cCustomTransportName,
                pContact->cCustomRouteID) ;
        
        output.append(cFoo) ;
    }

    return;
}

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

