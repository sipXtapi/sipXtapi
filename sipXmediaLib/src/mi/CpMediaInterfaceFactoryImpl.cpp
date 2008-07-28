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
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
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

// APPLICATION INCLUDES
#include "mi/CpMediaInterfaceFactoryImpl.h"
#include "os/OsLock.h"
#include "os/OsDatagramSocket.h"
#include "os/OsServerSocket.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "utl/UtlString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DEFAULT_NUM_CODEC_PATHS 10;
// STATIC VARIABLE INITIALIZATIONS
size_t CpMediaInterfaceFactoryImpl::mnCodecPaths = 0;
size_t CpMediaInterfaceFactoryImpl::mnAllocCodecPaths = 0;
UtlString* CpMediaInterfaceFactoryImpl::mpCodecPaths = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInterfaceFactoryImpl::CpMediaInterfaceFactoryImpl()
    : CpMediaDeviceMgr()
{
    miStartRtpPort = 0 ;
    miLastRtpPort = 0;
    miNextRtpPort = miStartRtpPort ;
}

// Destructor
CpMediaInterfaceFactoryImpl::~CpMediaInterfaceFactoryImpl()
{
    OsLock lock(mlockList) ;
    
    mlistFreePorts.destroyAll() ;
    mlistBusyPorts.destroyAll() ;
}

/* =========================== DESTRUCTORS ================================ */

/**
 * public interface for destroying this media interface
 */ 
void CpMediaInterfaceFactoryImpl::release()
{
   delete this;
}

/* ============================ MANIPULATORS ============================== */

// Static method to clear all codec paths
void CpMediaInterfaceFactoryImpl::clearCodecPaths()
{
   if(mpCodecPaths != NULL)
   {
      delete[] mpCodecPaths;
   }
   mpCodecPaths = NULL;
   mnAllocCodecPaths = 0;
   mnCodecPaths = 0;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


OsStatus CpMediaInterfaceFactoryImpl::ensureCapacityCodecPaths(size_t newSize)
{
   if (newSize <= mnAllocCodecPaths)
   {
      return OS_SUCCESS;
   }

   size_t newArraySz = mnAllocCodecPaths;

   // If this is the first time something has been added, 
   // then we need to give an initial size.
   if (newArraySz == 0)
   {
      newArraySz = DEFAULT_NUM_CODEC_PATHS;
   }

   if (newSize <= SIZE_MAX/2)
   {
      // if the requested new size is less than half the maximum capacity,
      // then it's safe to keep doubling our current allocation size until it 
      // is bigger than the requested new size, so this doesn't keep getting 
      // re-allocated.
      // (it's safe at least from the perspective of the maximum size that 
      //  size_t can hold)
      while (newArraySz <= newSize)
      {
         newArraySz *= 2;
      }
   }
   else
   {
      // If we're beyond half the maximum possible capacity, 
      // then just set the capacity to whatever the new requested size is,
      // without any reserve space.
      newArraySz = newSize;
   }

   // Allocate the new codec path array.
   // If we run out of memory on windows, the app actually crashes unfortunately,
   // and the NULL return actually never happens.
   UtlString* newArray = new UtlString[newArraySz];

   // If allocation failed, return OS_NO_MEMORY
   if(newArray == NULL)
   {
      return OS_NO_MEMORY;
   }

   // And copy over the old array values to the new array
   size_t i;
   for (i = 0; i < mnCodecPaths; i++)
   {
      newArray[i] = mpCodecPaths[i];
   }

   if(mpCodecPaths != NULL)
   {
      // Now delete the old codec paths array,
      delete[] mpCodecPaths;
   }

   // and copy over the new array pointer to the old array pointer var.
   mpCodecPaths = newArray;
   mnAllocCodecPaths = newArraySz;

   return OS_SUCCESS;
}

// Static method to add codec paths
OsStatus CpMediaInterfaceFactoryImpl::addCodecPaths(const size_t nCodecPaths, 
                                                    const UtlString codecPaths[])
{
   size_t i;

   // Check each codecPath to see if it is valid.  If any of them are invalid,
   // return failure.
   /*
   for(i = 0; i < nCodecPaths; i++)
   {
      // This isn't correct..  We want to check if it's *syntactically* valid
      // not checking for existence.
      if(!OsPath(codecPaths[i]).isValid())
      {
         return OS_FAILED;
      }
   }
   */

   if(ensureCapacityCodecPaths(mnCodecPaths+nCodecPaths) == OS_FAILED)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "CpMediaInterfaceFactory::addCodecPaths "
                    " - Error ensuring static codec path capacity when adding "
                    "%d codec paths to %d already existing paths!", 
                    nCodecPaths, mnCodecPaths);
      return OS_FAILED;
   }

   // Add the new codecs to the static codec path list.
   for (i = 0; i < nCodecPaths; i++)
   {
      mpCodecPaths[mnCodecPaths+i] = codecPaths[i];
   }
   // Now increase the indicated number of codecs stored by what we added.
   mnCodecPaths += nCodecPaths;
   
   return OS_SUCCESS;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



