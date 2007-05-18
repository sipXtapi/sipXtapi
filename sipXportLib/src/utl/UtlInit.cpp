// 
// Copyright (C) 2007 Jaroslav Libak
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlInit.h"
#include "utl/UtlLink.h"
#include "utl/UtlChainPool.h"
#include "utl/UtlContainer.h"
#include "utl/UtlListIterator.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

unsigned int UtlInit::msCount = 0;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

UtlInit::UtlInit(void)
{
   if (msCount++ == 0)
   {
      UtlContainer::spIteratorConnectionLock = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);

      UtlLink::spLinkPool = new UtlChainPool(UtlLink::allocate, UTLLINK_BLOCK_SIZE);

      UtlPair::spPairPool = new UtlChainPool(UtlPair::allocate, UTLLINK_BLOCK_SIZE);

      UtlListIterator::OFF_LIST_END = new UtlLink();
   }
}

UtlInit::~UtlInit(void)
{
   if (--msCount == 0)
   {
      delete UtlListIterator::OFF_LIST_END;
      UtlListIterator::OFF_LIST_END = NULL;

      delete UtlPair::spPairPool;
      UtlPair::spPairPool = NULL;

      delete UtlLink::spLinkPool;
      UtlLink::spLinkPool = NULL;

      delete UtlContainer::spIteratorConnectionLock;
      UtlContainer::spIteratorConnectionLock = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

