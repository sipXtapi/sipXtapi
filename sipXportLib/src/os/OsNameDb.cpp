// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES

// Keep OsNameDbInit.h as the first include!
// See OsNameDbInit class description for more information.
#include "os/OsNameDbInit.h"

#include "os/OsNameDb.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "utl/UtlInt.h"
#include "utl/UtlString.h"

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES

// CONSTANTS
static const int DEFAULT_NAMEDB_SIZE = 100;

// STATIC VARIABLE INITIALIZATIONS
OsNameDb* OsNameDb::spInstance = NULL;
OsBSem*   OsNameDb::spLock = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the singleton object, creating it if necessary
OsNameDb* OsNameDb::getNameDb(void)
{
   spLock->acquire();
   if (spInstance == NULL) // not created while getting lock?
   {
      spInstance = new OsNameDb();
   }
   spLock->release();
   
   return spInstance;
}

void OsNameDb::release(void)
{
   spLock->acquire();
   if (spInstance != NULL) // not created while getting lock?
   {
      delete spInstance ;
   }
   spLock->release();
}


// Destructor
// Since the name database is a singleton object, this destructor should
// not get called unless we are shutting down the system.
OsNameDb::~OsNameDb()
{
   mDict.destroyAll() ;
   spInstance = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Add the key-value pair to the name database
// Return OS_SUCCESS if successful, OS_NAME_IN_USE if the key is already in
// the database.
OsStatus OsNameDb::insert(const UtlString& rKey,
                          const int value)
{
   OsWriteLock          lock(mRWLock);
   UtlString* pDictKey;
   UtlInt*    pDictValue;
   UtlString* pInsertedKey;

   pDictKey   = new UtlString(rKey);
   pDictValue = new UtlInt(value);
   pInsertedKey = (UtlString*)
                  mDict.insertKeyAndValue(pDictKey, pDictValue);

   if (pInsertedKey == NULL)
   {                             // insert failed
      delete pDictKey;           // clean up the key and value objects
      delete pDictValue;

      return OS_NAME_IN_USE;
   }
   else
   {
      return OS_SUCCESS;
   }
}

// Remove the indicated key-value pair from the name database.
// If pValue is non-NULL, the value for the key-value pair is returned
// via pValue.
// Return OS_SUCCESS if the lookup is successful, return OS_NOT_FOUND
// if there is no match for the specified key.
OsStatus OsNameDb::remove(const UtlString& rKey,
                          int* pValue)
{
   OsWriteLock          lock(mRWLock);
   OsStatus   result = OS_NOT_FOUND;
   UtlString* pDictKey;
   UtlInt*    pDictValue;

   pDictKey =
      (UtlString*)
      mDict.removeKeyAndValue(&rKey, (UtlContainable*&) pDictValue);

   // If a value was found and removed ...
   if (pDictKey != NULL)
   {
      // If the caller provided a pointer through which to return the
      // integer value, do so.
      if (pValue != NULL)
      {
         *pValue = pDictValue->getValue();
      }

      // Delete the key and value objects.
      delete pDictKey;
      delete pDictValue;

      result =  OS_SUCCESS;
   }

   //  Return success or failure as appropriate.
   return result;
}
   
/* ============================ ACCESSORS ================================= */

// Retrieve the value associated with the specified key.
// If pValue is non-NULL, the value is returned via pValue.
// Return OS_SUCCESS if the lookup is successful, return OS_NOT_FOUND if
// there is no match for the specified key.
OsStatus OsNameDb::lookup(const UtlString& rKey,
                          int* pValue)
{
   OsReadLock lock(mRWLock);
   OsStatus   result = OS_NOT_FOUND;
   UtlInt*    pDictValue;

   pDictValue = (UtlInt*)
                mDict.findValue(&rKey); // perform the lookup

   if (pDictValue != NULL)
   {
      if (pValue != NULL)       // if we have a valid pointer,
      {                         //  return the corresponding value
         *pValue = pDictValue->getValue();
      }
      result = OS_SUCCESS;
   }

   return result;
}

// Return the number of key-value pairs in the name database
int OsNameDb::numEntries(void)
{
   OsReadLock lock(mRWLock);

   return mDict.entries();
}

/* ============================ INQUIRY =================================== */

// Return TRUE if the name database is empty
UtlBoolean OsNameDb::isEmpty(void)
{
   return (numEntries() == 0);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Constructor (only available internal to the class)
OsNameDb::OsNameDb() :
   mDict(),
   mRWLock(OsRWMutex::Q_PRIORITY)
{
   // since we plan to store object pointers as well as integer values in the
   //  database, we make sure the sizes are compatible
   assert(sizeof(void*) <= sizeof(int));

   // no other work required
}

/* ============================ FUNCTIONS ================================= */


