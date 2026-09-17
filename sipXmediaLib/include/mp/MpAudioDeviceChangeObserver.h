//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpAudioDeviceChangeObserver_h_
#define _MpAudioDeviceChangeObserver_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Interface implemented by whoever wants to be told that the set of
 * audio devices, or the system default audio device, has changed.
 *
 * MpAudioDeviceWatcher decides what is news and calls these methods.
 * Implementations translate to whatever their layer needs: the input
 * and output device managers post MPRNM_INPUT_DEVICE_NOW_PRESENT and
 * friends, a unit test records the calls.
 *
 * The watcher knows nothing about managers, message types or drivers.
 * That is deliberate: it is what lets the transition logic be tested
 * without COM and without a device.
 *
 * Identity is the platform device id.  Names are for display and for the
 * name based APIs.  Friendly names are ambiguous on real hardware: WinMM
 * truncates to 31 characters, and a machine can have two endpoints whose
 * truncated names are identical.
 *
 * The id is opaque.  Implementations compare it for equality against
 * MpInputDeviceDriver::getPlatformDeviceId() and never interpret it.  It
 * is empty on platforms where the driver name is already the identity,
 * which is every platform except Windows; an empty id means fall back to
 * name matching.
 *
 * All methods are called on the thread that delivered the platform
 * notification, which on Windows is a COM callback thread.  Do not
 * block in them.
 */
class MpAudioDeviceChangeObserver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   virtual ~MpAudioDeviceChangeObserver() {};

/* ============================ MANIPULATORS ============================== */

     /// @brief A device that was not previously available now is.
   virtual void onDevicePresent(const UtlString& deviceId,
                                const UtlString& deviceName) = 0;
     /**<
     *  Fired once per device, when the device enters the watcher's set
     *  of available devices.  Not fired for a device that was already
     *  in the set.
     */

     /// @brief A device that was available no longer is.
   virtual void onDeviceNotPresent(const UtlString& deviceId,
                                   const UtlString& deviceName) = 0;
     /**<
     *  Fired once per device, when the device leaves the watcher's set
     *  of available devices.  Not fired for a device that was never in
     *  the set.
     *
     *  deviceName is the name cached when the device entered the set,
     *  not a name resolved now.  The platform may no longer be able to
     *  resolve a name for a device that has gone away.
     */

     /// @brief This observer's own bound device has gone away.
   virtual void onOwnDeviceLost() = 0;
     /**<
     *  Fired in addition to onDeviceNotPresent, when the departing
     *  device is the one this observer is bound to.  What to do about
     *  it differs by direction: the input driver marks itself closed,
     *  the output driver also falls back to the MM timer so the
     *  flowgraph keeps ticking.
     *
     *  Takes no argument.  The match against this observer's own device
     *  has already been made by the caller, by platform device id where
     *  one exists, so there is nothing further to compare.
     */

     /// @brief The system default device changed.
   virtual void onDefaultDeviceChanged(const UtlString& deviceId,
                                       const UtlString& deviceName) = 0;
     /**<
     *  Fired once per direction for the role the watcher follows, not
     *  once per role.  Windows keeps a separate default for each of
     *  eConsole, eMultimedia and eCommunications, and they can point at
     *  different devices at the same time.
     *
     *  deviceName is never empty.  Absence of a default is reported by
     *  onNoDefaultDevice instead, because an empty name is ambiguous:
     *  it could equally mean name resolution failed, or that the
     *  endpoint has no friendly name, both of which occur in practice.
     *
     *  Both id and name are passed, as everywhere in this interface.
     *  The name is what an application hands back to
     *  sipxAudioSetCallInputDevice; the id is what a manager compares
     *  against the device it is currently bound to.
     */

     /// @brief There is no longer any default device for this direction.
   virtual void onNoDefaultDevice() = 0;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAudioDeviceChangeObserver_h_

