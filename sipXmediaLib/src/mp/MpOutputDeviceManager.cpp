//  
// Copyright (C) 2007-2017 SIPez LLC. All rights reserved.
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsWriteLock.h>
#include <os/OsReadLock.h>
#include <os/OsDateTime.h>
#include <os/OsSysLog.h>
#include <os/OsTask.h>
#include <utl/UtlInt.h>
#include <utl/UtlDList.h>
#include <utl/UtlDListIterator.h>
#include <utl/UtlHashBagIterator.h>
#include <mp/MpOutputDeviceManager.h>
#include <mp/MpOutputDeviceDriver.h>
#include <mp/MpAudioOutputConnection.h>
#include <mp/MpBuf.h>
#include <mp/MpAudioBuf.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// PRIVATE CLASSES

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpOutputDeviceManager::MpOutputDeviceManager(unsigned defaultSamplesPerFrame, 
                                             unsigned defaultSamplesPerSecond,
                                             MpFrameTime defaultMixerBufferLength)
: mRwMutex(OsRWMutex::Q_PRIORITY)
, mLastDeviceId(0)
, mDefaultSamplesPerFrame(defaultSamplesPerFrame)
, mDefaultSamplesPerSecond(defaultSamplesPerSecond)
, mDefaultBufferLength(defaultMixerBufferLength)
, mCurrentTickerDevice(MP_INVALID_OUTPUT_DEVICE_HANDLE)
{
   assert(mDefaultSamplesPerFrame > 0);
   assert(mDefaultSamplesPerSecond > 0);
   assert(mDefaultBufferLength > 0);

   OsDateTime::getCurTime(mTimeZero);
}


// Destructor
MpOutputDeviceManager::~MpOutputDeviceManager()
{
   // All devices (and connections, so) should be removed from manager before
   // manager destroyed.
   assert(mConnectionsByDeviceName.entries() == 0);
   assert(mConnectionsByDeviceId.entries() == 0);
}

/* ============================ MANIPULATORS ============================== */
int MpOutputDeviceManager::addDevice(MpOutputDeviceDriver *newDevice)
{
   OsWriteLock lock(mRwMutex);

   // Be sure device ID will not wrap over MAX_INT.
   assert(mLastDeviceId < mLastDeviceId+1);
   // Get new device ID.
   MpOutputDeviceHandle newDeviceId = ++mLastDeviceId;

   // Create a connection to contain the device and its buffered frames
   MpAudioOutputConnection* connection = 
      new MpAudioOutputConnection(newDeviceId, newDevice);

   // Map by device name string
   UtlInt* idValue = new UtlInt(newDeviceId);
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpOutputDeviceManager::addDevice dev: %s id: %d", 
                 newDevice->data(), newDeviceId);
   mConnectionsByDeviceName.insertKeyAndValue(newDevice, idValue);

   // Map by device ID
   mConnectionsByDeviceId.insert(connection);

   return newDeviceId;
}


MpOutputDeviceDriver* MpOutputDeviceManager::removeDevice(MpOutputDeviceHandle deviceId)
{
   MpAudioOutputConnection* connection = NULL;
   MpOutputDeviceDriver* deviceDriver = NULL;

   OsWriteLock lock(mRwMutex);

   connection = findConnectionBlocking(deviceId);

   if (connection != NULL && connection->getUseCount() == 0)
   {
      // Remove from the id indexed container
      mConnectionsByDeviceId.remove(connection);

      deviceDriver = connection->getDeviceDriver();
      assert(deviceDriver != NULL);

      // Get the int value mapped in the hash so we can clean up
      UtlInt* deviceIdInt =
         (UtlInt*) mConnectionsByDeviceName.findValue(deviceDriver);

      // Remove from the name indexed hash
      mConnectionsByDeviceName.remove(deviceDriver);
      if (deviceIdInt)
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MpOutputDeviceManager::removeDevice dev: %s id: %d", 
                       deviceDriver->data(), deviceIdInt->getValue());
         delete deviceIdInt;
         deviceIdInt = NULL;
      }

      delete connection;
      connection = NULL;
   }

   // deviceDriver of NULL is returned if:
   //    * The connection is not found.
   //    * The connection is found, but the connection was already in use.
   return deviceDriver;
}


int MpOutputDeviceManager::removeAllDevices()
{
    int numRemoved = 0;
    MpOutputDeviceHandle deviceId = MP_INVALID_OUTPUT_DEVICE_HANDLE;

    while(mConnectionsByDeviceId.entries())
    {
        {
            OsWriteLock lock(mRwMutex);
            MpAudioOutputConnection* connection = NULL;
            UtlHashBagIterator iterator(mConnectionsByDeviceId);
            if((connection = (MpAudioOutputConnection*) iterator()))
            {
                deviceId = connection->getValue();
            }
            else
            {
                deviceId = MP_INVALID_OUTPUT_DEVICE_HANDLE;
            }
        }

        if(deviceId > MP_INVALID_OUTPUT_DEVICE_HANDLE)
        {
            // If device is not disabled, disable it
            if(isDeviceEnabled(deviceId))
            {
                disableDevice(deviceId);
            }

            // Remove device
            MpOutputDeviceDriver* deviceDriver = removeDevice(deviceId);

            // Need to delete the device as it is not in removeDevice
            if(deviceDriver)
            {
                delete deviceDriver;
                deviceDriver = NULL;
            }

            numRemoved++;
        }
    }

    return(numRemoved);
}

OsStatus MpOutputDeviceManager::enableDevice(MpOutputDeviceHandle deviceId,
                                             MpFrameTime mixerBufferLength,
                                             uint32_t samplesPerFrame,
                                             uint32_t samplesPerSec)
{
   OsStatus status = OS_NOT_FOUND;
   MpAudioOutputConnection* connection = NULL;
   UtlInt deviceKey(deviceId);

   OsWriteLock lock(mRwMutex);

   // If mixer buffer length or sample rate or samples per frame
   // were not provided, use defaults.
   mixerBufferLength = (mixerBufferLength == 0) ? mDefaultBufferLength : mixerBufferLength;
   samplesPerFrame = (samplesPerFrame == 0) ? mDefaultSamplesPerFrame : samplesPerFrame;
   samplesPerSec = (samplesPerSec == 0) ? mDefaultSamplesPerSecond : samplesPerSec;

   connection = (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);

   if (connection != NULL)
   {
       status = connection->enableDevice(samplesPerFrame, 
                                         samplesPerSec,
                                         0,
                                         mixerBufferLength);
       OsSysLog::add(FAC_MP, PRI_DEBUG,
           "MpOutputDeviceManager::enableDevice enableDevice(samplesPerFrames: %d, samplesPerSec: %d,0, mixerBufferLength: %d) id: %d (%s) returned: %d",
           samplesPerFrame, samplesPerSec, mixerBufferLength, deviceId, (connection->getDeviceDriver()->getDeviceName()).data(), status);
   }
   else
   {
       OsSysLog::add(FAC_MP, PRI_ERR, "MpOutputDeviceManager::enableDevice deviceId: %d, connection not found",
           deviceId);
   }

   return(status);
}


OsStatus MpOutputDeviceManager::disableDevice(MpOutputDeviceHandle deviceId)
{
   OsStatus status = OS_NOT_FOUND;
   MpAudioOutputConnection* connection = NULL;
   UtlInt deviceKey(deviceId);

   {
      // Lock is taken to increase use count only.
      OsWriteLock lock(mRwMutex);

      connection = findConnectionBlocking(deviceId);

      if (connection != NULL)
      {
         if (connection->getUseCount() > 0)
         {
            // If the connection is in use by someone else,
            // then we indicate that the connection is busy.
            status = OS_BUSY;
         }
         else
         {
            // It's ok to disable now,
            // Set the connection in use.
            status = OS_SUCCESS;
            connection->increaseUseCount();
         }
      }
   }

   UtlString deviceName;

   if (status == OS_SUCCESS)
   {
      status = 
         connection->disableDevice();

      deviceName = connection->getDeviceDriver()->getDeviceName();

      {
         // Lock is took to decrease use count only.
         OsWriteLock lock(mRwMutex);
         connection->decreaseUseCount();
      }
   }

   OsSysLog::add(FAC_MP, PRI_DEBUG,
       "MpOutputDeviceManager::disableDevice(%d) (%s) return: %d",
       deviceId, deviceName.data(), status);

   return status;
}


OsStatus MpOutputDeviceManager::disableAllDevicesExcept(int exceptCount, MpOutputDeviceHandle exceptDeviceIds[])
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MpOutputDeviceManager::disableAllDevicesExcept(%d, %p)", exceptCount, exceptDeviceIds);
    OsStatus status = OS_SUCCESS;
    UtlDList exceptions;
    for(int i = 0; i < exceptCount; i++)
    {
        exceptions.append(new UtlInt(exceptDeviceIds[i]));
    }

    UtlDList disableList;

    // Lock is taken to get device IDs only.
    {
        OsWriteLock lock(mRwMutex);
        MpAudioOutputConnection* connection = NULL;
        UtlHashBagIterator iterator(mConnectionsByDeviceId);
        while((connection = (MpAudioOutputConnection*) iterator()))
        {
            // If not in the exception list, add to the list to be disabled
            UtlInt deviceIdInt(connection->getValue());
            if(exceptions.find(&deviceIdInt) == NULL)
            {
                disableList.append(new UtlInt(connection->getValue()));
            }
        }
    }

    // Disable the devices
    UtlDListIterator disableIterator(disableList);
    UtlInt* deviceIdPtr = NULL;
    while((deviceIdPtr = (UtlInt*) disableIterator()))
    {
        // Locking is done in disableDevice
        status = disableDevice(deviceIdPtr->getValue());
    }

    disableList.destroyAll();
    exceptions.destroyAll();

    return(status);
}

OsStatus MpOutputDeviceManager::pushFrameFirst(MpOutputDeviceHandle deviceId,
                                               MpFrameTime &frameTime,
                                               const MpBufPtr& frame,
                                               UtlBoolean initFrameTime)
{
   OsStatus status = OS_NOT_FOUND;
   MpAudioOutputConnection* connection = NULL;
   UtlInt deviceKey(deviceId);

   OsReadLock lock(mRwMutex);

   connection = (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);

   if (connection != NULL)
   {
      if (frame.isValid())
      {
         // Send actual data to output device.
         MpAudioBufPtr pAudioFrame(const_cast<MpBufPtr&>(frame));

         if (!initFrameTime)
         {
            status = 
               connection->pushFrame(pAudioFrame->getSamplesNumber(),
                                     pAudioFrame->getSamplesPtr(),
                                     frameTime);
         }
         else
         {
            status = 
               connection->pushFrameBeginning(pAudioFrame->getSamplesNumber(),
                                              pAudioFrame->getSamplesPtr(),
                                              frameTime);
         }
      } 
      else
      {
         // Notify output device that no data will be sent during this
         // processing interval.
         if (!initFrameTime)
         {
            status = 
               connection->pushFrame(connection->getSamplesPerFrame(),
                                     NULL,
                                     frameTime);
         }
         else
         {
            status = 
               connection->pushFrameBeginning(connection->getSamplesPerFrame(),
                                              NULL,
                                              frameTime);
         }
      }
   }

   return status;
}

OsStatus MpOutputDeviceManager::pushFrame(MpOutputDeviceHandle deviceId,
                                          MpFrameTime frameTime,
                                          const MpBufPtr& frame)
{
   return pushFrameFirst(deviceId, frameTime, frame, FALSE);
}

OsStatus MpOutputDeviceManager::setFlowgraphTickerSource(MpOutputDeviceHandle deviceId,
                                                         OsNotification *pFlowgraphTicker)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MpOutputDeviceManager::setFlowgraphTickerSource(deviceId: %d, pFlowgraphTicker: %p)",
        deviceId, pFlowgraphTicker);
   OsStatus status = OS_SUCCESS;
   MpAudioOutputConnection* connection = NULL;
   OsWriteLock lock(mRwMutex);

   if (mCurrentTickerDevice != deviceId)
   {
      // Stop current ticker if was enabled.
      if (mCurrentTickerDevice != MP_INVALID_OUTPUT_DEVICE_HANDLE)
      {
         UtlInt deviceKey(mCurrentTickerDevice);
         connection = (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);

         if (connection != NULL)
         {
            status = connection->disableFlowgraphTicker();
            OsSysLog::add(FAC_MP, PRI_ERR,
                "MpOutputDeviceManager::setFlowgraphTickerSource disableFlowgraphTicker currrent deviceId: %d returned: %d",
                mCurrentTickerDevice, status);
         }
         else
         {
             OsSysLog::add(FAC_MP, PRI_ERR,
                 "MpOutputDeviceManager::setFlowgraphTickerSource current deviceId: %d not found",
                 mCurrentTickerDevice);

            status = OS_INVALID_STATE;
         }

         mCurrentTickerDevice = MP_INVALID_OUTPUT_DEVICE_HANDLE;
      }

      // Start new ticker if requested.
      if (status == OS_SUCCESS && deviceId != MP_INVALID_OUTPUT_DEVICE_HANDLE)
      {
         UtlInt deviceKey(deviceId);
         connection = (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);

         if (connection != NULL)
         {
            status = connection->enableFlowgraphTicker(pFlowgraphTicker);
            if (status == OS_SUCCESS)
            {
               mCurrentTickerDevice = deviceId;
            }
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpOutputDeviceManager::setFlowgraphTickerSource enableFlowgraphTicker deviceId: %d returned: %d",
                deviceId, status);
         }
         else
         {
             OsSysLog::add(FAC_MP, PRI_ERR,
                 "MpOutputDeviceManager::setFlowgraphTickerSource new deviceId: %d not found",
                 deviceId);
            status = OS_NOT_FOUND;
         }
      }

   }

   return status;
}

/* ============================ ACCESSORS ================================= */

OsStatus MpOutputDeviceManager::getDeviceName(MpOutputDeviceHandle deviceId,
                                              UtlString& deviceName) const
{
   OsStatus status = OS_NOT_FOUND;
   MpAudioOutputConnection* connection = NULL;
   UtlInt deviceKey(deviceId);
   MpOutputDeviceDriver* deviceDriver = NULL;

   OsReadLock lock(mRwMutex);

   connection =
      (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);

   if (connection)
   {
      deviceDriver = connection->getDeviceDriver();
      assert(deviceDriver != NULL);
      if (deviceDriver != NULL)
      {
         status = OS_SUCCESS;
         deviceName = *deviceDriver;
      }
   }

   return status;
}


OsStatus MpOutputDeviceManager::getDeviceId(const UtlString& deviceName,
                                            MpOutputDeviceHandle &deviceId) const
{
   UtlString deviceString(deviceName);

   OsReadLock lock(mRwMutex);

   UtlInt* deviceKey = (UtlInt*) mConnectionsByDeviceName.findValue(&deviceString);

   if (deviceKey != NULL)
   {
      deviceId = deviceKey->getValue();
      return OS_SUCCESS;
   }
   else
   {
      deviceId = MP_INVALID_OUTPUT_DEVICE_HANDLE;
      return OS_NOT_FOUND;
   }
}


OsStatus MpOutputDeviceManager::getMixerBufferLength(MpOutputDeviceHandle deviceId,
                                                     MpFrameTime &length) const
{
   OsStatus status = OS_FAILED;
   MpAudioOutputConnection* connection = NULL;
   UtlInt deviceKey(deviceId);

   OsReadLock lock(mRwMutex);

   connection =
      (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);

   if (connection)
   {
      length = connection->getMixerBufferLength();
      status = OS_SUCCESS;
   }
   else
   {
      length = 0;
      status = OS_NOT_FOUND;
   }

   return status;
}

MpFrameTime MpOutputDeviceManager::getCurrentFrameTime(MpOutputDeviceHandle deviceId) const
{
   MpFrameTime curFrameTime = 0;

   OsReadLock lock(mRwMutex);

   MpAudioOutputConnection* connectionFound = NULL;
   UtlInt deviceKey(deviceId);
   connectionFound =
      (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);

   if (connectionFound)
   {
      curFrameTime = connectionFound->getCurrentFrameTime();
   }

   return curFrameTime;
}

OsStatus MpOutputDeviceManager::getDeviceSamplesPerSec(MpOutputDeviceHandle deviceId,
                                                       uint32_t& samplesPerSec) const
{
   OsStatus ret = OS_NOT_FOUND;
   UtlInt deviceKey(deviceId);
   MpAudioOutputConnection* connectionFound = 
      (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);
   MpOutputDeviceDriver* pDevDriver = 
      (connectionFound) ? connectionFound->getDeviceDriver() : NULL;

   if(pDevDriver)
   {
      samplesPerSec = pDevDriver->getSamplesPerSec();
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
          "MpOutputDeviceManager::getDeviceSamplesPerSec device sample rate: %d", samplesPerSec);
#endif
      ret = OS_SUCCESS;
   }
   else
   {
       samplesPerSec = mDefaultSamplesPerSecond;
   }
   return ret;
}


OsStatus MpOutputDeviceManager::getDeviceSamplesPerFrame(MpOutputDeviceHandle deviceId,
                                                         uint32_t& samplesPerFrame) const
{
   OsStatus ret = OS_NOT_FOUND;
   UtlInt deviceKey(deviceId);
   MpAudioOutputConnection* connectionFound = 
      (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);
   MpOutputDeviceDriver* pDevDriver = 
      (connectionFound) ? connectionFound->getDeviceDriver() : NULL;

   if(pDevDriver)
   {
      samplesPerFrame = pDevDriver->getSamplesPerFrame();
      ret = OS_SUCCESS;
   }

   return ret;
}

/* ============================ INQUIRY =================================== */

UtlBoolean MpOutputDeviceManager::isDeviceEnabled(MpOutputDeviceHandle deviceId) const
{
   UtlBoolean enabledState = FALSE;
   OsReadLock lock(mRwMutex);

   MpAudioOutputConnection* connectionFound = NULL;
   UtlInt deviceKey(deviceId);
   connectionFound =
      (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);
   MpOutputDeviceDriver* deviceDriver = NULL;

   if (connectionFound)
   {
      deviceDriver = connectionFound->getDeviceDriver();
      assert(deviceDriver);
      if (deviceDriver)
      {
         enabledState = 
            deviceDriver->isEnabled();
      }
   }
   return(enabledState);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

MpAudioOutputConnection* MpOutputDeviceManager::findConnectionBlocking(
                                                   MpOutputDeviceHandle deviceId,
                                                   int tries) const
{
   UtlInt deviceKey(deviceId);
   MpAudioOutputConnection* connection = NULL;

   for (int i = 0; i < tries; i--)
   {
      // Lookup connection on every iteration, as we may lose it when released
      // lock.
      connection =
         (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);

      // If we couldn't find a connection, or we found the connection
      // and it isn't in use, then no need to continue looping.
      // The loop only continues if a connection was found and in use.
      if ( (connection == NULL) ||
           (connection->getUseCount() == 0))
      {
         break;
      }

      // If the device is found and in use, release the manager lock,
      // wait a small bit, and get the lock again to give the manager a
      // chance to finish what it was doing  with the connection.
      mRwMutex.releaseWrite();
      OsTask::delay(10);
      mRwMutex.acquireWrite();
   }

   if (connection == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                     "MpOutputDeviceManager::findConnectionBlocking():"
                     "could not find device with handle=%d\n", 
                     deviceId);
   } else if (connection->getUseCount() > 0)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                     "MpOutputDeviceManager::findConnectionBlocking():"
                     "device with handle=%d in use even after %d tries.\n", 
                     deviceId, tries);
   }

   return connection;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

