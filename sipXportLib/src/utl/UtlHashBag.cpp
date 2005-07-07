//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlContainable.h"
#include "utl/UtlHashBag.h"
#include "utl/UtlHashBagIterator.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlHashBag::TYPE = "UtlHashBag";

// STATIC VARIABLE INITIALIZATIONS

/**
 * Design Notes
 *
 * UtlHashBag is implemented using a GHashTable; the key is the object,
 * and the value is a GList* that points to a list of like elements.
 *
 */

/* //////////////////////////// PUBLIC /////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

UtlHashBag::UtlHashBag()
   : mEntries(0)
{
   mpHashTable = g_hash_table_new(utlObjectHash,utlObjectEqual);
}


// Destructor
UtlHashBag::~UtlHashBag()
{
   OsLock take(mContainerLock);

   if ( mpHashTable )
   {
      g_hash_table_destroy(mpHashTable);
      mpHashTable=NULL;
   }
   mEntries=0;
}

/* ============================ MANIPULATORS ============================== */


UtlContainable* UtlHashBag::insert(UtlContainable* insertedContainable)
{
   GList* instances = NULL;

   if(insertedContainable)
   {
      OsLock take(mContainerLock);

      instances = (GList*)g_hash_table_lookup(mpHashTable, insertedContainable);

      GList* newInstances = g_list_append(instances, insertedContainable);

      if (instances != newInstances)
      {
         if (instances != NULL)
         {
            UtlContainable* oldKeyObject = (UtlContainable*)instances->data;
            g_hash_table_remove(mpHashTable,oldKeyObject);
         }

         g_hash_table_insert(mpHashTable, insertedContainable, newInstances);
      }

      mEntries++;
   }

   return insertedContainable;
}


UtlContainable* UtlHashBag::remove(UtlContainable* object)
{
   UtlContainable* removed = NULL;
   GList* likeElements = NULL;

   if (object)
   {
      OsLock take(mContainerLock);

      likeElements = (GList*)g_hash_table_lookup(mpHashTable, object);

      if (likeElements)
      {
         GList* node = g_list_first(likeElements);

         removed = (UtlContainable*)node->data;
         notifyIteratorsOfRemove(node);

         // remove the element from the inner list
         likeElements = g_list_delete_link(likeElements,node);
         mEntries--;

         if (likeElements) // are there still some of this key in the bag?
         {
            // yes, so change the key to one that is still in the list.
            g_hash_table_remove(mpHashTable,removed);

            g_hash_table_insert(mpHashTable, g_list_first(likeElements)->data,
                                likeElements);
         }
         else
         {
            // all like elements have been removed, so take this
            // entry out of the hash table.
            g_hash_table_remove(mpHashTable,removed);
         }
      }
   }

   return removed;
}

/**
 * Removed the designated object by reference
 * (as opposed to searching for an equality match).
 *
 * @return the object if successful, otherwise null
 */
UtlContainable* UtlHashBag::removeReference(const UtlContainable* object)
{
   UtlContainable* removed = NULL;
   GList* likeElements = NULL;

   if (object)
   {
      OsLock take(mContainerLock);

      likeElements = (GList*)g_hash_table_lookup(mpHashTable, object);

      if (likeElements)
      {
         GList* foundNode;
         GList* node;
         for ( foundNode = NULL, node = g_list_first(likeElements);
               foundNode == NULL && node != NULL;
               node = g_list_next(node)
              )
         {
            if ( object == (UtlContainable*)node->data )
            {
               foundNode = node;
            }
         }

         if ( foundNode )
         {
            removed = (UtlContainable*)foundNode->data;;
            notifyIteratorsOfRemove(foundNode);

            // remove the element from the inner list
            likeElements = g_list_delete_link(likeElements,foundNode);
            mEntries--;

            if (likeElements) // are there still some of this key in the bag?
            {
               // yes, so change the key to one that is still in the list.
               g_hash_table_remove(mpHashTable,removed);

               g_hash_table_insert(mpHashTable,g_list_first(likeElements)->data,likeElements);
            }
            else
            {
               // all like elements have been removed, so take this
               // entry out of the hash table.
               g_hash_table_remove(mpHashTable,removed);
            }
         }
      }
   }

   return removed;
}


UtlBoolean UtlHashBag::destroy(UtlContainable* object)
{
   UtlBoolean deletedAnObject = FALSE;

   // no need to take locks... all the changes are inside remove
   UtlContainable* wasRemoved = remove(object);

   if(wasRemoved)
   {
      delete wasRemoved;
      deletedAnObject = TRUE;
   }

   return deletedAnObject;
}


void UtlHashBag::removeAll()
{
   OsLock take(mContainerLock);

   g_hash_table_foreach_remove(mpHashTable, clearAndNotifyEachRemoved, this);
   mEntries = 0;
}



void UtlHashBag::destroyAll()
{
   OsLock take(mContainerLock);

   g_hash_table_foreach_remove(mpHashTable, clearAndNotifyEachDeleted, this);
   mEntries = 0;
}


/* ============================ ACCESSORS ================================= */


UtlContainable* UtlHashBag::find(const UtlContainable* object) const
{
   UtlContainable* foundObject = NULL;

   OsLock take(const_cast<OsBSem&>(mContainerLock));

   GList* likeItems = (GList*)g_hash_table_lookup(mpHashTable, object);

   if ( likeItems )
   {
      foundObject = (UtlContainable*)(g_list_first(likeItems)->data);
   }

   return foundObject;
}


/* ============================ INQUIRY =================================== */


size_t UtlHashBag::entries() const
{
   OsLock take(const_cast<OsBSem&>(mContainerLock));

   return mEntries;
}


UtlBoolean UtlHashBag::isEmpty() const
{
   OsLock take(const_cast<OsBSem&>(mContainerLock));

   return mEntries == 0;
}


UtlBoolean UtlHashBag::contains(const UtlContainable* object)  const
{
   OsLock take(const_cast<OsBSem&>(mContainerLock));

   return g_hash_table_lookup(mpHashTable, object) != NULL;
}


/**
 * Get the ContainableType for the hash bag as a contained object.
 */
UtlContainableType UtlHashBag::getContainableType() const
{
   return UtlHashBag::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
guint UtlHashBag::utlObjectHash(gconstpointer v)
{
   UtlContainable* containable = (UtlContainable*)v;
   return (guint)containable->hash();
}

gboolean UtlHashBag::utlObjectEqual(gconstpointer v,gconstpointer v2)
{
   UtlContainable* containable1 = (UtlContainable*) v;
   UtlContainable* containable2 = (UtlContainable*) v2;

   return containable1->compareTo(containable2) == 0;
}
/* //////////////////////////// PRIVATE /////////////////////////////////// */


gboolean UtlHashBag::clearAndNotifyEachRemoved(gpointer key, gpointer value, gpointer user_data)
{
   UtlHashBag* I = (UtlHashBag*)user_data; // 'this' passed through C library

   GList* listNode;

   for ( listNode = g_list_first((GList*)value);
         listNode;
         listNode = g_list_first(listNode)
        )
   {
      I->notifyIteratorsOfRemove(listNode);
      listNode = g_list_delete_link( listNode, listNode );
   }

   return TRUE;
}


gboolean UtlHashBag::clearAndNotifyEachDeleted(gpointer key, gpointer value, gpointer user_data)
{
   UtlHashBag* I = (UtlHashBag*)user_data; // 'this' passed through C library
   GList* listNode;

   for ( listNode = g_list_first((GList*)value);
         listNode;
         listNode = g_list_first(listNode)
        )
   {
      UtlContainable* object = (UtlContainable*)listNode->data;
      I->notifyIteratorsOfRemove(listNode);
      delete object;
      listNode = g_list_delete_link( listNode, listNode );
   }

   return TRUE;
}


void UtlHashBag::notifyIteratorsOfRemove(const GList* key)
{
   GList* listNode;
   UtlHashBagIterator* foundIterator;

   for (listNode = g_list_first(mpIteratorList); listNode; listNode = g_list_next(listNode))
   {
      foundIterator = (UtlHashBagIterator*)listNode->data;
      foundIterator->removing(key);
   }
}

/* ============================ FUNCTIONS ================================= */
