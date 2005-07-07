//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _UtlHashBagIterator_h_
#define _UtlHashBagIterator_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlIterator.h"
#include "utl/UtlHashBag.h"
#include "glib.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable ;

/**
 * UtlHashBagIterator allows developers to iterator (walks through) an 
 * UtlHashBag.
 * 
 * @see UtlIterator
 * @see UtlSList
 */
class UtlHashBagIterator : public UtlIterator
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /**
    * Construct an iterator over all objects in a given UtlHashBag
    * If key is specified, iterate only over objects that match that key
    * (UtlHashBags may have any number of copies of a given object)
    */
   UtlHashBagIterator(UtlHashBag& hashBag, UtlContainable* key = NULL);

   /**
     * Destructor
     */
    virtual ~UtlHashBagIterator();

/* ============================ MANIPULATORS ============================== */

    /**
     * Return the next element.
     * 
     * @return The next element or NULL if no more elements are available.
     */
    virtual UtlContainable*    operator()() ;

    /**
     * Reset the list by moving the iterator cursor to the location before the
     * first element. 
     */
    virtual void reset() ; 

/* ============================ ACCESSORS ================================= */

    /**
     * Gets the key of the current element
     */
    UtlContainable* key() const ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static const size_t BEFORE_FIRST;

    friend class UtlHashBag;
    
    /**
     * removing is called by the UtlHashMap when an element is about to be
     * removed from the container.  The iterator must ensure that the element
     * for the removed node is not returned by any subsequent call.
     */
    virtual void removing(const GList* node);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    void init(UtlHashBag* hashBag);
   

    void flush();
   

    static void fillInKeyLists(gpointer key,
                               gpointer value,
                               gpointer user_data
                               );
   
    UtlContainable* mpSubsetMatch; // if non-NULL, points to the key that defines the subset
    
    size_t   mKeyListSize; // size of the mpKeyLists array
    GList**  mpKeyLists;   // each element is the list header (the hash table value entry) for a given key
    size_t   mListIndex;   // index into mpKeyLists
    GList*   mpListNode;   // position in the current list

    // no copy constructor
    UtlHashBagIterator(UtlHashBagIterator&);
} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlHashBagIterator_h_

