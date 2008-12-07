//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlInt.h"
#include "utl/UtlContainablePair.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType UtlContainablePair::TYPE = "UtlContainablePair";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

UtlContainablePair::UtlContainablePair(UtlContainable *pFirst, UtlContainable *pSecond)
: mpFirst(pFirst)
, mpSecond(pSecond)
{
}

UtlContainablePair::~UtlContainablePair()
{
   delete mpFirst;
   delete mpSecond;
}

/* ============================ MANIPULATORS ============================== */

UtlContainable* UtlContainablePair::setFirst(UtlContainable *val)
{
    UtlContainable* pOldValue = mpFirst;
    mpFirst = val;
    return pOldValue;
}

UtlContainable* UtlContainablePair::setSecond(UtlContainable *val)
{
   UtlContainable* pOldValue = mpSecond;
   mpSecond = val;
   return pOldValue;
}

/* ============================ ACCESSORS ================================= */

UtlContainable* UtlContainablePair::getFirst() const 
{
    return mpFirst; 
}

UtlContainable* UtlContainablePair::getSecond() const 
{
   return mpSecond; 
}

unsigned UtlContainablePair::hash() const
{
    return mpFirst->hash() ^ mpSecond->hash();
}


UtlContainableType UtlContainablePair::getContainableType() const
{
    return UtlContainablePair::TYPE;
}

/* ============================ INQUIRY =================================== */

int UtlContainablePair::compareTo(UtlContainable const *inVal) const
{
   int result; 

   assert(mpFirst != NULL && mpSecond != NULL);
   if (inVal->isInstanceOf(UtlContainablePair::TYPE))
   {
      UtlContainablePair *pOther = (UtlContainablePair*)inVal;
      assert(pOther->mpFirst != NULL && pOther->mpSecond != NULL);
      result = mpFirst->compareTo(pOther->mpFirst);
      if (result != 0)
      {
         result = mpSecond->compareTo(pOther->mpSecond);
      }
   }
   else
   {
      result = -1; 
   }

   return result;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
