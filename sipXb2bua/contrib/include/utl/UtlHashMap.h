//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlHashMap_h_
#define _UtlHashMap_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlContainer.h"

// DEFINES
// MACROS
#define NUM_HASHMAP_BUCKETS(bits) (1<<bits)

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;
class UtlPair;

/**
 * UtlHashMap is a container object that allows you to store keys and 
 * values.  Key must be unique (testing for equality using the
 * UtlContainer::isEquals(...) method).
 */
class UtlHashMap : public UtlContainer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Default Constructor
     */
    UtlHashMap();

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
     * Return the value for a given key or NULL if not found.
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

    /// The current number of buckets in the hash.
    size_t numberOfBuckets() const
       {
          return NUM_HASHMAP_BUCKETS(mBucketBits);
       }
    

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlHashMapIterator;

    static const UtlContainable* INTERNAL_NULL;

    /// If the Hash is too full, add additional buckets.
    /**
     * Assumes that the caller is holding the mContainerLock.
     *
     * This calls resize to actually do the resize if it is safe.
     */
    void resizeIfNeededAndSafe()
       {
          if (   ( mElements / NUM_HASHMAP_BUCKETS(mBucketBits) >= 3 ) // mean bucket 3 or more
              && ( mIteratorList.isUnLinked() )   /* there are no iterators -
                                                   * resizing moves elements to new buckets,
                                                   * which could cause an iterator to miss some
                                                   * and to return others more than once.
                                                   */
              )
          {
             resize();
          }
       }
    
    size_t    mElements;   ///< number of UtlContainable objects in this UtlHashMap
    size_t    mBucketBits; ///< number of bits used to index the buckets
    UtlChain* mpBucket;    ///< an array of 2**n UtlChain elements, each used as a list header.

    static    UtlChainPool* spPairPool; ///< pool of available UtlPair objects.
    
    static const UtlContainableType TYPE;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Allocate additional buckets and redistribute existing contents.
    void resize();
    /**
     * This should only be called through resizeIfNeededAndSafe.
     */          

    /// Search for a given key value and return the the UtlPair and bucket for it.
    bool lookup(const UtlContainable* key, ///< The key to locate.
                UtlChain*&      bucket,    /**< The bucket list header in which it belongs.
                                            *   This is set regardless of whether or not the
                                            *   key was found in the table. */
                UtlPair*&       pair       /**< If the key was found, the UtlPair for the entry.
                                            *   If the key was not found, this is NULL. */
                ) const;
    /**<
     * @return true if the key was found, and false if not.
     */
    
    /// Insert a pair into a bucket.
    void insert(UtlPair*        pair,   /**< The UtlPair for the entry - data, value, and hash
                                         *   are already set. */
                UtlChain*       bucket  ///< The bucket list header where the entry belongs.
                );

    /// Calculate the bucket number for a given hash.
    size_t bucketNumber(unsigned hash) const;
    
    // no copy constructor is provided
    UtlHashMap(UtlHashMap&);

    /** Use copyInto instead */
    UtlHashMap& operator=(const UtlHashMap&);
    
    /**
     * notifyIteratorsOfRemove - called before removing any entry from the UtlHashMap
     */
    void notifyIteratorsOfRemove(const UtlPair* pair);
};

#endif    // _UtlHashMap_h_


