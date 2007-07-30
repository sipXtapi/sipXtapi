//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
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
#include <mp/MpOutputDeviceManager.h>
#include <mp/MpOutputDeviceDriver.h>
#include <mp/MpAudioOutputConnection.h>
#include <mp/MpBuf.h>
#include <mp/MpAudioBuf.h>
#include <utl/UtlInt.h>

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
   assert(defaultSamplesPerFrame > 0);
   assert(defaultSamplesPerSecond > 0);

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


OsStatus MpOutputDeviceManager::enableDevice(MpOutputDeviceHandle deviceId,
                                             MpFrameTime mixerBufferLength)
{
   OsStatus status = OS_NOT_FOUND;
   MpAudioOutputConnection* connection = NULL;
   UtlInt deviceKey(deviceId);

   OsWriteLock lock(mRwMutex);

   connection = (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);

   if (connection != NULL)
   {
       status = connection->enableDevice(mDefaultSamplesPerFrame, 
                                         mDefaultSamplesPerSecond,
                                         getCurrentFrameTime(),
                                         mixerBufferLength
                                         );
   }
   return status;
}


OsStatus MpOutputDeviceManager::enableDevice(MpOutputDeviceHandle deviceId)
{
   return enableDevice(deviceId, mDefaultBufferLength);
}


OsStatus MpOutputDeviceManager::disableDevice(MpOutputDeviceHandle deviceId)
{
   OsStatus status = OS_NOT_FOUND;
   MpAudioOutputConnection* connection = NULL;
   UtlInt deviceKey(deviceId);

   {
      // Lock is took to increase use count only.
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

   if (status == OS_SUCCESS)
   {
      status = 
         connection->disableDevice();

      {
         // Lock is took to decrease use count only.
         OsWriteLock lock(mRwMutex);
         connection->decreaseUseCount();
      }
   }

   return status;
}


OsStatus MpOutputDeviceManager::pushFrame(MpOutputDeviceHandle deviceId,
                                          MpFrameTime frameTime,
                                          const MpBufPtr& frame)
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

         status = 
            connection->pushFrame(pAudioFrame->getSamplesNumber(),
                                  pAudioFrame->getSamplesPtr(),
                                  frameTime);
      } 
      else
      {
         // Notify output device that no data will be sent during this
         // processing interval.
         status = 
            connection->pushFrame(connection->getSamplesPerFrame(),
                                  NULL,
                                  frameTime);
      }
   }

   return status;
}

OsStatus MpOutputDeviceManager::setFlowgraphTickerSource(MpOutputDeviceHandle deviceId)
{
   OsStatus status = OS_SUCCESS;
   MpAudioOutputConnection* connection = NULL;

   // No need to take lock here as access to mConnectionsByDeviceId is atomic
   // read operation.
   if (mCurrentTickerDevice != deviceId)
   {
      OsWriteLock lock(mRwMutex);

      // Stop current ticker if was enabled.
      if (mCurrentTickerDevice != MP_INVALID_OUTPUT_DEVICE_HANDLE)
      {
         UtlInt deviceKey(mCurrentTickerDevice);
         connection = (MpAudioOutputConnection*) mConnectionsByDeviceId.find(&deviceKey);

         if (connection != NULL)
         {
            status = connection->disableFlowgraphTicker();
         }
         else
         {
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
            status = connection->enableFlowgraphTicker();
            if (status == OS_SUCCESS)
            {
               mCurrentTickerDevice = deviceId;
            }
         }
         else
         {
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


MpFrameTime MpOutputDeviceManager::getCurrentFrameTime() const
{
   OsTime now;
   OsDateTime::getCurTime(now);

   now -= mTimeZero;

   return(now.seconds() * 1000 + now.usecs() / 1000);
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

