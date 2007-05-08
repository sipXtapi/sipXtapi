//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpOSSDeviceWrapperContainer.h"
#include "mp/MpOSSDeviceWrapper.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS
MpOSSDeviceWrapperContainer mOSSContainer;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpOSSDeviceWrapperContainer::MpOSSDeviceWrapperContainer()
: mMutex(OsMutex::Q_FIFO)
{
}

// Destructor
MpOSSDeviceWrapperContainer::~MpOSSDeviceWrapperContainer()
{
   for (;;)
   {
      UtlHashMapIterator itor(*this);
      UtlString* pDevName;

      if ((pDevName = static_cast<UtlString*>(itor())))
      {
         MpOSSDeviceWrapper *pDev =
                          static_cast<MpOSSDeviceWrapper*>(findValue(pDevName));
         if (pDev)
         {
            excludeFromContainer(pDev);
         }
      }
      else
      {
         break;
      }
   }
}


void MpOSSDeviceWrapperContainer::excludeFromContainer(MpOSSDeviceWrapper* pDevWrap)
{
   OsLock lock(mMutex);

   UtlHashMapIterator itor(*this);
   UtlString* pDevName;

   while ((pDevName = static_cast<UtlString*>(itor())))
   {
      if (static_cast<MpOSSDeviceWrapper*>(itor.value()) == pDevWrap)
      {
         break;
      }
   }

   assert (pDevName != NULL);

   UtlContainable* pDevTmp;
   UtlString* pRetName;

   pRetName = static_cast<UtlString*>(removeKeyAndValue(pDevName, pDevTmp));
   delete pRetName;

   MpOSSDeviceWrapper *pDev = static_cast<MpOSSDeviceWrapper*>(pDevTmp);
   delete pDev;
}

MpOSSDeviceWrapper* MpOSSDeviceWrapperContainer::getOSSDeviceWrapper(const UtlString& ossdev)
{
   OsLock lock(mMutex);

   MpOSSDeviceWrapper *pDev;
   UtlString* pName;

   pDev = static_cast<MpOSSDeviceWrapper*>(findValue(&ossdev));

   if (pDev == NULL)
   {
      pName = new UtlString(ossdev);
      pDev = new MpOSSDeviceWrapper;
      insertKeyAndValue(pName, pDev);
   }

   return pDev;
}
