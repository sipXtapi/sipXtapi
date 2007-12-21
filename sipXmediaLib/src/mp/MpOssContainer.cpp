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
#include "utl/UtlInit.h"
#include "mp/MpOssContainer.h"
#include "mp/MpOss.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
MpOssContainer* MpOssContainer::mpCont = NULL;
int MpOssContainer::refCount = 0;

// GLOBAL VARIABLE INITIALIZATIONS
//MpOSSDeviceWrapperContainer mOSSContainer;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpOssContainer::MpOssContainer()
: mMutex(OsMutex::Q_FIFO)
{
}

// Destructor
MpOssContainer::~MpOssContainer()
{
   for (;;)
   {
      UtlHashMapIterator itor(*this);
      UtlString* pDevName;

      if ((pDevName = static_cast<UtlString*>(itor())))
      {
         MpOss *pDev = static_cast<MpOss*>(findValue(pDevName));
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


void MpOssContainer::excludeFromContainer(MpOss* pDevWrap)
{
   OsLock lock(mMutex);

   UtlHashMapIterator itor(*this);
   UtlString* pDevName;

   while ((pDevName = static_cast<UtlString*>(itor())))
   {
      if (static_cast<MpOss*>(itor.value()) == pDevWrap)
      {
         break;
      }
   }

   assert (pDevName != NULL);

   UtlContainable* pDevTmp;
   UtlString* pRetName;

   pRetName = static_cast<UtlString*>(removeKeyAndValue(pDevName, pDevTmp));
   delete pRetName;

   MpOss *pDev = static_cast<MpOss*>(pDevTmp);
   delete pDev;
}

MpOss* MpOssContainer::getOSSDeviceWrapper(const UtlString& ossdev)
{
   OsLock lock(mMutex);

   MpOss *pDev;
   UtlString* pName;

   pDev = static_cast<MpOss*>(findValue(&ossdev));

   if (pDev == NULL)
   {
      pName = new UtlString(ossdev);
      pDev = new MpOss;
      insertKeyAndValue(pName, pDev);
   }

   return pDev;
}

UtlBoolean MpOssContainer::excludeWrapperFromContainer(MpOss* pDev)
{
   if (refCount == 0)
   {
      assert(mpCont != NULL);
      mpCont->excludeFromContainer(pDev);

      return TRUE;
   }

   return FALSE;
}

/* ============================ ACCESSORS ================================= */

MpOssContainer* MpOssContainer::getContainer()
{
   if (refCount == 0)
   {
      assert(mpCont == NULL);
      mpCont = new MpOssContainer;
   }
   refCount++;

   return mpCont;
}

void MpOssContainer::releaseContainer(MpOssContainer* pCont)
{
   assert(refCount > 0);
   assert(mpCont == pCont);

   refCount--;
   if (refCount == 0)
   {
      delete mpCont;
      mpCont = NULL;
   }
}

