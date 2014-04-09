//
// Copyright (C) 2005-2013 SIPez LLC.  All rights reserved.  
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
#include "utl/UtlRegex.h"
#include <utl/UtlSListIterator.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DEFAULT_NUM_CODEC_PATHS 10;
//#define TEST_PRINT

// STATIC VARIABLE INITIALIZATIONS
size_t CpMediaInterfaceFactoryImpl::mnCodecPaths = 0;
size_t CpMediaInterfaceFactoryImpl::mnAllocCodecPaths = 0;
UtlString* CpMediaInterfaceFactoryImpl::mpCodecPaths = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInterfaceFactoryImpl::CpMediaInterfaceFactoryImpl()
: miInterfaceId(0)
, mlockList(OsMutex::Q_FIFO)
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


void CpMediaInterfaceFactoryImpl::setRtpPortRange(int startRtpPort, int lastRtpPort) 
{
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpMediaInterfaceFactoryImpl::setRtpPortRange start=%d end=%d",
        startRtpPort, lastRtpPort);
    miStartRtpPort = startRtpPort ;
    if (miStartRtpPort < 0)
    {
        miStartRtpPort = 0 ;
    }
    miLastRtpPort = lastRtpPort ;
    miNextRtpPort = miStartRtpPort ;
}

#define MAX_PORT_CHECK_ATTEMPTS     miLastRtpPort - miStartRtpPort
#define MAX_PORT_CHECK_WAIT_MS      50
OsStatus CpMediaInterfaceFactoryImpl::getNextRtpPort(const char* bindAddress, int &rtpPort) 
{
//#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpMediaInterfaceFactoryImpl::getNextRtpPort bindAddress=%s start=%d end=%d last=%d",
        bindAddress ? bindAddress : "NULL", miStartRtpPort, miLastRtpPort, miNextRtpPort);
//#endif

    UtlString freePorts;
    UtlString busyPorts;

    OsLock lock(mlockList) ;
    bool bGoodPort = false ;
    int iAttempts = 0 ;

    // Re-add busy ports to end of free list
    while (mlistBusyPorts.entries())
    {
        UtlInt* pInt = (UtlInt*) mlistBusyPorts.first() ;
        mlistBusyPorts.remove(pInt) ;

        mlistFreePorts.append(pInt) ;
    }

    while (!bGoodPort && (iAttempts < MAX_PORT_CHECK_ATTEMPTS))
    {
        iAttempts++ ;

        // First attempt to get a free port for the free list, if that
        // fails, return a new one. 
        if (mlistFreePorts.entries())
        {
            UtlInt* pInt = (UtlInt*) mlistFreePorts.first() ;
            mlistFreePorts.remove(pInt) ;
            rtpPort = pInt->getValue() ;
            delete pInt ;
        }
        else
        {
            rtpPort = miNextRtpPort ;

            // Only allocate if the nextRtpPort is greater then 0 -- otherwise we
            // are allowing the system to allocate ports.
            if (miNextRtpPort > 0)
            {
                miNextRtpPort += 2 ; 
            }
        }

        // The only think that I can think of is that this is some weird x86_64 complier bug.
        // isAddressPortBusy is not derived that I can see and yet CpMediaInterfaceFactoryImpl::isAddressPortBusy
        // does not get called if the class scope is not explicitly used.  A strange value of 522 is
        // somehow returned.
        UtlBoolean portBusy =  CpMediaInterfaceFactoryImpl::isAddressPortBusy(bindAddress, rtpPort, MAX_PORT_CHECK_WAIT_MS);
        bGoodPort = (FALSE == portBusy);
        if (!bGoodPort)
        {
            mlistBusyPorts.insert(new UtlInt(rtpPort)) ;
        }
    }

    // If unable to find a usable port, let the system pick one.
    if (!bGoodPort)
    {
        OsSysLog::add(FAC_CP, PRI_WARNING, "CpMediaInterfaceFactoryImpl::getNextRtpPort found no ports defaulting to 0");
        rtpPort = 0 ;
    }
    
    UtlSListIterator freeIterator(mlistFreePorts);
    UtlInt* freePort = NULL;
    while((freePort = (UtlInt*) freeIterator()))
    {
        freePorts.appendFormat("%d, ", freePort->getValue());
    }

    UtlSListIterator busyIterator(mlistBusyPorts);
    UtlInt* busyPort = NULL;
    while((busyPort = (UtlInt*) busyIterator()))
    {
        busyPorts.appendFormat("%d, ", busyPort->getValue());
    }

    OsSysLog::add(FAC_CP, PRI_DEBUG,
            "CpMediaInterfaceFactoryImpl::getNextRtpPort free ports: %s", 
            freePorts.data());

    OsSysLog::add(FAC_CP, PRI_DEBUG,
            "CpMediaInterfaceFactoryImpl::getNextRtpPort busy ports: %s",
            busyPorts.data());

    return OS_SUCCESS ;
}


OsStatus CpMediaInterfaceFactoryImpl::releaseRtpPort(const int rtpPort) 
{
    OsSysLog::add(FAC_CP, PRI_DEBUG,
            "CpMediaInterfaceFactoryImpl::releaseRtpPort(%d)",
            rtpPort);
    UtlString freePorts;

    OsLock lock(mlockList) ;

    // Only bother noting the free port if the next port isn't 0 (OS selects 
    // port)
    if (miNextRtpPort != 0)
    {
        // if it is not already in the list...
        UtlInt rtpPortInt(rtpPort);
        if (mlistFreePorts.find(&rtpPortInt) == NULL)
        {
            // Release port to head of list (generally want to reuse ports)
            mlistFreePorts.insert(new UtlInt(rtpPort)) ;
        }

        UtlSListIterator iterator(mlistFreePorts);
        UtlInt* freePort = NULL;
        while((freePort = (UtlInt*) iterator()))
        {
            freePorts.appendFormat("%d, ", freePort->getValue());
        }
    }

    OsSysLog::add(FAC_CP, PRI_DEBUG,
            "CpMediaInterfaceFactoryImpl::releaseRtpPort free ports: %s", 
            freePorts.data());

    return OS_SUCCESS ;
}

// Static method to add codec paths
OsStatus CpMediaInterfaceFactoryImpl::addCodecPaths(const size_t nCodecPaths, 
                                                    const UtlString codecPaths[])
{
   size_t i;

   // Check each codecPath to see if it is valid.  If any of them are 
   // syntactically invalid.  If so, return failure.
   // TODO: Could use a more sophisticated determination of path validity. This one is pretty dumb.
   RegEx invalidPathChars("^[^*?\"<>|]*$");
   for(i = 0; i < nCodecPaths; i++)
   {
      if(! invalidPathChars.Search(codecPaths[i].data()) )
      {
         return OS_FAILED;
      }
   }

   if(ensureCapacityCodecPaths(mnCodecPaths+nCodecPaths) == OS_FAILED)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "CpMediaInterfaceFactory::addCodecPaths "
                    " - Error ensuring static codec path capacity when adding "
                    "%d codec paths to %d already existing paths!", 
                    (int) nCodecPaths, (int) mnCodecPaths);
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

void CpMediaInterfaceFactoryImpl::setConfiguredIpAddress(const UtlString& configuredIpAddress)
{
    mConfiguredIpAddress = configuredIpAddress;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean CpMediaInterfaceFactoryImpl::isAddressPortBusy(const char* bindAddress, int iPort, int checkTimeMS) 
{
    UtlBoolean bBusy = FALSE ;

    if (iPort > 0)
    {
        OsDatagramSocket* pSocket = new OsDatagramSocket(0, NULL, iPort, bindAddress);
        if (pSocket != NULL)
        {
            if (!pSocket->isOk())
            {
                OsSysLog::add(FAC_CP, PRI_DEBUG, 
                        "CpMediaInterfaceFactoryImpl::isAddressPortBusy UDP port %d not OK",
                        iPort);
                bBusy = TRUE;
            }
            else if(pSocket->isReadyToRead(checkTimeMS))
            {
                OsSysLog::add(FAC_CP, PRI_DEBUG, 
                        "CpMediaInterfaceFactoryImpl::isAddressPortBusy UDP port %d ready to read",
                        iPort);
                bBusy = TRUE;
            }
            pSocket->close() ;
            delete pSocket ;
        }
        
        // also check TCP port availability
        if(bBusy == FALSE)
        {
            OsServerSocket* pTcpSocket = new OsServerSocket(64, iPort, bindAddress, 1);
            if (pTcpSocket != NULL)
            {
                if (!pTcpSocket->isOk())
                {
                    OsSysLog::add(FAC_CP, PRI_DEBUG, 
                            "CpMediaInterfaceFactoryImpl::isAddressPortBusy TCP port %d not OK",
                            iPort);
                    bBusy = TRUE;
                }
                pTcpSocket->close();
                delete pTcpSocket;
            }
        }
    }

    return(bBusy);
}

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

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



