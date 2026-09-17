//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpAudioDeviceWatcher.h>
#include <mp/MpAudioDeviceChangeObserver.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsSysLog.h>
#include <utl/UtlHashMapIterator.h>

// EXTERNAL FUNCTIONS

// Defined by the platform subclass's own .cpp.  Declared here rather
// than in the header so that no caller can name the concrete type.
// The Windows subclass defines this in its own .cpp, so that no header
// names the concrete type.  Until that file exists there is nothing to
// link against, and create() is the only caller.
#define MP_HAVE_WIN_AUDIO_DEVICE_WATCHER 0

#if defined(WIN32) && MP_HAVE_WIN_AUDIO_DEVICE_WATCHER // [
extern MpAudioDeviceWatcher*
   mpCreateWinAudioDeviceWatcher(MpAudioDeviceWatcher::MpDeviceFlow flow,
                                 MpAudioDeviceChangeObserver* observer);
#endif // WIN32 ]

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// static
MpAudioDeviceWatcher*
MpAudioDeviceWatcher::create(MpDeviceFlow flow,
                             MpAudioDeviceChangeObserver* observer)
{
   // The only platform ifdef in this design.  Everywhere else a caller
   // holds a base pointer and does not know or care what it is.
#if defined(WIN32) && MP_HAVE_WIN_AUDIO_DEVICE_WATCHER // [
   return mpCreateWinAudioDeviceWatcher(flow, observer);
#else // WIN32 ][
   // No implementation for this platform.  Callers treat NULL as "no
   // device change notification available" and fall back to whatever
   // they did before, which is a static device list.
   OsSysLog::add(FAC_MP, PRI_INFO,
      "MpAudioDeviceWatcher::create no implementation for this platform, "
      "flow: %d observer: %p",
      (int) flow, observer);
   return NULL;
#endif // WIN32 ]
}

MpAudioDeviceWatcher::MpAudioDeviceWatcher(MpDeviceFlow flow,
                                           MpAudioDeviceChangeObserver* observer)
: mFlow(flow)
, mpObserver(observer)
, mIsWatching(FALSE)
, mMutex(OsRWMutex::Q_PRIORITY)
{
   if (mpObserver == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "MpAudioDeviceWatcher constructed with NULL observer, flow: %d. "
         "Device changes will be tracked but not reported.",
         (int) mFlow);
   }
}

MpAudioDeviceWatcher::~MpAudioDeviceWatcher()
{
   if (mIsWatching)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "~MpAudioDeviceWatcher still watching at destruction, flow: %d. "
         "stop() should have been called first.",
         (int) mFlow);
   }
   mAvailableDevices.destroyAll();
}

MpAudioDeviceWatcher::MpDeviceEntry::MpDeviceEntry(const UtlString& deviceId,
                                                   const UtlString& displayName,
                                                   const UtlString& apiName)
: UtlString(deviceId)
, mDisplayName(displayName)
, mApiName(apiName)
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpAudioDeviceWatcher::start()
{
   if (mIsWatching)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "MpAudioDeviceWatcher::start already watching, flow: %d",
         (int) mFlow);
      return OS_FAILED;
   }

   // Seed first, register second.  A change arriving in between would be
   // compared against an empty set, and every device already present
   // would look like an arrival.
   OsStatus status = seedAvailableDevices();
   if (status != OS_SUCCESS)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpAudioDeviceWatcher::start seed failed, flow: %d status: %d",
         (int) mFlow, status);
      return status;
   }

   OsSysLog::add(FAC_MP, PRI_INFO,
      "MpAudioDeviceWatcher::start seeded flow: %d with %d device(s), "
      "default: \"%s\"",
      (int) mFlow, getNumDevices(), mDefaultDisplayName.data());

   status = registerForChanges();
   if (status != OS_SUCCESS)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpAudioDeviceWatcher::start register failed, flow: %d status: %d",
         (int) mFlow, status);
      return status;
   }

   mIsWatching = TRUE;
   return OS_SUCCESS;
}

OsStatus MpAudioDeviceWatcher::stop()
{
   if (!mIsWatching)
   {
      return OS_SUCCESS;
   }

   // The subclass must not return from this until any notification
   // already running has finished.  After it returns, the observer
   // pointer may be destroyed.
   OsStatus status = unregisterForChanges();
   mIsWatching = FALSE;

   OsSysLog::add(FAC_MP, PRI_INFO,
      "MpAudioDeviceWatcher::stop flow: %d status: %d", (int) mFlow, status);

   return status;
}

void MpAudioDeviceWatcher::onDeviceChanged(const UtlString& deviceId,
                                           const UtlString& displayName,
                                           const UtlString& apiName,
                                           UtlBoolean isAvailable)
{
   if (deviceId.isNull())
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "MpAudioDeviceWatcher::onDeviceChanged empty device id, flow: %d "
         "name: \"%s\" available: %d -- ignored",
         (int) mFlow, displayName.data(), isAvailable);
      return;
   }

   UtlBoolean isNews = FALSE;
   UtlString  reportedName;

   // Decide under the lock, report outside it.  An observer is free to
   // call back into this watcher; doing that while holding the lock
   // would deadlock.
   {
      OsWriteLock lock(mMutex);

      MpDeviceEntry* entry =
         (MpDeviceEntry*) mAvailableDevices.findValue(&deviceId);

      if (isAvailable)
      {
         if (entry == NULL)
         {
            mAvailableDevices.insertKeyAndValue(
               new UtlString(deviceId),
               new MpDeviceEntry(deviceId, displayName, apiName));
            reportedName = apiName;
            isNews = TRUE;
         }
         // Already known.  Nothing changed as far as anyone above us is
         // concerned, whatever the platform's reason for telling us.
      }
      else
      {
         if (entry != NULL)
         {
            // Report the name cached when the device arrived, not one
            // resolved now.  A device that has gone away often cannot
            // be asked for its name any more, and an event carrying an
            // empty name is no use to an application.
            reportedName = entry->mApiName;
            // destroy() frees both the key and the value.
            mAvailableDevices.destroy((UtlContainable*) &deviceId);
            isNews = TRUE;
         }
         // Never available, so its departure is not a change.  This is
         // the case the current code gets wrong: a device that was
         // already unplugged before we started, moving to some other
         // unavailable state, is reported as a removal.
      }
   }

   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpAudioDeviceWatcher::onDeviceChanged flow: %d id: \"%s\" "
      "name: \"%s\" available: %d news: %d",
      (int) mFlow, deviceId.data(), displayName.data(),
      isAvailable, isNews);

   if (isNews && mpObserver != NULL)
   {
      if (isAvailable)
      {
         mpObserver->onDevicePresent(deviceId, reportedName);
      }
      else
      {
         mpObserver->onDeviceNotPresent(deviceId, reportedName);
      }
   }
}

void MpAudioDeviceWatcher::onDefaultChanged(const UtlString& deviceId,
                                            const UtlString& displayName)
{
   UtlBoolean isNews = FALSE;
   UtlString  reportedName;

   {
      OsWriteLock lock(mMutex);

      if (deviceId.compareTo(mDefaultDeviceId) != 0)
      {
         mDefaultDeviceId = deviceId;
         mDefaultDisplayName = displayName;
         isNews = TRUE;
      }
      // One device change can produce several notifications naming the
      // same device.  Only the first is news.

      reportedName = mDefaultDisplayName;
   }

   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpAudioDeviceWatcher::onDefaultChanged flow: %d id: \"%s\" "
      "name: \"%s\" news: %d",
      (int) mFlow, deviceId.data(), displayName.data(), isNews);

   if (isNews && mpObserver != NULL)
   {
      if (deviceId.isNull())
      {
         // No default at all, which is not the same as a default whose
         // name we could not resolve.  Distinct callback so that an
         // application never has to guess which it is looking at.
         mpObserver->onNoDefaultDevice();
      }
      else
      {
         mpObserver->onDefaultDeviceChanged(deviceId, reportedName);
      }
   }
}

/* ============================ ACCESSORS ================================= */

int MpAudioDeviceWatcher::getDeviceNames(UtlContainer& deviceNames) const
{
   OsReadLock lock(mMutex);

   int count = 0;

   // The iterator yields keys.  The entry is the value behind the key.
   UtlHashMap& devices = (UtlHashMap&) mAvailableDevices;
   UtlHashMapIterator iterator(devices);
   UtlString* key = NULL;
   while ((key = (UtlString*) iterator()))
   {
      MpDeviceEntry* entry = (MpDeviceEntry*) devices.findValue(key);
      if (entry == NULL)
      {
         continue;
      }

      // The API name, not the display name.  These feed the list an
      // application enumerates and then hands back to the name-based
      // set-device calls, so they have to be names those calls accept.
      deviceNames.insert(new UtlString(entry->mApiName));
      count++;
   }

   return count;
}

UtlBoolean MpAudioDeviceWatcher::getDeviceInfo(const UtlString& deviceId,
                                               UtlString& displayName,
                                               UtlString& apiName) const
{
   OsReadLock lock(mMutex);

   MpDeviceEntry* entry = (MpDeviceEntry*)
      ((UtlHashMap&) mAvailableDevices).findValue(&deviceId);
   if (entry == NULL)
   {
      displayName.remove(0);
      apiName.remove(0);
      return FALSE;
   }

   displayName = entry->mDisplayName;
   apiName = entry->mApiName;
   return TRUE;
}

UtlBoolean MpAudioDeviceWatcher::findDeviceByName(const UtlString& name,
                                                  UtlString& deviceId) const
{
   if (name.isNull())
   {
      deviceId.remove(0);
      return FALSE;
   }

   OsReadLock lock(mMutex);

   MpDeviceEntry* firstMatch = NULL;
   UtlString      firstMatchId;

   UtlHashMap& devices = (UtlHashMap&) mAvailableDevices;
   UtlHashMapIterator iterator(devices);
   UtlString* key = NULL;
   while ((key = (UtlString*) iterator()))
   {
      MpDeviceEntry* entry = (MpDeviceEntry*) devices.findValue(key);
      if (entry == NULL)
      {
         continue;
      }

      // Either name matches.  An application may have taken one from
      // the device list and the other from an event, and has no reason
      // to know they are different strings.
      if (name.compareTo(entry->mApiName) == 0 ||
          name.compareTo(entry->mDisplayName) == 0)
      {
         if (firstMatch == NULL)
         {
            firstMatch = entry;
            firstMatchId = *key;
         }
         else
         {
            OsSysLog::add(FAC_MP, PRI_WARNING,
               "MpAudioDeviceWatcher::findDeviceByName \"%s\" matches more "
               "than one device on flow %d: \"%s\" and \"%s\". Using the "
               "first. Rename the devices so they differ, if the platform "
               "allows it.",
               name.data(), (int) mFlow,
               firstMatchId.data(), key->data());
         }
      }
   }

   if (firstMatch == NULL)
   {
      deviceId.remove(0);
      return FALSE;
   }

   deviceId = firstMatchId;
   return TRUE;
}

UtlBoolean MpAudioDeviceWatcher::getDefaultDevice(UtlString& deviceId,
                                                  UtlString& displayName) const
{
   OsReadLock lock(mMutex);

   deviceId = mDefaultDeviceId;
   displayName = mDefaultDisplayName;

   return (!mDefaultDeviceId.isNull());
}

/* ============================ INQUIRY =================================== */

int MpAudioDeviceWatcher::getNumDevices() const
{
   OsReadLock lock(mMutex);
   return (int) mAvailableDevices.entries();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpAudioDeviceWatcher::addSeedDevice(const UtlString& deviceId,
                                         const UtlString& displayName,
                                         const UtlString& apiName)
{
   if (deviceId.isNull())
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "MpAudioDeviceWatcher::addSeedDevice empty device id, flow: %d "
         "name: \"%s\" -- ignored",
         (int) mFlow, displayName.data());
      return;
   }

   OsWriteLock lock(mMutex);

   if (mAvailableDevices.findValue(&deviceId) != NULL)
   {
      return;
   }

   // Silent by design.  Seeding records what was already true; it is
   // not an observation that anything changed.
   mAvailableDevices.insertKeyAndValue(
      new UtlString(deviceId),
      new MpDeviceEntry(deviceId, displayName, apiName));

   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpAudioDeviceWatcher::addSeedDevice flow: %d id: \"%s\" "
      "display: \"%s\" api: \"%s\"",
      (int) mFlow, deviceId.data(), displayName.data(), apiName.data());
}

void MpAudioDeviceWatcher::setSeedDefault(const UtlString& deviceId,
                                          const UtlString& displayName)
{
   OsWriteLock lock(mMutex);

   mDefaultDeviceId = deviceId;
   mDefaultDisplayName = displayName;
}

