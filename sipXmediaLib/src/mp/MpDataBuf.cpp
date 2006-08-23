//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpDataBuf.h"
#include "mp/MpBufPool.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
MpBufPool *MpDataBuf::smpDefaultPool = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

void MpDataBuf::sDestroy(MpBuf *pBuffer)
{
   ((MpDataBuf*)pBuffer)->mpData.release();
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

char *MpDataBuf::getDataPtr()
{
   // We use this to delegate all work to const version of function
   return (char*)((const MpDataBuf *)this)->getDataPtr();
};

const char *MpDataBuf::getDataPtr() const
{
   if (mpData.isValid())
      return mpData->getDataPtr();
   else
      return NULL;
};

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpDataBuf::init()
{
   // mpData must be invalid pointer. Else it will be considered as allocated
   // buffer and in attempt to free it random memory will be corrupted.
//   assert(!mpData.isValid());
   (*(MpArrayBuf**)(&mpData)) = NULL;

   // Update virtual functions to MpDataBuf versions.
   mpDestroy = MpDataBuf::sDestroy;
   mpInitClone = MpDataBuf::sInitClone;

#ifdef MPBUF_DEBUG
   osPrintf(">>> MpDataBuf::init()\n");
#endif
}

void MpDataBuf::sInitClone(MpBuf *pBuffer)
{
   // First pass initialization to parent..
   MpBuf::sInitClone(pBuffer);

   // Get clone of the data
   MpArrayBufPtr pDataClone = ((MpDataBuf*)pBuffer)->mpData.makeClone();

   // This prevents mpData from being freed here:
   ((MpDataBuf*)pBuffer)->mpData.swap(pDataClone);
   (*(MpArrayBuf**)(&pDataClone)) = NULL;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
