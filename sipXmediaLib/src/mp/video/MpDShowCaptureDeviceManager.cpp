//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef SIPX_VIDEO // [

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpDShowCaptureDeviceManager.h"
#include "mp/video/MpDShowCaptureDevice.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpDShowCaptureDeviceManager::MpDShowCaptureDeviceManager()
{

}

MpDShowCaptureDeviceManager::~MpDShowCaptureDeviceManager()
{

}

/* ============================ MANIPULATORS ============================== */

OsStatus MpDShowCaptureDeviceManager::initialize()
{
   return OS_SUCCESS;
}

OsStatus MpDShowCaptureDeviceManager::unInitialize()
{
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

MpCaptureDeviceBase *MpDShowCaptureDeviceManager::getDeviceByName(const UtlString &deviceName)
{
   MpDShowCaptureDevice *pDevice = new MpDShowCaptureDevice(deviceName);
   return pDevice;
}

MpCaptureDeviceBase *MpDShowCaptureDeviceManager::getDeviceByNum(int deviceNum)
{
   return NULL;
}

int MpDShowCaptureDeviceManager::getDeviceCount() const
{
   return 0;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

#endif // SIPX_VIDEO ]
