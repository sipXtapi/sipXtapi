//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _SipContactDb_h_
#define _SipContactDb_h_

// SYSTEM INCLUDES
//#include <...>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>
#include <os/OsMutex.h>
#include <os/OsSocket.h>
#include "tapi/sipXtapi.h"

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS



// FORWARD DECLARATIONS

class SipContactDb
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
    SipContactDb();

    virtual
    ~SipContactDb();

    /**
     * Inserts a contact into the contact table.  Fails if there is
     * already an entry with the same port and IP address.
     * If the ID of the incoming CONTACT_ADDRESS is less that 1,
     * which it should be, then this method will assign
     * a contact id.
     * @param contact Reference to a contact structure, which will be
     *        copied, and the copy will be added to the DB.
     */   
    const bool addContact(SIPX_CONTACT_ADDRESS& contact);


    /**
     * Searches for a similar contact and the datebase and updates the
     * address/port.
     * 
     * @param contact Reference to a contact structure
     */   
    const bool updateContact(SIPX_CONTACT_ADDRESS& contact);

    /**
     * Removes a contact record from the DB.  
     *
     * @param id Key value (the contact id) used to find
     *        a matching record for deletion.
     */
    const bool deleteContact(const SIPX_CONTACT_ID id);
    
    /** 
     * Finds a contact in the DB, by SIPX_CONTACT_ID.
     *
     * @param id The SIPX_CONTACT_ID of the record to find.
     */
    SIPX_CONTACT_ADDRESS* find(SIPX_CONTACT_ID id);

    /** 
     * Finds a contact in the DB, by IP address.
     *
     * @param id The IP Address of the record to find.
     */    
	SIPX_CONTACT_ADDRESS* find(const UtlString szIpAddress, const int port, SIPX_CONTACT_TYPE type);

    /**
     * Finds the first contact by a given contact type
     */
    SIPX_CONTACT_ADDRESS* findByType(SIPX_CONTACT_TYPE type, SIPX_TRANSPORT_TYPE transportType, UtlString sCustomTransport = "") ;

    /**
     * Return a transport type given the specified transport name.  The name 
     * could be tls, tcp, udp, or a custom transport type.
     */
    SIPX_TRANSPORT_TYPE findTransportType(const char* transportName) ;

    /*
     * Find the local contact from a contact id.
     */
    SIPX_CONTACT_ADDRESS* getLocalContact(SIPX_CONTACT_ID id) ;
    
    /**
     * Populates a CONTACT_ADDRESS array with all of the contacts
     * stored in this DB.
     *
     * @param contacts Pre-allocated array of CONTACT_ADDRESS pointers.
              Should be allocated using the MAX_IP_ADDRESSES for the size.
     * @param actualNum The number of contacts.
     */
    void getAll(SIPX_CONTACT_ADDRESS* contacts[], int& actualNum) const;
    
    
    /**
     * Populates a CONTACT_ADDRESS array with all of the contacts
     * stored in this DB that match a particular adapter name.
     *
     * @param contacts Pre-allocated array of CONTACT_ADDRESS pointers.
              Should be allocated using the MAX_IP_ADDRESSES for the size.
     * @param szAdapter Adapter name for which to look-up contacts.
     * @param actualNum The number of contacts.
     */
    void getAllForAdapter(const SIPX_CONTACT_ADDRESS* contacts[],
                          const char* szAdapter,
                          int& actualNum,
                          SIPX_CONTACT_TYPE typeFilter = CONTACT_ALL) const;
                                    
    const bool getRecordForAdapter(SIPX_CONTACT_ADDRESS& contact,
                                   const char* szAdapter,
                                   const SIPX_CONTACT_TYPE contactFilter) const;

    const bool getRecordForAdapter(SIPX_CONTACT_ADDRESS& contact,
                                   const char* szAdapter,
                                   const SIPX_CONTACT_TYPE contactFilter,
                                   const SIPX_TRANSPORT_TYPE transportFilter) const;

    
    void replicateForTransport(const SIPX_TRANSPORT_TYPE originalTransport, 
                               const SIPX_TRANSPORT_TYPE newTransport,
                               const char* szTransport,
                               const char* szRoutingID);

    void removeForTransport(const SIPX_TRANSPORT_TYPE transport);

    void dump(UtlString& output) ;

/* ============================ MANIPULATORS ============================== */

    void enableTurn(bool bEnable) ;


/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:



/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /** Disabled copy constructor */
    SipContactDb(const SipContactDb& rSipContactDb);

    //** Disabled assignment operator */
    SipContactDb& operator=(const SipContactDb& rhs);
    
    /** Checks this database for a duplicate record by key */
    const bool isDuplicate(const SIPX_CONTACT_ID id);
    
    /** Checks this database for a duplicate record by ipAddress and port */
    const bool isDuplicate(const UtlString& ipAddress, const int port, SIPX_CONTACT_TYPE type, SIPX_TRANSPORT_TYPE transportType);

    /**
     * Given a contact record containing an ID which is set
     * to a value less than 1, this method will generate a contact 
     * ID.
     * 
     * @param contact Reference to the CONTACT_ADDRESS object to be
     *        modified.
     */
    const bool assignContactId(SIPX_CONTACT_ADDRESS& contact);

    /** hash map storage for contact information, keyed by Contact Record ID */
    UtlHashMap mContacts;   

    int mNextContactId;
    
    mutable OsMutex mLock;

    bool mbTurnEnabled ;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipContactDb_h_
