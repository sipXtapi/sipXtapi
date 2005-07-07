//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _UtlHashMap_h_
#define _UtlHashMap_h_

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
class UtlContainable;

/**
 * UtlHashMap is a container object that allows you to store keys and 
 * values.  Key must be unique (testing for equality using the
 * UtlContainer::isEquals(...) method).
 */
class UtlHashMap : public UtlContainer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   friend class UtlHashMapIterator;

/* ============================ CREATORS ================================== */

    /**
     * Default Constructor
     */
    UtlHashMap();


    /**
     * Constructor accepting a starting number of bins and a flag to control
     * the allocation of buckets.  If you imagine the UtlHashMap implemented 
     * using an array, the number of bins is the array size and the buckets 
     * are a data structure filled into each array slot.
     *
     * @param numBins The initial number of bins.
     * @param bPrealloc true to preallocate all of the buckets, false to allow
     *        the buckets to be allocated when needed.
     */
    UtlHashMap(int numBins, UtlBoolean bPrealloc = false);


    /**
     * Destructor
     */
    virtual ~UtlHashMap();

/* ============================ MANIPULATORS ============================== */

    /**
     * Inserts a key and value pair into the hash map.
     *
     * If the inserted key is already in the table, this method 
     * fails (returns NULL - note that this means if value is NULL,
     * then you can't tell whether  there was an error or not).
     * To replace the value for a given key, the old value must
     * be Removed before the new value is inserted.
     *
     * @return the key on success, otherwise NULL
     */
    UtlContainable* insertKeyAndValue(UtlContainable* key, UtlContainable* value);


    /**
     * Inserts the designated containable object into the list
     * with a NULL value (see note regarding use of NULL value
     * in insertKeyAndValue).
     * 
     * @return the object if successful, otherwise NULL
     */
    UtlContainable* insert(UtlContainable* obj);


    /**
     * Remove the designated key and its associated value.
     *
     * @return the key or NULL if not found
     */
    UtlContainable* remove(UtlContainable* key);


    /**
     * Remove the designated key and its associated value.
     *
     * @return the key or NULL if not found
     */
    UtlContainable* removeReference(const UtlContainable* key);
    

    /**
     * Remove the designated key and its associated value.  The pointer of value
     * is returned as part of the call if successful.
     *
     * @return the key or NULL if not found
     */
    UtlContainable* removeKeyAndValue(const UtlContainable* key, UtlContainable*& value);


    /**
     * Removes the designated key and its associated value from the map
     * and frees the object by calling delete.
     */ 
    virtual UtlBoolean destroy(UtlContainable* key);    


    /**
     * Removes all elements from the hash map and deletes each element.
     */
    virtual void destroyAll();


    /**
     * Removes all elements from the hash map without deleting the elements
     */
    virtual void removeAll();

/* ============================ ACCESSORS ================================= */

    /**
     * Return the key for a given value or NULL if not found.
     */
    UtlContainable* findValue(const UtlContainable* key) const;


    /**
     * Return the designated key if found otherwise NULL.
     */
    virtual UtlContainable* find(const UtlContainable* key) const;

/* ============================ INQUIRY =================================== */

    /**
     * Return the total number of keys in the hash map
     */
    size_t entries() const;


    /**
     * Return true if the hash map is empty (entries() == 0), otherwise false.
     */
    UtlBoolean isEmpty() const;


    /**
     * Return true if the hash map includes an entry with the specified key.
     */
    UtlBoolean contains(const UtlContainable* key) const;
  

    /**
     * Get the ContainableType for the hash bag as a contained object.
     */
    virtual UtlContainableType getContainableType() const;

    /**
     * Make a copy of all of the items BY POINTER in (*this) instance
     * into the given map. It does not clear the given map. IF USING
     * destroyAll call, be sure to call this on only ONE map instance.
     */
    void copyInto(UtlHashMap& map) const;


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /**
     * Return the key for a given value or NULL if not found.
     */
    UtlContainable* getValue(const UtlContainable* key) const;
    /**<
     * Assumes that the caller is holding the mContainerLock
     */
    

    GHashTable* mpHashTable;


    static const UtlContainableType TYPE;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    // internal shared constructor code
    void init();
    
    // no copy constructor is provided
    UtlHashMap(UtlHashMap&);

    /** Use copyInto instead */
    UtlHashMap& operator=(const UtlHashMap&);
    
    // static callback functions for ghashtable.
    static guint callbackHash(gconstpointer v);
    static gboolean callbackEqual(gconstpointer v, gconstpointer v2);

    /**
     * notifyIteratorsOfRemove - called before removing any entry from the UtlHashMap
     */
    void notifyIteratorsOfRemove(const UtlContainable* key);
    static gboolean UtlHashMap::notifyEachRemoved(gpointer  key,gpointer  value,gpointer  user_data);

    static gboolean UtlHashMap::notifyEachDeleted(gpointer  key,gpointer  value,gpointer  user_data);

};

#endif    // _UtlHashMap_h_


