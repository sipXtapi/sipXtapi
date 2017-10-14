//
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlInit.h>
#include <mp/MpAlsaContainer.h>
#include <mp/MpAlsa.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlVoidPtr.h>
#include <os/OsLock.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
MpAlsaContainer* MpAlsaContainer::mpCont = NULL;
int MpAlsaContainer::refCount = 0;

// GLOBAL VARIABLE INITIALIZATIONS
//MpALSADeviceWrapperContainer mALSAContainer;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpAlsaContainer::MpAlsaContainer()
: mMutex(OsMutex::Q_FIFO)
{
}

// Destructor
MpAlsaContainer::~MpAlsaContainer()
{
   for (;;)
   {
      UtlHashMapIterator itor(*this);
      UtlString* pDevName;

      if ((pDevName = static_cast<UtlString*>(itor())))
      {
         MpAlsa *pDev = static_cast<MpAlsa*>(findValue(pDevName));
         if (pDev)
         {
             OsSysLog::add(FAC_MP, PRI_DEBUG,
                           "MpAlsaContainer::~MpAlsaContainer removing ALSA device: %p from container",
                           pDev);
             excludeFromContainer(pDev);
         }
      }
      else
      {
         break;
      }
   }
}


void MpAlsaContainer::excludeFromContainer(MpAlsa* pDevWrap)
{
   OsLock lock(mMutex);

   UtlHashMapIterator itor(*this);
   UtlString* pDevName;

   while ((pDevName = static_cast<UtlString*>(itor())))
   {
      if (static_cast<MpAlsa*>(itor.value()) == pDevWrap)
      {
         break;
      }
   }

   // I think this ok if not found.  I think that it is possible
   // for MpidAlsa and MpodAlsa to both disable, the second of which
   // should remove the MpAlsa from the container.  Then later in the
   // MpAlsa destructor, it may check again to remove the MpAlsa/itself.
   //assert (pDevName != NULL);
   if(pDevName != NULL)
   {
       UtlContainable* pDevTmp;
       UtlString* pRetName;

       pRetName = static_cast<UtlString*>(removeKeyAndValue(pDevName, pDevTmp));
       delete pRetName;

       MpAlsa *pDev = static_cast<MpAlsa*>(pDevTmp);
       delete pDev;
   }
}

MpAlsa* MpAlsaContainer::getALSADeviceWrapper(const UtlString& ossdev)
{
   OsLock lock(mMutex);

   MpAlsa *pDev;
   UtlString* pName;

   pDev = static_cast<MpAlsa*>(findValue(&ossdev));

   if (pDev == NULL)
   {
      pName = new UtlString(ossdev);
      pDev = new MpAlsa;
      insertKeyAndValue(pName, pDev);
   }

   return pDev;
}

UtlBoolean MpAlsaContainer::excludeWrapperFromContainer(MpAlsa* pDev)
{
   if (refCount == 0)
   {
      assert(mpCont != NULL);
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MpAlsaContainer::excludeWrapperFromContainer removing ALSA device: %p from container",
                    pDev);
      mpCont->excludeFromContainer(pDev);

      return TRUE;
   }

   return FALSE;
}

/* ============================ ACCESSORS ================================= */

MpAlsaContainer* MpAlsaContainer::getContainer()
{
   if (refCount == 0)
   {
      assert(mpCont == NULL);
      mpCont = new MpAlsaContainer;
   }
   refCount++;

   return mpCont;
}

void MpAlsaContainer::releaseContainer(MpAlsaContainer* pCont)
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

