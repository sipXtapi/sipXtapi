//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "utl/UtlContainable.h"
#include "utl/UtlInt.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlHashMap.h"
#include "os/OsLock.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlHashMap::TYPE = "UtlHashMap";

// STATIC VARIABLE INITIALIZATIONS

static const UtlInt INTERNAL_NULL_OBJECT(666);
static const UtlContainable* INTERNAL_NULL = &INTERNAL_NULL_OBJECT;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default Constructor
UtlHashMap::UtlHashMap()
{
   init();
}


// Constructor 
UtlHashMap::UtlHashMap(int N, UtlBoolean prealloc)
{
   // we don't actually use these variables - prevent compiler warnings
   N = N;
   prealloc = prealloc;
   
   init();
}

void UtlHashMap::init()
{
   mpHashTable = g_hash_table_new(callbackHash,callbackEqual);
}

// Destructor
UtlHashMap::~UtlHashMap()
{
   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerLock);
      
   invalidateIterators();

   UtlContainer::releaseIteratorConnectionLock();

   g_hash_table_destroy(mpHashTable);
   mpHashTable=NULL;
}

/* ============================ MANIPULATORS ============================== */

UtlContainable* UtlHashMap::remove(UtlContainable* key) 
{
   UtlContainable* unusedValue;
   
   return removeKeyAndValue(key, unusedValue);
}


UtlContainable* UtlHashMap::removeReference(const UtlContainable* key) 
{
   UtlContainable* unusedValue;
   
   // Locking is done by removeKeyAndValue().

   return removeKeyAndValue(key, unusedValue);
}



UtlBoolean UtlHashMap::destroy(UtlContainable* key) 
{
   UtlBoolean wasRemoved = FALSE;
   UtlContainable* value;

   // Locking is done by removeKeyAndValue().

   UtlContainable* removedKey = removeKeyAndValue(key, value);
   
   if(removedKey)
   {
      wasRemoved = TRUE;
      delete removedKey;
      if (value != INTERNAL_NULL)
      {
         delete value;
      }
   }

   return wasRemoved;
}


void UtlHashMap::removeAll() 
{
   OsLock take(mContainerLock);
   
   g_hash_table_foreach_remove(mpHashTable, notifyEachRemoved, this);
}


void UtlHashMap::destroyAll() 
{
   OsLock take(mContainerLock);
   
   // notifyEachDeleted() will delete the keys and values.
   g_hash_table_foreach_remove(mpHashTable, notifyEachDeleted, this);
}

   
// insert a key with a NULL value
UtlContainable* UtlHashMap::insert(UtlContainable* obj)
{
   // Locking will be done by insertKeyAndValue().

   return insertKeyAndValue(obj, NULL);
}

UtlContainable* UtlHashMap::insertKeyAndValue(UtlContainable* key, UtlContainable* value) 
{
   UtlContainable* insertedKey = NULL;

   if (!value)
   {
      value = const_cast<UtlContainable*>(INTERNAL_NULL);
   }
   
   if (key && value) // NULL keys and values are not allowed
   {
      OsLock take(mContainerLock);   

      if(g_hash_table_lookup(mpHashTable, key) == NULL)
      {
         g_hash_table_insert(mpHashTable, key, value);
         insertedKey = key;
      }
   }
   return insertedKey;
}

UtlContainable* UtlHashMap::removeKeyAndValue(const UtlContainable* key, UtlContainable*& value)    
{
   UtlContainable* removed = NULL;
   value = NULL;
   
   if (key)
   {
      gpointer returnedValue;
      gpointer returnedKey;
      
      OsLock take(mContainerLock);

      if(g_hash_table_lookup_extended(mpHashTable, key, &returnedKey, &returnedValue))
      {
         removed = (UtlContainable*)returnedKey;
         if ((UtlContainable*)returnedValue != INTERNAL_NULL)
         {
            value = (UtlContainable*)returnedValue;
         }
         
         notifyIteratorsOfRemove(removed);
         
         g_hash_table_remove(mpHashTable,returnedKey);
      }
   }
   
   return removed;
}


void UtlHashMap::copyInto(UtlHashMap& into) const
{
    UtlHashMapIterator i(*this);
    while (i() != NULL)
    {
       into.insertKeyAndValue(i.key(), i.value());
    }
}


/* ============================ ACCESSORS ================================= */

UtlContainable* UtlHashMap::findValue(const UtlContainable* key) const 
{
   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   return getValue(key);
}


UtlContainable* UtlHashMap::find(const UtlContainable* key) const
{   
   gpointer foundKey;
   gpointer unusedValue;
   
   OsLock take(const_cast<OsBSem&>(mContainerLock));   

   if (!g_hash_table_lookup_extended(mpHashTable, key, &foundKey, &unusedValue))
   {
      foundKey = NULL; 
   }

   return (UtlContainable*)foundKey;
}


/* ============================ INQUIRY =================================== */

size_t UtlHashMap::entries() const 
{
   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   return g_hash_table_size(mpHashTable); 
}


UtlBoolean UtlHashMap::isEmpty() const 
{
   return entries() == 0; 
}


UtlBoolean UtlHashMap::contains(const UtlContainable* key)  const 
{
   return find(key) != NULL;
}


/**
 * Get the ContainableType for the hash map as a contained object.
 */
UtlContainableType UtlHashMap::getContainableType() const
{
   return UtlHashMap::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlContainable* UtlHashMap::getValue(const UtlContainable* key) const
{
   // caller is holding the mContainerLock

   UtlContainable* value = (UtlContainable*)g_hash_table_lookup(mpHashTable, key);

   return (value != INTERNAL_NULL) ? value : NULL;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// static callback functions for ghashtable.
guint UtlHashMap::callbackHash(gconstpointer obj)
{
   UtlContainable* key = (UtlContainable*) obj;

   return (guint)key->hash();
}


gboolean UtlHashMap::callbackEqual(gconstpointer obj1, gconstpointer obj2)
{
   UtlContainable* key1 = (UtlContainable*) obj1;
   UtlContainable* key2 = (UtlContainable*) obj2;

   return key1->compareTo(key2) == 0;
}


gboolean UtlHashMap::notifyEachRemoved(gpointer key, gpointer value, gpointer user_data)
{
   UtlHashMap* I = (UtlHashMap*)user_data; // 'this' passed through C library
   
   I->notifyIteratorsOfRemove((UtlContainable*)key);
   
   return TRUE;
}


gboolean UtlHashMap::notifyEachDeleted(gpointer key, gpointer value, gpointer user_data)
{
   UtlHashMap* I = (UtlHashMap*)user_data; // 'this' passed through C library
   I->notifyIteratorsOfRemove((UtlContainable*)key);
   
   delete (UtlContainable*)key;
   if ((UtlContainable*)value != INTERNAL_NULL)
   {
      delete (UtlContainable*)value;
   }
   
   return TRUE;
}

void UtlHashMap::notifyIteratorsOfRemove(const UtlContainable* key)
{
   GList* listNode;
   UtlHashMapIterator* foundIterator;
   
   for (listNode = g_list_first(mpIteratorList); listNode; listNode = g_list_next(listNode))
   {
      foundIterator = (UtlHashMapIterator*)listNode->data;
      foundIterator->removing(key);
   }
}


/* ============================ FUNCTIONS ================================= */
