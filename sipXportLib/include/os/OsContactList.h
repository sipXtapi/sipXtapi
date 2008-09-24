//
// Copyright (C) 2006 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsContactList_h_
#define _OsContactList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlSList.h"
#include "os/OsContact.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;

/**
 * OsContactList is a simple extension of UtlSList that allows for the
 * setting and getting of a 'primary' contact.
 *
 * @see UtlSList
 * @see UtlContainer 
 * @see UtlContainable
 */
class OsContactList : public UtlSList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ FRIENDS =================================== */
    friend class OsContactListTest;

/* ============================ CREATORS ================================== */

    
    /**
     * Default Constructor
     */
    OsContactList();

/* ============================ MANIPULATORS ============================== */

    /**
     * Method for designating a 'primary' contact, which may or 
     * may not already be in the list.  If it is not already in the list
     * it is added to the list.
     * 
     * @param contact The contact to set as the 'primary'.
     */    
    void setPrimary(const OsContact& contact);

/* ============================ INQUIRY =================================== */

    /**
     * Method for gettting the 'primary' contact.  If no primary contact has
     * be explicitly set with setPrimary, the first contact in the list is 
     * returned.
     */    
    const OsContact* getPrimary();


    /**
     * Get the ContainableType for the OsContactList as a contained object.
     */
    virtual UtlContainableType getContainableType() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlSListIterator;


    static const UtlContainableType TYPE;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        
} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _OsContactList_h_


