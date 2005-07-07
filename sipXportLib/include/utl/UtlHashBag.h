//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _UtlHashBag_h_
#define _UtlHashBag_h_

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
 * A UtlHashBag is a orderless container that efficiently allows for both 
 * random access and iteration.  It also allows for duplicate entries.
 */
class UtlHashBag : public UtlContainer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Constructor
    */
   UtlHashBag();

   /**
    * Destructor
    */
   virtual ~UtlHashBag(); 
/* ============================ MANIPULATORS ============================== */

   /**
    * Insert the designated object into this container.  
    * 
    * @return the given object on success otherwise null.
    */
   virtual UtlContainable* insert(UtlContainable* object);

   /**
    * Remove the first matching object from this container.  
    * 
    * @return the removed object if a match was found, otherwise NULL.
    */
   virtual UtlContainable* remove(UtlContainable* object);
    
   /**
    * Remove the designated object by reference
    * (as opposed to searching for an equality match).  
    *
    * @return the object if successful, otherwise null
    */
   virtual UtlContainable* removeReference(const UtlContainable* object);

   /**
    * Removes the first matching object from the bag and deletes the object 
    *
    * @return true if a match was found, false if not
    */ 
   virtual UtlBoolean destroy(UtlContainable* object);    

   /**
    * Removes all elements from the container and deletes each one.
    */
   virtual void destroyAll();

   /**
    * Removes all elements from the container without freeing the objects.
    */
   virtual void removeAll();

/* ============================ ACCESSORS ================================= */

   /**
    * Return the designated object if found, otherwise null.
    */
   virtual UtlContainable* find(const UtlContainable* object) const;


/* ============================ INQUIRY =================================== */


   /**
    * Return the total number of elements within the container.
    */
   size_t entries() const;

   /**
    * Return true of the container is empty (entries() == 0), otherwise false.
    */
   UtlBoolean isEmpty() const;

   /**
    * Return true if the container includes the designated object.  Each 
    * element within the list is tested for equality against the designated 
    * object using the equals() method. 
    */
   UtlBoolean contains(const UtlContainable* object) const;


   /**
    * Get the ContainableType for the hash bag as a contained object.
    */
   virtual UtlContainableType getContainableType() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   friend class UtlHashBagIterator;

   void UtlHashBag::notifyIteratorsOfRemove(const GList* key);

   static UtlContainableType TYPE;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   GHashTable* mpHashTable;
   size_t      mEntries;

   static gboolean utlObjectEqual(gconstpointer v, gconstpointer v2);

   static guint utlObjectHash(gconstpointer v);

   static gboolean UtlHashBag::clearAndNotifyEachRemoved(gpointer key, gpointer value, gpointer user_data);
   
   static gboolean UtlHashBag::clearAndNotifyEachDeleted(gpointer key, gpointer value, gpointer user_data);
   
   // Don't allow the implicit copy constructor.
   UtlHashBag(UtlHashBag&);

   UtlHashBag& operator=(UtlHashBag&);
   
};

/* ============================ INLINE METHODS ============================ */


#endif    // _UtlHashBag_h_
