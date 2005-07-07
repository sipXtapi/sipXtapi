//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _UtlList_h_
#define _UtlList_h_

//#define GLIST_SANITY_TEST
#ifdef GLIST_SANITY_TEST
# include "assert.h"
#endif

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlContainer.h"

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
 * UtlSList is a singularly linked list designed to contain any number
 * of UtlContainable derived object.  The list may contain non-like objects 
 * (e.g. UtlInts and UtlVoidPtrs), however, sorting and comparison behavior
 * may be non-obvious.
 * 
 * Most list accessors and inquiry methods are performed by equality as 
 * opposed to by referencing (pointers).  For example, a list.contains(obj) 
 * call will loop through all of the list objects and test equality by calling
 * the isEquals(...) method.  A  list.containsReference(obj) call will search 
 * for a pointer match.
 * 
 * @see UtlSListIterator
 * @see UtlContainer
 * @see UtlContainable
 */
class UtlList : public UtlContainer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

// this base class cannot be instantiated directly - the constructor is protected

// Destructor
   virtual UtlList::~UtlList();
   


/* ============================ MANIPULATORS ============================== */
   
    /**
     * Removes and returns the first item in the list (pop).
     * 
     * @return the first object if successful, otherwise null
     */
    virtual UtlContainable* get() ;  

    /**
     * Removed the designated object by reference
     * (as opposed to searching for an equality match).  
     *
     * @return the object if successful, otherwise null
     */
    virtual UtlContainable* removeReference(const UtlContainable* obj);    

    /**
     * Remove the designated object by equality (as opposed to by reference).
     */
    virtual UtlContainable* remove(const UtlContainable* object) = 0;

    /**
     * Removes the designated objects from the list and frees the object 
     * by calling delete.
     */ 
    virtual UtlBoolean destroy(UtlContainable*);

    /**
     * Removes all elements from the list and deletes each one.
     */
    virtual void destroyAll();

    /**
     * Removes all elements from the list without freeing the objects.
     */
    virtual void removeAll();

/* ============================ ACCESSORS ================================= */

    /**
     * Find the first occurence of the designated object by equality (as 
     * opposed to by reference).
     */
    virtual UtlContainable* find(const UtlContainable*) const = 0;

    /**
     * Return the element at position N or null if N is out of bounds.
     */
    virtual UtlContainable* at(size_t N) const;

    /**
     * Return the first element (head) of the list
     */
    virtual UtlContainable* first() const ;

    /**
     * Return the last element (tail) of the list
     */
    virtual UtlContainable* last() const ;

/* ============================ INQUIRY =================================== */

    /**
     * Return the total number of elements within the container
     */
    virtual size_t entries() const;

    /**
     * Return true of the container is empty (entries() == 0), otherwise false.
     */
    virtual UtlBoolean isEmpty() const;

    /**
     * Return true if the container includes the designated object.  Each 
     * element within the list is tested for equality against the designated 
     * object using the equals() method. 
     */
    virtual UtlBoolean contains(const UtlContainable* object) const;

    /**
     * Return true if the list contains the designated object reference.
     */
    virtual UtlBoolean containsReference(const UtlContainable *) const ;

    /**
     * Return the number of occurrences of the designated object
     */
    virtual size_t occurrencesOf(const UtlContainable* obj) const = 0;

    /**
     * Return the list position of the designated object or UTL_NOT_FOUND  if
     * not found.
     */
    virtual size_t index(const UtlContainable* obj) const = 0;


   /**
    * Get the ContainableType for the list as a contained object.
    */
   virtual UtlContainableType getContainableType() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    GList* mpList;

    /**
     * The UtlList constructor is protected - only subclasses may be instantiated
     */
    UtlList::UtlList();
    
    friend class UtlListIterator;

    /**
     * notifyIteratorsOfRemove - called before removing any element in the collection
     */
    void notifyIteratorsOfRemove(GList* element);

    /**
     * removeLink is used internally to manipulate the links.
     *
     * :NOTE: the caller must hold the mContainerLock
     *
     * This does not return a new value for the current list position;
     * this is because it will call the <some-list-iterator>::removing method on the
     * removed element, passing the new value.  This means the that current position
     * update is always done the same way no matter what routine did the removing.
     */
    virtual void removeLink(GList* toBeRemoved);

#ifdef GLIST_SANITY_TEST
#  define GLIST_SANITY_CHECK { if (!sanityCheck()){ assert(FALSE); } }
    bool sanityCheck() const;
#else
#  define GLIST_SANITY_CHECK /* sanityCheck() */
#endif

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    static UtlContainableType TYPE ;    /** < Class type used for runtime checking */
} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlList_h_


