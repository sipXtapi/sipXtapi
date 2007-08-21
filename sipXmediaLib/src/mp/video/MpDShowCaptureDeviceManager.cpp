//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef SIPX_VIDEO // [

// SYSTEM INCLUDES
#include <VideoSupport/VideoCapture.h>

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
   VideoCapture::DeviceNames names;
   VideoCapture::EnumDevices(names);
   if (deviceNum < 0 || deviceNum >= int(names.size()))
      return NULL;

   UtlString name = names[deviceNum].c_str();
   return getDeviceByName(name);
}

int MpDShowCaptureDeviceManager::getDeviceCount() const
{
   VideoCapture::DeviceNames names;
   VideoCapture::EnumDevices(names);
   return names.size();
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

#endif // SIPX_VIDEO ]
