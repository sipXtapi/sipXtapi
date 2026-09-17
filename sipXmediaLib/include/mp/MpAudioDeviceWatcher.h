//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpAudioDeviceWatcher_h_
#define _MpAudioDeviceWatcher_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsRWMutex.h>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlContainer.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpAudioDeviceChangeObserver;

/**
 * @brief Tracks which audio devices are available, and tells an observer
 *        when that changes.
 *
 * One watcher per device manager, so one per direction.  Not one per
 * driver: drivers come and go during a device switch, and two watchers
 * alive at once seed themselves from different worlds and then disagree
 * about whether a later change is news.
 *
 * @section theset The available-device set
 *
 * The watcher keeps a set of the devices it believes are available,
 * seeded at construction before any platform notification is registered.
 * Every incoming change is compared against it:
 *
 *   - now available, not in the set   -> insert, call onDevicePresent
 *   - now available, already in set   -> nothing
 *   - now gone, in the set            -> erase, call onDeviceNotPresent
 *   - now gone, not in the set        -> nothing
 *
 * That comparison is the whole point.  Without it there is no way to
 * tell a real arrival or departure from a transition between two states
 * the application could never see anyway, and the current code gets
 * both cases wrong: it reports devices that were never available, and
 * stays silent about ones that really did go away.
 *
 * The set is also what the application's device list is built from.
 * There is deliberately not a second enumeration of the platform's
 * devices elsewhere; two implementations of "list the audio devices"
 * will disagree, and reconciling them by name is the fragile string
 * matching this design exists to avoid.
 *
 * @section entries What an entry holds
 *
 * Keyed by platform device id.  Each entry caches both names for the
 * device, because they are different strings serving different jobs.
 *
 *   displayName  what to show a user.  The platform's full, human
 *                readable name for the device.
 *   apiName      what the platform's audio API accepts when asked to
 *                open the device by name, and therefore what the
 *                name-based sipXtapi set-device calls have to be given.
 *
 * On most platforms these are the same string.  On Windows they are
 * not: the display name comes from MMDevice and the API name from
 * WinMM, which truncates to 31 characters, so
 * "Microphone (C-Media USB Audio Device   )" is opened as
 * "Microphone (C-Media USB Audio D".  Both are stored so neither has
 * to be derived from the other by string surgery.
 *
 * Names are cached at insertion, while the device is still available.
 * Resolving a name at removal time can fail, and an event carrying an
 * empty name is useless to an application.
 *
 * @section states Device states
 *
 * There is no notion of unplugged, disabled or not-present here.  Any
 * state other than available is a departure.  Platform subclasses
 * collapse whatever their platform reports down to the boolean that
 * onDeviceChanged takes, and nothing above this class sees the
 * difference.
 *
 * @section threading Threading
 *
 * onDeviceChanged and onDefaultChanged are called from whatever thread
 * the platform delivers notifications on, which on Windows is a COM
 * callback thread.  The device list accessors are called from the
 * application's thread.  The set is guarded accordingly.
 *
 * Observer methods are called with the lock released, so an observer
 * may call back into the watcher without deadlocking.
 *
 * @nosubgrouping
 */
class MpAudioDeviceWatcher
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

     /// @brief Which direction of devices this watcher tracks.
   enum MpDeviceFlow
   {
      MP_DEVICE_FLOW_CAPTURE,
      MP_DEVICE_FLOW_RENDER
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Make a watcher for this platform, or NULL if there is none.
   static MpAudioDeviceWatcher* create(MpDeviceFlow flow,
                                       MpAudioDeviceChangeObserver* observer);
     /**<
     *  The only place a concrete watcher is constructed, and the only
     *  place the platform ifdef lives.  Returns NULL on platforms with
     *  no implementation; callers treat that as "no device change
     *  notification available" and carry on.
     *
     *  The observer must outlive the watcher.
     */

   virtual ~MpAudioDeviceWatcher();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @brief Begin watching.  Seeds the set, then registers for changes.
   virtual OsStatus start();
     /**<
     *  Seeding happens before registration, deliberately.  A change
     *  arriving between the two would otherwise be compared against an
     *  empty set and reported as an arrival for a device that was
     *  already there.
     */

     /// @brief Stop watching.  Unregisters and waits out any in-flight call.
   virtual OsStatus stop();

     /// @brief A device became available, or stopped being available.
   void onDeviceChanged(const UtlString& deviceId,
                        const UtlString& displayName,
                        const UtlString& apiName,
                        UtlBoolean isAvailable);
     /**<
     *  Called by the platform subclass for every device state change it
     *  receives, with the platform's states already collapsed to
     *  available or not.  Applies the set comparison and calls the
     *  observer if the change is news.
     *
     *  Public, and takes no platform types, so that the set logic can
     *  be driven directly by a test with synthetic ids and names.  That
     *  is the only way to test the customer-reported cases: they need
     *  devices that arrive and depart on cue, which no test machine can
     *  be relied on to provide.
     *
     *  The names are used only when inserting.  On removal the cached
     *  names are reported and these are ignored.
     */

     /// @brief The system default device for this direction changed.
   void onDefaultChanged(const UtlString& deviceId,
                         const UtlString& displayName);
     /**<
     *  An empty deviceId means there is no longer a default device, and
     *  results in onNoDefaultDevice rather than onDefaultDeviceChanged.
     *
     *  Deduplicated against the last default reported, because a single
     *  device change can produce several notifications naming the same
     *  device.
     *
     *  Does not touch the available-device set.  A default change means
     *  the same devices with a different one preferred.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @brief Append the API name of every available device.
   virtual int getDeviceNames(UtlContainer& deviceNames) const;
     /**<
     *  The API name, not the display name: these feed the device list
     *  the application enumerates and then hands back to the name-based
     *  set-device calls, so they have to be names those calls accept.
     *
     *  @returns the number of names appended.
     */

     /// @brief Look up a device's ids and names.  FALSE if not available.
   virtual UtlBoolean getDeviceInfo(const UtlString& deviceId,
                                    UtlString& displayName,
                                    UtlString& apiName) const;

     /// @brief Find a device by either of its names.  FALSE if no match.
   virtual UtlBoolean findDeviceByName(const UtlString& name,
                                       UtlString& deviceId) const;
     /**<
     *  Matches the display name and the API name.  An application may
     *  hand back either: it might have read one from the device list
     *  and the other from an event.
     */

     /// @brief The current system default, or FALSE if there is none.
   virtual UtlBoolean getDefaultDevice(UtlString& deviceId,
                                       UtlString& displayName) const;

     /// @brief Which direction this watcher tracks.
   inline MpDeviceFlow getFlow() const { return mFlow; };

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// @brief How many devices are currently available.
   virtual int getNumDevices() const;

     /// @brief TRUE between a successful start() and stop().
   inline UtlBoolean isWatching() const { return mIsWatching; };

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpAudioDeviceWatcher(MpDeviceFlow flow,
                        MpAudioDeviceChangeObserver* observer);
     /**< Constructed only through create(). */

     /// @brief Fill the set with what is available now.
   virtual OsStatus seedAvailableDevices() = 0;
     /**<
     *  Called by start() before registering for changes.  The subclass
     *  enumerates its platform and calls addSeedDevice for each device.
     *
     *  A test subclass adds whatever devices the case under test needs
     *  and returns, which is what lets the set logic be exercised with
     *  no real hardware.
     */

     /// @brief Register for platform notifications.
   virtual OsStatus registerForChanges() = 0;

     /// @brief Unregister, and wait out any notification already running.
   virtual OsStatus unregisterForChanges() = 0;

     /// @brief Add a device during seeding, without notifying anyone.
   void addSeedDevice(const UtlString& deviceId,
                      const UtlString& displayName,
                      const UtlString& apiName);
     /**<
     *  Seeding is establishing what was already true, not observing a
     *  change, so it produces no events.
     */

     /// @brief Record the default device during seeding, silently.
   void setSeedDefault(const UtlString& deviceId,
                       const UtlString& displayName);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// @brief One available device.
   class MpDeviceEntry : public UtlString
   {
   public:
      MpDeviceEntry(const UtlString& deviceId,
                    const UtlString& displayName,
                    const UtlString& apiName);

      UtlString mDisplayName;   ///< For showing to a user.
      UtlString mApiName;       ///< For handing to the platform audio API.
   };
     /**<
     *  Derives from UtlString holding the device id, so that an entry
     *  identifies itself when one is held without its key.  The map
     *  stores a separate UtlString key alongside it, because UtlHashMap
     *  is a key to value map and its iterator yields keys.
     */

   MpDeviceFlow                 mFlow;
   MpAudioDeviceChangeObserver* mpObserver;      ///< Not owned.
   UtlHashMap                   mAvailableDevices; ///< id -> MpDeviceEntry.
   UtlString                    mDefaultDeviceId;
   UtlString                    mDefaultDisplayName;
   UtlBoolean                   mIsWatching;
   mutable OsRWMutex            mMutex;          ///< Guards the two above.

     /// @brief Disabled copy constructor.
   MpAudioDeviceWatcher(const MpAudioDeviceWatcher& rMpAudioDeviceWatcher);

     /// @brief Disabled assignment operator.
   MpAudioDeviceWatcher& operator=(const MpAudioDeviceWatcher& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAudioDeviceWatcher_h_

