//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: 


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpOSSDeviceWrapperContainer.h"
#include "mp/MpOSSDeviceWrapper.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsRWMutex.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


MpOSSDeviceWrapperContainer mOSSContainer;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Constructor
MpOSSDeviceWrapperContainer::MpOSSDeviceWrapperContainer() :
                                mMutex(OsRWMutex::Q_FIFO)
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
                //printf("Object destriyng in global destructor: dev %p with key %p\n", pDev, pDevName); 
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
    //printf("MpOSSDeviceWrapperContainer::excludeFromContainer dev=%p\n", pDevWrap);
    //Lock
    mMutex.acquireWrite();
    
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
    
    //printf("OSSdev %s removing\n", (pDevName->data()) ? pDevName->data() : "<null>" );
    
    pRetName = static_cast<UtlString*>(removeKeyAndValue(pDevName, pDevTmp));
    delete pRetName;

    MpOSSDeviceWrapper *pDev = static_cast<MpOSSDeviceWrapper*>(pDevTmp);    
    delete pDev;
    
    //printf("Removed!\n"); fflush(stdout);
    //Unlock
    mMutex.releaseWrite();
}

MpOSSDeviceWrapper* MpOSSDeviceWrapperContainer::getOSSDeviceWrapper(const UtlString& ossdev)
{
    //Lock    
    mMutex.acquireWrite();
    
    MpOSSDeviceWrapper *pDev;
    UtlString* pName;    
    
    pDev = static_cast<MpOSSDeviceWrapper*>(findValue(&ossdev));

    if (pDev == NULL)
    {
        //printf("OSSdev %s not found\n",  (ossdev.data()) ? ossdev.data() : "<null>");
        pName = new UtlString(ossdev);
        pDev = new MpOSSDeviceWrapper;
        insertKeyAndValue(pName, pDev);
        //printf("Adding new dev %p with key %p\n", pDev, pName);        
    } else {
        //printf("OSSdev %s found: %p!\n",  ossdev.data(), pDev);
    }
    //Unlock
    mMutex.releaseWrite();  
    return pDev;
}

