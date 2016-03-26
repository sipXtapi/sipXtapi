//  
// Copyright (C) 2008-2011 SIPez LLC. All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
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
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
#define NUM_HASHMAP_BUCKETS(bits) (1ull<<bits)
#else
#define NUM_HASHMAP_BUCKETS(bits) (1<<bits)
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;
class UtlPair;

/**
*  UtlHashMap is a container object that allows you to store keys and values.
*
*  Key must be unique (testing for equality using the UtlContainer::isEquals()
*  method).
*/
class UtlHashMap : public UtlContainer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    static const UtlContainableType TYPE;

/* ============================ CREATORS ================================== */

      /// Default Constructor
    UtlHashMap();

      /// Destructor
    virtual ~UtlHashMap();

    /// @copydoc UtlCopyableContainable::clone
    virtual UtlHashMap* clone() const;

/* ============================ MANIPULATORS ============================== */

      /// Inserts a key and value pair into the hash map.
    UtlContainable* insertKeyAndValue(UtlContainable* key, UtlContainable* value);
      /**<
      *  If the inserted key is already in the table, this method 
      *  fails (returns NULL - note that this means if value is NULL,
      *  then you can't tell whether  there was an error or not).
      *  To replace the value for a given key, the old value must
      *  be Removed before the new value is inserted.
      *
      *  @return the key on success, otherwise NULL
      */


      /// @brief Inserts the designated containable object into the list
      /// with a NULL value.
    UtlContainable* insert(UtlContainable* obj);
      /**<
      *  If there is an equal key in the UtlHashMap already,
      *  the insert will fail.
      *
      *  @see note regarding use of NULL value in insertKeyAndValue().
      *
      *  @return the object if successful, otherwise NULL
      */


      /// Remove the designated key and its associated value.
    UtlContainable* remove(UtlContainable* key);
      /**<
      *  @return the key or NULL if not found
      */


      /// Remove the designated key and its associated value.
    UtlContainable* removeReference(const UtlContainable* key);
      /**<
      *  @return the key or NULL if not found
      */
    
      /// Remove the designated key and its associated value.
    UtlContainable* removeKeyAndValue(const UtlContainable* key, UtlContainable*& value);
      /**<
      *  The pointer of value is returned as part of the call if successful.
      *
      *  @return the key or NULL if not found
      */


      /// @brief Removes the designated key and its associated value from the map
      /// and frees the key and the value (if not NULL) by calling delete.
    virtual UtlBoolean destroy(UtlContainable* key);    
      /**<
      *  @param pointer to a UtlContainable for which key->operator==(the actual key)
      *
      *  @return the key or NULL if not found
      */


      /// Removes all elements from the hash map and deletes each element.
    virtual void destroyAll();


      /// Removes all elements from the hash map without deleting the elements
    virtual void removeAll();

/* ============================ ACCESSORS ================================= */

      /// Return the value for a given key or NULL if not found.
    UtlContainable* findValue(const UtlContainable* key) const;


      /// Return the designated key if found otherwise NULL.
    virtual UtlContainable* find(const UtlContainable* key) const;

/* ============================ INQUIRY =================================== */

      /// Return the total number of keys in the hash map
    size_t entries() const;


      /// Return true if the hash map is empty (entries() == 0), otherwise false.
    UtlBoolean isEmpty() const;


      /// Return true if the hash map includes an entry with the specified key.
    UtlBoolean contains(const UtlContainable* key) const;
  

      /// Get the ContainableType for the hash bag as a contained object.
    virtual UtlContainableType getContainableType() const;

      /// @brief Make a copy of all of the items BY POINTER in (*this) instance
      /// into the given map.
    void copyInto(UtlHashMap& map) const;
      /**<
      *  It does not clear the given map. IF USING destroyAll call, be sure
      *  to call this on only ONE map instance.
      */

    OsStatus deepCopyInto(UtlHashMap& map) const;
      /**<
       *  This does a deep copy of the object in this UtlHashMap.
       *  WARNING: buyer beware, all objects (both keys and values)
       *  MUST be derived from UtlCopyableContainable or this method will
       *  fail.  The clone method is invoked on both keys and values to
       *  create copies into the given map.  The given map does not need
       *  to be empty, but if the key already exists, the copy will fail.
       *
       *  @param map - target UtlHashMap to copy key/value pairs to.
       *
       *  @returns OS_NAME_IN_USE if key already exists in map
       *           OS_NOT_SUPPORTED if any key or object is not derived from UtlCopyableContainable
       *
       */

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
    inline
    void resizeIfNeededAndSafe();
      /**<
      * Assumes that the caller is holding the mContainerLock.
      *
      * This calls resize to actually do the resize if it is safe.
      */
    
    size_t    mElements;   ///< number of UtlContainable objects in this UtlHashMap
    size_t    mBucketBits; ///< number of bits used to index the buckets
    UtlChain* mpBucket;    ///< an array of 2**n UtlChain elements, each used as a list header.
    
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Allocate additional buckets and redistribute existing contents.
    void resize();
      /**<
      *  This should only be called through resizeIfNeededAndSafe.
      */          

      /// Search for a given key value and return the the UtlPair and bucket for it.
    bool lookup(const UtlContainable* key,
                UtlChain*&      bucket,
                UtlPair*&       pair) const;
      /**<
      *  @param[in]  key - The key to locate.
      *  @param[out] bucket - The bucket list header in which it belongs.
      *              This is set regardless of whether or not the
      *              key was found in the table.
      *  @param[out] pair - If the key was found, the UtlPair for the entry.
      *              If the key was not found, this is NULL
      *  @return true if the key was found, and false if not.
      */
    
      /// Insert a pair into a bucket.
    void insert(UtlPair*        pair,
                UtlChain*       bucket);
      /**<
      *  @param[in] pair - The UtlPair for the entry - data, value, and hash
      *             are already set.
      *  @param[in] bucket - The bucket list header where the entry belongs.
      */

      /// Calculate the bucket number for a given hash.
    size_t bucketNumber(unsigned hash) const;
    
      /// No copy constructor is provided.
    UtlHashMap(UtlHashMap&);

      /// Use copyInto instead
    UtlHashMap& operator=(const UtlHashMap&);
    
      /// Called before removing any entry from the UtlHashMap
    void notifyIteratorsOfRemove(const UtlPair* pair);
};

void UtlHashMap::resizeIfNeededAndSafe()
{
   // IF mean bucket 3 or more AND there are no iterators.
   // Resizing moves elements to new buckets, which could cause an iterator
   // to miss some and to return others more than once.
   if (   ( mElements / NUM_HASHMAP_BUCKETS(mBucketBits) >= 3 ) 
       && ( mIteratorList.isUnLinked() ))
   {
      resize();
   }
}

#endif    // _UtlHashMap_h_
