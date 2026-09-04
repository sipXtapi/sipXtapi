//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// btaudio_ctl.cpp -- connect and disconnect Bluetooth audio devices from
// the command line, without touching the radio and without unpairing.
//
// Why this exists
// ---------------
// Testing how sipXtapi reacts to an audio device going away needs a
// device that can be taken away on cue.  Measurements on a paired
// Bluetooth speaker show that the state Windows reports depends on how
// the device goes away, not on which device it is:
//
//    action                        endpoint state
//    ----------------------------  --------------
//    Bluetooth radio switched off  NOTPRESENT
//    device disconnected           UNPLUGGED
//    device powered off            UNPLUGGED
//
// The customer's virtual audio driver produces UNPLUGGED, so a test that
// switches the radio off exercises the wrong path.  Only the disconnect
// reproduces it, and Windows offers no command for that: the Settings
// page and the Win+K panel are the only ways in.
//
// How it works
// ------------
// Windows connects Bluetooth audio through the audio driver rather than
// the Bluetooth API, so the control is in the Core Audio stack.  The
// adapter device behind an endpoint accepts KSPROPSETID_BtAudio with
// KSPROPERTY_ONESHOT_DISCONNECT or KSPROPERTY_ONESHOT_RECONNECT.  This
// is the route the Settings page uses.  The idea comes from
// https://github.com/m2jean/ToothTray (BSD-2-Clause); no code is taken
// from it.
//
// The sequence for reaching IKsControl is the one documented in
// "Using the IKsControl Interface to Access Audio Properties":
//
//   1. endpoint IMMDevice -> Activate(IDeviceTopology)
//   2. IDeviceTopology::GetConnector -> IConnector
//   3. IConnector::GetDeviceIdConnectedTo -> the adapter device id
//   4. IMMDeviceEnumerator::GetDevice(adapter id) -> adapter IMMDevice
//   5. adapter->Activate(IID_IKsControl, ...)
//
// Step 5 is on the adapter device, not on the endpoint and not on the
// connector's IPart.  IPart::Activate does not offer IKsControl at all:
// its documented list is IAudioBass, IKsJackDescription and the like.
// Asking IPart for one returns E_NOINTERFACE for every device on the
// machine, which reads as "nothing here speaks Bluetooth" and is not
// what is happening.
//
// Diagnosis
// ---------
// Every step of the topology walk fails routinely for some endpoint or
// other, and the failures mean different things.  So no step fails
// silently: --list prints why control could not be reached, and --diag
// prints the whole walk for one device.  Debugging this without the
// HRESULTs is guesswork.
//
// Build, from ~/dev/sipXtapi/sipXmediaLib/src/test/mp in a Cygwin ssh
// shell on the build VM:
//
//   V=$(cygpath -d '/cygdrive/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars64.bat') && cmd /s /c "call $V && cl /nologo /EHsc /W3 btaudio_ctl.cpp ole32.lib oleaut32.lib"
//
// The cygpath -d short path is what makes that work: it has no spaces
// and no parentheses, so nothing needs quoting on the Windows side.
//
// No administrator rights are needed.
//
// Usage:
//   btaudio_ctl.exe --list
//   btaudio_ctl.exe --diag "SB510"
//   btaudio_ctl.exe --status "SB510"
//   btaudio_ctl.exe --disconnect "SB510"
//   btaudio_ctl.exe --connect "SB510"
//   btaudio_ctl.exe --disconnect "SB510" --timeout 20 --quiet
//   btaudio_ctl.exe --disconnect "SB510" --force
//
// Exit codes, so a script can tell the failures apart:
//   0  did what was asked
//   1  bad arguments
//   2  COM would not start
//   3  no endpoint matched
//   4  matched, but no Bluetooth control could be reached
//   5  the property request failed
//   6  request accepted but the state did not change in time
//   7  already in the requested state, nothing to do
//

#define _WIN32_WINNT 0x0600
#define INITGUID

#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <devicetopology.h>
#include <functiondiscoverykeys_devpkey.h>
#include <ks.h>
#include <ksmedia.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <vector>
#include <string>

// ---------------------------------------------------------------------------
// Interfaces and identifiers declared locally
//
// These are declared in Windows headers too, but in headers that have
// moved between SDK versions, and IKsControl is declared differently in
// kernel and user mode.  Picking up the wrong declaration does not fail
// to compile; it fails at run time with E_NOINTERFACE on every endpoint,
// which looks exactly like "this machine has no Bluetooth audio".
// Declaring them here costs nothing and removes that whole class of
// confusion.
// ---------------------------------------------------------------------------

// IKsControl.  The IID and the method order are those of ksproxy.h,
// confirmed against the SDK's ksuuids.h: KsProperty, KsMethod, KsEvent.
// Declared here, under a different name, so that including any header
// that also declares IKsControl cannot clash, and so that the IID does
// not depend on which header wins.
struct __declspec(uuid("28F54685-06FD-11D2-B27A-00A0C9223196"))
SipxKsControl : public IUnknown
{
   virtual HRESULT STDMETHODCALLTYPE KsProperty(PKSPROPERTY property,
                                                ULONG propertyLength,
                                                void* propertyData,
                                                ULONG dataLength,
                                                ULONG* bytesReturned) = 0;
   virtual HRESULT STDMETHODCALLTYPE KsMethod(PKSMETHOD method,
                                              ULONG methodLength,
                                              void* methodData,
                                              ULONG dataLength,
                                              ULONG* bytesReturned) = 0;
   virtual HRESULT STDMETHODCALLTYPE KsEvent(PKSEVENT eventData,
                                             ULONG eventLength,
                                             void* data,
                                             ULONG dataLength,
                                             ULONG* bytesReturned) = 0;
};

// KSPROPSETID_BtAudio, {7FA06C40-B8F6-4C7E-8556-E8C33A12E54D}.
static const GUID SIPX_KSPROPSETID_BtAudio =
   { 0x7fa06c40, 0xb8f6, 0x4c7e,
     { 0x85, 0x56, 0xe8, 0xc3, 0x3a, 0x12, 0xe5, 0x4d } };

#define SIPX_KSPROPERTY_ONESHOT_RECONNECT   0
#define SIPX_KSPROPERTY_ONESHOT_DISCONNECT  1

// PKEY_Device_ContainerId.  Confirmed against the MMDevices registry,
// where this key holds a VT_CLSID.
DEFINE_PROPERTYKEY(SIPX_PKEY_Device_ContainerId,
                   0x8c7ed206, 0x3f8a, 0x4827,
                   0xb3, 0xab, 0xae, 0x9e, 0x1f, 0xae, 0xfc, 0x6c, 2);

#define DEFAULT_TIMEOUT_SECONDS  20
#define POLL_INTERVAL_MS         250
#define WHY_LEN                  200

// ---------------------------------------------------------------------------
// Output
// ---------------------------------------------------------------------------

static int g_quiet = 0;
static int g_verbose = 0;

static void say(const char* format, ...)
{
   if (g_quiet)
   {
      return;
   }
   va_list args;
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
   fflush(stdout);
}

static void sayAlways(const char* format, ...)
{
   va_list args;
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
   fflush(stdout);
}

static void sayVerbose(const char* format, ...)
{
   if (!g_verbose)
   {
      return;
   }
   va_list args;
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
   fflush(stdout);
}

static const char* stateName(DWORD state)
{
   switch (state)
   {
      case DEVICE_STATE_ACTIVE:     return "ACTIVE";
      case DEVICE_STATE_DISABLED:   return "DISABLED";
      case DEVICE_STATE_NOTPRESENT: return "NOTPRESENT";
      case DEVICE_STATE_UNPLUGGED:  return "UNPLUGGED";
      case 0:                       return "GONE";
      default:                      return "UNKNOWN";
   }
}

static const char* flowName(EDataFlow flow)
{
   switch (flow)
   {
      case eRender:  return "render";
      case eCapture: return "capture";
      default:       return "?";
   }
}

/// @brief Name the HRESULTs this code actually produces.
///
/// Each means something different about what to do next, and looking
/// them up by hand every time is how an afternoon disappears.
static const char* hresultHint(HRESULT hr)
{
   switch ((unsigned long) hr)
   {
      case 0x80004002UL: return "E_NOINTERFACE, that interface is not on offer";
      case 0x80070490UL: return "ERROR_NOT_FOUND, nothing connected on the other side";
      case 0x88890008UL: return "AUDCLNT_E_DEVICE_INVALIDATED";
      case 0x80070005UL: return "E_ACCESSDENIED";
      case 0x8007007AUL: return "ERROR_INSUFFICIENT_BUFFER, the property wants a bigger buffer";
      case 0x80070032UL: return "ERROR_NOT_SUPPORTED, the driver does not implement it";
      case 0x8007000DUL: return "ERROR_INVALID_DATA";
      case 0x80070006UL: return "E_HANDLE";
      case 0x80004001UL: return "E_NOTIMPL";
      case 0x80004005UL: return "E_FAIL";
      case 0x80070001UL: return "ERROR_INVALID_FUNCTION, the property set is unknown here";
      default:           return "";
   }
}

static void sayHresult(const char* what, HRESULT hr)
{
   const char* hint = hresultHint(hr);
   if (hint[0] != '\0')
   {
      say("      %s = 0x%08lX  (%s)\n", what, (unsigned long) hr, hint);
   }
   else
   {
      say("      %s = 0x%08lX\n", what, (unsigned long) hr);
   }
}

/// Case-insensitive substring test on wide strings.
static bool containsNoCase(const std::wstring& haystack,
                           const std::wstring& needle)
{
   if (needle.empty())
   {
      return true;
   }
   // Do not name a local "small" here: rpcndr.h, reached through
   // windows.h, does #define small char.
   std::wstring hay = haystack;
   std::wstring pin = needle;
   for (size_t i = 0; i < hay.size(); i++) { hay[i] = towlower(hay[i]); }
   for (size_t i = 0; i < pin.size(); i++) { pin[i] = towlower(pin[i]); }
   return hay.find(pin) != std::wstring::npos;
}

static std::string guidToString(const GUID& guid)
{
   char buffer[64];
   sprintf_s(buffer, sizeof(buffer),
             "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
             guid.Data1, guid.Data2, guid.Data3,
             guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
             guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
   return std::string(buffer);
}

/// @brief Is this container id usable for grouping?
///
/// Windows uses {00000000-0000-0000-FFFF-FFFFFFFFFFFF} to mean "the
/// container is not known", and every device without one shares it.  On
/// a laptop under test that placeholder was shared by six unrelated
/// endpoints: two codec endpoints, two AUX jacks and two HD Audio
/// devices.  Grouping by it would mean a disconnect aimed at one device
/// took out five others.
static bool isUsableContainer(const GUID& guid)
{
   static const GUID unknownContainer =
      { 0x00000000, 0x0000, 0x0000,
        { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };

   if (IsEqualGUID(guid, unknownContainer) || IsEqualGUID(guid, GUID_NULL))
   {
      return false;
   }
   return true;
}

// ---------------------------------------------------------------------------
// Endpoint description
// ---------------------------------------------------------------------------

struct Endpoint
{
   std::wstring id;
   std::wstring name;
   std::wstring filterId;       ///< id of the adapter device behind it
   DWORD        state;
   EDataFlow    flow;
   GUID         containerId;
   bool         hasContainer;   ///< a usable one, not the placeholder
   bool         isBluetooth;    ///< the filter answered for the BtAudio set
   bool         looksBluetooth; ///< the filter id smells of Bluetooth
   bool         controlReachable;
   char         why[WHY_LEN];   ///< which step failed, when one did

   Endpoint()
   : state(0)
   , flow(eAll)
   , hasContainer(false)
   , isBluetooth(false)
   , looksBluetooth(false)
   , controlReachable(false)
   {
      memset(&containerId, 0, sizeof(containerId));
      why[0] = '\0';
   };
};

// Where the last topology walk gave up.  Set by getKsControlForEndpoint
// so that callers can report it without every one of them plumbing an
// error string through.
static char g_lastWhy[WHY_LEN] = "";

// ---------------------------------------------------------------------------
// Reaching the adapter device behind an endpoint
//
// endpoint -> IDeviceTopology -> a connector -> the id of the adapter on
// the other side -> that adapter's IMMDevice -> IKsControl.  See the
// note at the top of this file for why the last step is on the adapter
// rather than on the connector's part.
//
// Each step fails routinely rather than exceptionally.  An endpoint that
// is NOTPRESENT has no adapter to reach.  An endpoint whose device was
// removed has a connector with nothing on the other side.  So a failure
// is recorded and returned, never treated as fatal to the run.
//
// Every connector is tried, not just index 0.  An endpoint usually has
// one, but nothing guarantees the one we want is first.
// ---------------------------------------------------------------------------

static HRESULT getKsControlForEndpoint(IMMDeviceEnumerator* pEnumerator,
                                       IMMDevice* pDevice,
                                       SipxKsControl** ppKsControl,
                                       std::wstring* pAdapterId)
{
   *ppKsControl = NULL;
   g_lastWhy[0] = '\0';
   if (pAdapterId != NULL)
   {
      pAdapterId->clear();
   }

   IDeviceTopology* pTopology = NULL;
   HRESULT hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL,
                                  NULL, (void**) &pTopology);
   if (FAILED(hr) || pTopology == NULL)
   {
      sprintf_s(g_lastWhy, WHY_LEN, "Activate(IDeviceTopology)=0x%08lX %s",
                (unsigned long) hr, hresultHint(hr));
      return FAILED(hr) ? hr : E_FAIL;
   }

   UINT connectorCount = 0;
   hr = pTopology->GetConnectorCount(&connectorCount);
   if (FAILED(hr))
   {
      sprintf_s(g_lastWhy, WHY_LEN, "GetConnectorCount=0x%08lX %s",
                (unsigned long) hr, hresultHint(hr));
      pTopology->Release();
      return hr;
   }
   sayVerbose("      connectors: %u\n", connectorCount);

   if (connectorCount == 0)
   {
      sprintf_s(g_lastWhy, WHY_LEN, "the endpoint has no connectors");
      pTopology->Release();
      return E_FAIL;
   }

   HRESULT lastFailure = E_FAIL;
   char    lastStep[WHY_LEN];
   sprintf_s(lastStep, WHY_LEN, "no connector yielded a control interface");

   for (UINT index = 0; index < connectorCount; index++)
   {
      IConnector* pConnector = NULL;
      hr = pTopology->GetConnector(index, &pConnector);
      if (FAILED(hr) || pConnector == NULL)
      {
         sprintf_s(lastStep, WHY_LEN, "GetConnector(%u of %u)=0x%08lX %s",
                   index, connectorCount, (unsigned long) hr, hresultHint(hr));
         lastFailure = FAILED(hr) ? hr : E_FAIL;
         continue;
      }

      // The id of the adapter device on the other side.  Fails with
      // ERROR_NOT_FOUND when nothing is connected, which is the normal
      // state for an endpoint whose device has been removed.
      LPWSTR connectedId = NULL;
      hr = pConnector->GetDeviceIdConnectedTo(&connectedId);
      pConnector->Release();

      if (FAILED(hr) || connectedId == NULL)
      {
         sprintf_s(lastStep, WHY_LEN,
                   "GetDeviceIdConnectedTo(connector %u)=0x%08lX %s",
                   index, (unsigned long) hr, hresultHint(hr));
         lastFailure = FAILED(hr) ? hr : E_FAIL;
         continue;
      }

      std::wstring adapterId = connectedId;
      CoTaskMemFree(connectedId);
      sayVerbose("      connector %u connects to %ws\n",
                 index, adapterId.c_str());

      if (pAdapterId != NULL && pAdapterId->empty())
      {
         *pAdapterId = adapterId;
      }

      IMMDevice* pAdapter = NULL;
      hr = pEnumerator->GetDevice(adapterId.c_str(), &pAdapter);
      if (FAILED(hr) || pAdapter == NULL)
      {
         sprintf_s(lastStep, WHY_LEN, "GetDevice(adapter)=0x%08lX %s",
                   (unsigned long) hr, hresultHint(hr));
         lastFailure = FAILED(hr) ? hr : E_FAIL;
         continue;
      }

      // Argument order is iid first, then the class context: see
      // IMMDevice::Activate.  The endpoint's Activate would refuse this
      // interface; the adapter's does not.
      hr = pAdapter->Activate(__uuidof(SipxKsControl), CLSCTX_ALL, NULL,
                              (void**) ppKsControl);
      pAdapter->Release();

      if (SUCCEEDED(hr) && *ppKsControl != NULL)
      {
         pTopology->Release();
         return S_OK;
      }

      // E_NOINTERFACE here on every device would mean the IID in the
      // SipxKsControl declaration is wrong, rather than the devices
      // being uncooperative.
      sprintf_s(lastStep, WHY_LEN,
                "IMMDevice::Activate(IKsControl)=0x%08lX %s",
                (unsigned long) hr, hresultHint(hr));
      lastFailure = FAILED(hr) ? hr : E_FAIL;
   }

   strcpy_s(g_lastWhy, WHY_LEN, lastStep);
   pTopology->Release();
   return lastFailure;
}

/// @brief Does the adapter device id look like a Bluetooth device?
///
/// A secondary signal only.  Hardware ids are not a documented contract
/// and Windows is free to change them.  It is here because when the
/// property query fails this is the only evidence left about what we
/// were talking to, and "a Bluetooth filter refused" is a different
/// problem from "that was the wrong device".
static bool filterIdLooksBluetooth(const std::wstring& adapterId)
{
   return containsNoCase(adapterId, L"bthenum") ||
          containsNoCase(adapterId, L"bthhfenum") ||
          containsNoCase(adapterId, L"bthhfp") ||
          containsNoCase(adapterId, L"bthavrcp") ||
          containsNoCase(adapterId, L"bthle");
}

/// @brief Does this filter understand the Bluetooth audio property set?
///
/// Tried two ways.  BASICSUPPORT is documented to return a DWORD of
/// support flags, but a driver may want a full KSPROPERTY_DESCRIPTION,
/// in which case the DWORD form comes back with
/// ERROR_INSUFFICIENT_BUFFER.  Failing both is not proof of anything, so
/// the reason is written to why rather than swallowed.
static bool supportsBtAudio(SipxKsControl* pKsControl, char* why, size_t whyLen)
{
   KSPROPERTY property;
   ULONG      returned = 0;
   DWORD      support = 0;

   memset(&property, 0, sizeof(property));
   property.Set = SIPX_KSPROPSETID_BtAudio;
   property.Id = SIPX_KSPROPERTY_ONESHOT_DISCONNECT;
   property.Flags = KSPROPERTY_TYPE_BASICSUPPORT;

   HRESULT hr = pKsControl->KsProperty(&property, sizeof(property),
                                       &support, sizeof(support), &returned);
   if (SUCCEEDED(hr))
   {
      if (why != NULL)
      {
         sprintf_s(why, whyLen, "BASICSUPPORT ok, flags 0x%08lX",
                   (unsigned long) support);
      }
      return true;
   }
   HRESULT dwordHr = hr;

   KSPROPERTY_DESCRIPTION description;
   memset(&description, 0, sizeof(description));
   returned = 0;

   memset(&property, 0, sizeof(property));
   property.Set = SIPX_KSPROPSETID_BtAudio;
   property.Id = SIPX_KSPROPERTY_ONESHOT_DISCONNECT;
   property.Flags = KSPROPERTY_TYPE_BASICSUPPORT;

   hr = pKsControl->KsProperty(&property, sizeof(property),
                               &description, sizeof(description), &returned);
   if (SUCCEEDED(hr))
   {
      if (why != NULL)
      {
         sprintf_s(why, whyLen,
                   "BASICSUPPORT ok with description, access 0x%08lX",
                   (unsigned long) description.AccessFlags);
      }
      return true;
   }

   if (why != NULL)
   {
      sprintf_s(why, whyLen, "BASICSUPPORT dword=0x%08lX description=0x%08lX %s",
                (unsigned long) dwordHr, (unsigned long) hr,
                hresultHint(dwordHr));
   }
   return false;
}

/// @brief Ask the driver to connect or disconnect.
///
/// The one-shot properties carry no data and are documented as GET
/// requests.  SET is tried too, because a driver wanting SET would fail
/// in a way indistinguishable from the device refusing, and that is a
/// miserable thing to debug remotely.  Both results are reported.
static HRESULT sendOneShot(SipxKsControl* pKsControl, ULONG propertyId,
                           const char** whichWorked)
{
   KSPROPERTY property;
   ULONG returned = 0;

   memset(&property, 0, sizeof(property));
   property.Set = SIPX_KSPROPSETID_BtAudio;
   property.Id = propertyId;
   property.Flags = KSPROPERTY_TYPE_GET;

   HRESULT hr = pKsControl->KsProperty(&property, sizeof(property),
                                       NULL, 0, &returned);
   if (SUCCEEDED(hr))
   {
      *whichWorked = "GET";
      return hr;
   }
   HRESULT getHr = hr;

   memset(&property, 0, sizeof(property));
   property.Set = SIPX_KSPROPSETID_BtAudio;
   property.Id = propertyId;
   property.Flags = KSPROPERTY_TYPE_SET;

   hr = pKsControl->KsProperty(&property, sizeof(property),
                               NULL, 0, &returned);
   if (SUCCEEDED(hr))
   {
      *whichWorked = "SET";
      return hr;
   }

   sayHresult("one-shot as GET", getHr);
   sayHresult("one-shot as SET", hr);
   *whichWorked = "neither";
   return hr;
}

// ---------------------------------------------------------------------------
// Enumeration
// ---------------------------------------------------------------------------

static HRESULT enumerateEndpoints(IMMDeviceEnumerator* pEnumerator,
                                  std::vector<Endpoint>& endpoints,
                                  bool probeControl)
{
   endpoints.clear();

   IMMDeviceCollection* pCollection = NULL;
   HRESULT hr = pEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATEMASK_ALL,
                                                &pCollection);
   if (FAILED(hr) || pCollection == NULL)
   {
      sayAlways("ERROR: EnumAudioEndpoints = 0x%08lX %s\n",
                (unsigned long) hr, hresultHint(hr));
      return FAILED(hr) ? hr : E_FAIL;
   }

   UINT count = 0;
   hr = pCollection->GetCount(&count);
   if (FAILED(hr))
   {
      sayAlways("ERROR: GetCount = 0x%08lX\n", (unsigned long) hr);
      pCollection->Release();
      return hr;
   }

   for (UINT i = 0; i < count; i++)
   {
      IMMDevice* pDevice = NULL;
      hr = pCollection->Item(i, &pDevice);
      if (FAILED(hr) || pDevice == NULL)
      {
         sayAlways("WARNING: endpoint %u could not be opened, 0x%08lX\n",
                   i, (unsigned long) hr);
         continue;
      }

      Endpoint endpoint;

      LPWSTR deviceId = NULL;
      hr = pDevice->GetId(&deviceId);
      if (SUCCEEDED(hr) && deviceId != NULL)
      {
         endpoint.id = deviceId;
         CoTaskMemFree(deviceId);
      }
      else
      {
         // Without an id it cannot be acted on later, so do not pretend
         // it is usable.
         sayAlways("WARNING: endpoint %u has no id, 0x%08lX, skipped\n",
                   i, (unsigned long) hr);
         pDevice->Release();
         continue;
      }

      if (FAILED(pDevice->GetState(&endpoint.state)))
      {
         endpoint.state = 0;
      }

      IMMEndpoint* pEndpoint = NULL;
      if (SUCCEEDED(pDevice->QueryInterface(__uuidof(IMMEndpoint),
                                            (void**) &pEndpoint)) &&
          pEndpoint != NULL)
      {
         pEndpoint->GetDataFlow(&endpoint.flow);
         pEndpoint->Release();
      }

      IPropertyStore* pProps = NULL;
      hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
      if (SUCCEEDED(hr) && pProps != NULL)
      {
         PROPVARIANT value;

         PropVariantInit(&value);
         if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &value)) &&
             value.vt == VT_LPWSTR && value.pwszVal != NULL)
         {
            endpoint.name = value.pwszVal;
         }
         PropVariantClear(&value);

         PropVariantInit(&value);
         if (SUCCEEDED(pProps->GetValue(SIPX_PKEY_Device_ContainerId, &value)) &&
             value.vt == VT_CLSID && value.puuid != NULL)
         {
            endpoint.containerId = *value.puuid;
            endpoint.hasContainer = isUsableContainer(endpoint.containerId);
         }
         PropVariantClear(&value);

         pProps->Release();
      }
      else
      {
         sayVerbose("      OpenPropertyStore = 0x%08lX\n", (unsigned long) hr);
      }

      if (endpoint.name.empty())
      {
         endpoint.name = L"(no name)";
      }

      if (probeControl)
      {
         SipxKsControl* pKsControl = NULL;
         hr = getKsControlForEndpoint(pEnumerator, pDevice, &pKsControl,
                                      &endpoint.filterId);
         if (SUCCEEDED(hr) && pKsControl != NULL)
         {
            endpoint.controlReachable = true;
            endpoint.isBluetooth =
               supportsBtAudio(pKsControl, endpoint.why, WHY_LEN);
            pKsControl->Release();
         }
         else
         {
            strcpy_s(endpoint.why, WHY_LEN, g_lastWhy);
         }
         endpoint.looksBluetooth = filterIdLooksBluetooth(endpoint.filterId);
      }

      endpoints.push_back(endpoint);
      pDevice->Release();
   }

   pCollection->Release();
   return S_OK;
}

static DWORD getEndpointState(IMMDeviceEnumerator* pEnumerator,
                              const std::wstring& id)
{
   IMMDevice* pDevice = NULL;
   HRESULT hr = pEnumerator->GetDevice(id.c_str(), &pDevice);
   if (FAILED(hr) || pDevice == NULL)
   {
      return 0;
   }

   DWORD state = 0;
   if (FAILED(pDevice->GetState(&state)))
   {
      state = 0;
   }
   pDevice->Release();
   return state;
}

// ---------------------------------------------------------------------------
// Selection
//
// A Bluetooth device shows up as several endpoints: hands-free capture,
// hands-free render, stereo render.  Windows keeps the link until every
// one of them is disconnected, so acting only on the one whose name
// matched would appear to work and do nothing.  Match by name, then
// widen to everything sharing a usable container id.
// ---------------------------------------------------------------------------

static void selectByNameAndContainer(const std::vector<Endpoint>& all,
                                     const std::wstring& match,
                                     std::vector<Endpoint>& selected)
{
   selected.clear();

   std::vector<GUID> containers;
   bool matchedWithoutContainer = false;

   for (size_t i = 0; i < all.size(); i++)
   {
      if (!containsNoCase(all[i].name, match))
      {
         continue;
      }
      if (all[i].hasContainer)
      {
         bool known = false;
         for (size_t c = 0; c < containers.size(); c++)
         {
            if (IsEqualGUID(containers[c], all[i].containerId))
            {
               known = true;
               break;
            }
         }
         if (!known)
         {
            containers.push_back(all[i].containerId);
         }
      }
      else
      {
         matchedWithoutContainer = true;
      }
   }

   for (size_t i = 0; i < all.size(); i++)
   {
      bool take = false;

      if (all[i].hasContainer)
      {
         for (size_t c = 0; c < containers.size(); c++)
         {
            if (IsEqualGUID(containers[c], all[i].containerId))
            {
               take = true;
               break;
            }
         }
      }

      // An endpoint with no usable container can only be matched by
      // name.  Widening on the unknown-container placeholder would sweep
      // in every unrelated device that also lacks one.
      if (!take && matchedWithoutContainer && containsNoCase(all[i].name, match))
      {
         take = true;
      }

      if (take)
      {
         selected.push_back(all[i]);
      }
   }
}

// ---------------------------------------------------------------------------
// Commands
// ---------------------------------------------------------------------------

static int doList(IMMDeviceEnumerator* pEnumerator)
{
   std::vector<Endpoint> endpoints;
   if (FAILED(enumerateEndpoints(pEnumerator, endpoints, true)))
   {
      return 3;
   }

   sayAlways("%-11s %-8s %-4s %-38s %s\n",
             "STATE", "FLOW", "BT", "CONTAINER", "NAME");

   int bluetoothCount = 0;
   int unreachableCount = 0;

   for (size_t i = 0; i < endpoints.size(); i++)
   {
      const Endpoint& endpoint = endpoints[i];

      const char* bt = "no";
      if (endpoint.isBluetooth)
      {
         bt = "YES";
         bluetoothCount++;
      }
      else if (endpoint.looksBluetooth)
      {
         // The filter id says Bluetooth but the property query did not
         // answer.  Flagged separately: --force can still act on these,
         // and the reason line below says what went wrong.
         bt = "bth?";
      }
      else if (!endpoint.controlReachable)
      {
         bt = "?";
         unreachableCount++;
      }

      std::string container = "-";
      if (endpoint.hasContainer)
      {
         container = guidToString(endpoint.containerId);
      }

      sayAlways("%-11s %-8s %-4s %-38s %ws\n",
                stateName(endpoint.state),
                flowName(endpoint.flow),
                bt,
                container.c_str(),
                endpoint.name.c_str());

      if (!endpoint.isBluetooth && endpoint.why[0] != '\0')
      {
         sayAlways("            %s\n", endpoint.why);
      }
      if (g_verbose && !endpoint.filterId.empty())
      {
         sayAlways("            filter %ws\n", endpoint.filterId.c_str());
      }
   }

   sayAlways("\n%d endpoint(s), %d Bluetooth, %d with no control interface\n",
             (int) endpoints.size(), bluetoothCount, unreachableCount);

   if (bluetoothCount == 0)
   {
      sayAlways("\nNo endpoint accepted the Bluetooth property set.\n");
      sayAlways("If a Bluetooth device is connected, run --diag with part of\n");
      sayAlways("its name for the whole topology walk.\n");
   }

   return 0;
}

/// @brief Everything known about the endpoints matching a name.
///
/// The mode to run when --list says something unexpected.  Verbose is
/// forced on for its duration, so connector counts, filter ids and every
/// HRESULT come out in one go rather than one recompile at a time.
static int doDiag(IMMDeviceEnumerator* pEnumerator, const std::wstring& match)
{
   int savedVerbose = g_verbose;
   g_verbose = 1;

   std::vector<Endpoint> all;
   if (FAILED(enumerateEndpoints(pEnumerator, all, false)))
   {
      g_verbose = savedVerbose;
      return 3;
   }

   std::vector<Endpoint> selected;
   selectByNameAndContainer(all, match, selected);

   if (selected.empty())
   {
      sayAlways("no endpoint matching \"%ws\". Run --list to see them all.\n",
                match.c_str());
      g_verbose = savedVerbose;
      return 3;
   }

   sayAlways("%d endpoint(s) selected for \"%ws\"\n\n",
             (int) selected.size(), match.c_str());

   int reachable = 0;
   int bluetooth = 0;

   for (size_t i = 0; i < selected.size(); i++)
   {
      const Endpoint& endpoint = selected[i];

      sayAlways("%ws\n", endpoint.name.c_str());
      sayAlways("   state     %s\n", stateName(endpoint.state));
      sayAlways("   flow      %s\n", flowName(endpoint.flow));
      sayAlways("   id        %ws\n", endpoint.id.c_str());
      sayAlways("   container %s\n",
                endpoint.hasContainer
                   ? guidToString(endpoint.containerId).c_str()
                   : "none usable");

      IMMDevice* pDevice = NULL;
      HRESULT hr = pEnumerator->GetDevice(endpoint.id.c_str(), &pDevice);
      if (FAILED(hr) || pDevice == NULL)
      {
         sayAlways("   GetDevice = 0x%08lX %s\n\n",
                   (unsigned long) hr, hresultHint(hr));
         continue;
      }

      std::wstring filterId;
      SipxKsControl* pKsControl = NULL;
      hr = getKsControlForEndpoint(pEnumerator, pDevice, &pKsControl, &filterId);
      pDevice->Release();

      if (!filterId.empty())
      {
         sayAlways("   adapter   %ws\n", filterId.c_str());
         sayAlways("   adapter looks bluetooth: %s\n",
                   filterIdLooksBluetooth(filterId) ? "yes" : "no");
      }

      if (FAILED(hr) || pKsControl == NULL)
      {
         sayAlways("   no control interface: %s\n\n", g_lastWhy);
         continue;
      }

      reachable++;
      sayAlways("   control interface obtained\n");

      char why[WHY_LEN];
      why[0] = '\0';
      if (supportsBtAudio(pKsControl, why, WHY_LEN))
      {
         bluetooth++;
         sayAlways("   bluetooth property set: yes (%s)\n", why);
      }
      else
      {
         sayAlways("   bluetooth property set: no (%s)\n", why);
      }

      pKsControl->Release();
      sayAlways("\n");
   }

   sayAlways("%d of %d endpoint(s) gave a control interface, %d spoke Bluetooth\n",
             reachable, (int) selected.size(), bluetooth);

   g_verbose = savedVerbose;
   return 0;
}

static int doStatus(IMMDeviceEnumerator* pEnumerator, const std::wstring& match)
{
   std::vector<Endpoint> all;
   if (FAILED(enumerateEndpoints(pEnumerator, all, false)))
   {
      return 3;
   }

   std::vector<Endpoint> selected;
   selectByNameAndContainer(all, match, selected);

   if (selected.empty())
   {
      sayAlways("no endpoint matching \"%ws\"\n", match.c_str());
      return 3;
   }

   for (size_t i = 0; i < selected.size(); i++)
   {
      sayAlways("%-11s %-8s %ws\n",
                stateName(selected[i].state),
                flowName(selected[i].flow),
                selected[i].name.c_str());
   }

   return 0;
}

static int doConnectOrDisconnect(IMMDeviceEnumerator* pEnumerator,
                                 const std::wstring& match,
                                 bool wantConnect,
                                 int timeoutSeconds,
                                 bool force)
{
   const char* action = wantConnect ? "connect" : "disconnect";
   DWORD wantedState = wantConnect ? DEVICE_STATE_ACTIVE
                                   : DEVICE_STATE_UNPLUGGED;
   ULONG propertyId = wantConnect ? SIPX_KSPROPERTY_ONESHOT_RECONNECT
                                  : SIPX_KSPROPERTY_ONESHOT_DISCONNECT;

   std::vector<Endpoint> all;
   if (FAILED(enumerateEndpoints(pEnumerator, all, false)))
   {
      return 3;
   }

   std::vector<Endpoint> selected;
   selectByNameAndContainer(all, match, selected);

   if (selected.empty())
   {
      sayAlways("no endpoint matching \"%ws\". Try --list.\n", match.c_str());
      return 3;
   }

   say("%s: %d endpoint(s) in this device's group\n",
       action, (int) selected.size());

   // Stale endpoints from an earlier pairing are NOTPRESENT and cannot
   // change, so they must not be read as work needing doing.  Otherwise
   // "already in the requested state" is never reported, and a caller
   // cannot tell a transition it caused from one that had already
   // happened -- which is the difference that matters to a test.
   bool anyNeedsChange = false;
   for (size_t i = 0; i < selected.size(); i++)
   {
      if (selected[i].state == DEVICE_STATE_NOTPRESENT &&
          wantedState != DEVICE_STATE_NOTPRESENT)
      {
         continue;
      }
      if (selected[i].state != wantedState)
      {
         anyNeedsChange = true;
      }
   }
   if (!anyNeedsChange)
   {
      // Its own outcome, not success.  A test driving this needs to know
      // whether it caused a transition or merely found one already done.
      sayAlways("already %s -- every endpoint is %s\n",
                wantConnect ? "connected" : "disconnected",
                stateName(wantedState));
      return 7;
   }

   int accepted = 0;
   int unreachable = 0;
   int refused = 0;
   int notBluetooth = 0;

   for (size_t i = 0; i < selected.size(); i++)
   {
      const Endpoint& endpoint = selected[i];

      say("   %ws [%s %s]\n",
          endpoint.name.c_str(),
          stateName(endpoint.state),
          flowName(endpoint.flow));

      IMMDevice* pDevice = NULL;
      HRESULT hr = pEnumerator->GetDevice(endpoint.id.c_str(), &pDevice);
      if (FAILED(hr) || pDevice == NULL)
      {
         // A device can go away between enumerating and acting.
         sayHresult("GetDevice", hr);
         unreachable++;
         continue;
      }

      std::wstring filterId;
      SipxKsControl* pKsControl = NULL;
      hr = getKsControlForEndpoint(pEnumerator, pDevice, &pKsControl, &filterId);
      pDevice->Release();

      if (FAILED(hr) || pKsControl == NULL)
      {
         // Routine when already disconnected: there is no filter to talk
         // to.  Another endpoint of the same device may still work, and
         // for a reconnect any one of them is enough.
         say("      no control interface: %s\n", g_lastWhy);
         unreachable++;
         continue;
      }

      char why[WHY_LEN];
      why[0] = '\0';
      bool speaksBt = supportsBtAudio(pKsControl, why, WHY_LEN);

      if (!speaksBt && !force)
      {
         say("      not a Bluetooth audio endpoint: %s\n", why);
         if (filterIdLooksBluetooth(filterId))
         {
            say("      but its adapter id looks like Bluetooth; --force will\n");
            say("      send the request anyway\n");
         }
         pKsControl->Release();
         notBluetooth++;
         continue;
      }
      if (!speaksBt)
      {
         say("      forcing past: %s\n", why);
      }

      const char* whichWorked = "neither";
      hr = sendOneShot(pKsControl, propertyId, &whichWorked);
      pKsControl->Release();

      if (SUCCEEDED(hr))
      {
         say("      %s request accepted (%s)\n", action, whichWorked);
         accepted++;
      }
      else
      {
         say("      %s request failed\n", action);
         refused++;
      }
   }

   if (accepted == 0)
   {
      if (refused > 0)
      {
         sayAlways("every %s request failed (%d refused)\n", action, refused);
         return 5;
      }
      if (notBluetooth > 0)
      {
         sayAlways("no endpoint of \"%ws\" speaks the Bluetooth property set\n",
                   match.c_str());
         sayAlways("run --diag \"%ws\" for the whole walk, or --force to try anyway\n",
                   match.c_str());
         return 4;
      }
      sayAlways("no control interface reached for \"%ws\" (%d endpoint(s))\n",
                match.c_str(), unreachable);
      sayAlways("run --diag \"%ws\" for the whole walk\n", match.c_str());
      return 4;
   }

   // The driver acknowledges before the link has gone.  Reconnects
   // measured on real hardware took between 3 and 13 seconds, so poll
   // rather than assuming.
   say("%d request(s) accepted, waiting up to %d s for the state to change\n",
       accepted, timeoutSeconds);

   DWORD deadline = GetTickCount() + (DWORD) timeoutSeconds * 1000;
   for (;;)
   {
      int reached = 0;
      int checked = 0;

      for (size_t i = 0; i < selected.size(); i++)
      {
         // A stale endpoint from an earlier pairing keeps the device's
         // container id, so it is selected with the live ones.  It was
         // NOTPRESENT before we started and will stay that way, so
         // waiting for it to reach the wanted state would turn every
         // run into a timeout.
         if (selected[i].state == DEVICE_STATE_NOTPRESENT &&
             wantedState != DEVICE_STATE_NOTPRESENT)
         {
            continue;
         }

         DWORD state = getEndpointState(pEnumerator, selected[i].id);
         if (state != 0)
         {
            checked++;
            if (state == wantedState)
            {
               reached++;
            }
         }
      }

      if (checked > 0 && reached == checked)
      {
         sayAlways("%s: all %d endpoint(s) now %s\n",
                   action, reached, stateName(wantedState));
         return 0;
      }

      if ((long) (GetTickCount() - deadline) >= 0)
      {
         sayAlways("%s: timed out after %d s, %d of %d endpoint(s) reached %s\n",
                   action, timeoutSeconds, reached, checked,
                   stateName(wantedState));

         for (size_t i = 0; i < selected.size(); i++)
         {
            // A stale endpoint left behind by an earlier pairing keeps the
            // device's container id, so it is selected alongside the live
            // ones.  It was NOTPRESENT before we started and will stay so,
            // and waiting for it turns every run into a false timeout.
            if (selected[i].state == DEVICE_STATE_NOTPRESENT &&
                wantedState != DEVICE_STATE_NOTPRESENT)
            {
               continue;
            }

            DWORD state = getEndpointState(pEnumerator, selected[i].id);
               sayAlways("   %-11s %ws\n", stateName(state),
                         selected[i].name.c_str());
         }
         return 6;
      }

      Sleep(POLL_INTERVAL_MS);
   }
}

// ---------------------------------------------------------------------------

static void usage()
{
   printf("btaudio_ctl -- connect and disconnect Bluetooth audio devices\n\n");
   printf("  --list                 every audio endpoint, with its state and\n");
   printf("                         whether it accepts Bluetooth control\n");
   printf("  --diag <name>          the whole topology walk for one device,\n");
   printf("                         with every HRESULT.  Run this when --list\n");
   printf("                         says something unexpected.\n");
   printf("  --status <name>        state of the endpoints matching <name>\n");
   printf("  --disconnect <name>    disconnect, leaving the pairing intact\n");
   printf("  --connect <name>       reconnect\n");
   printf("  --timeout <seconds>    how long to wait for the state to change\n");
   printf("                         (default %d)\n", DEFAULT_TIMEOUT_SECONDS);
   printf("  --force                send the request even if the endpoint did\n");
   printf("                         not admit to speaking Bluetooth\n");
   printf("  --verbose              connector counts and adapter device ids\n");
   printf("  --quiet                only the final result line\n\n");
   printf("<name> is any part of the endpoint name, case insensitive.\n");
   printf("All endpoints of the matched device are acted on together;\n");
   printf("Windows keeps the link until every one of them is disconnected.\n\n");
   printf("Exit: 0 done, 1 bad args, 2 no COM, 3 no match, 4 no control,\n");
   printf("      5 request failed, 6 timed out, 7 already in that state.\n");
   fflush(stdout);
}

int wmain(int argc, wchar_t** argv)
{
   std::wstring match;
   int  timeoutSeconds = DEFAULT_TIMEOUT_SECONDS;
   bool doListMode = false;
   bool doDiagMode = false;
   bool doStatusMode = false;
   bool doConnect = false;
   bool doDisconnect = false;
   bool force = false;

   for (int i = 1; i < argc; i++)
   {
      if (_wcsicmp(argv[i], L"--help") == 0 || _wcsicmp(argv[i], L"-h") == 0)
      {
         usage();
         return 0;
      }
      else if (_wcsicmp(argv[i], L"--list") == 0)
      {
         doListMode = true;
      }
      else if (_wcsicmp(argv[i], L"--quiet") == 0)
      {
         g_quiet = 1;
      }
      else if (_wcsicmp(argv[i], L"--verbose") == 0)
      {
         g_verbose = 1;
      }
      else if (_wcsicmp(argv[i], L"--force") == 0)
      {
         force = true;
      }
      else if (_wcsicmp(argv[i], L"--diag") == 0 && i + 1 < argc)
      {
         doDiagMode = true;
         match = argv[++i];
      }
      else if (_wcsicmp(argv[i], L"--status") == 0 && i + 1 < argc)
      {
         doStatusMode = true;
         match = argv[++i];
      }
      else if (_wcsicmp(argv[i], L"--connect") == 0 && i + 1 < argc)
      {
         doConnect = true;
         match = argv[++i];
      }
      else if (_wcsicmp(argv[i], L"--disconnect") == 0 && i + 1 < argc)
      {
         doDisconnect = true;
         match = argv[++i];
      }
      else if (_wcsicmp(argv[i], L"--timeout") == 0 && i + 1 < argc)
      {
         timeoutSeconds = _wtoi(argv[++i]);
         if (timeoutSeconds < 1)
         {
            timeoutSeconds = 1;
         }
      }
      else
      {
         printf("unknown or incomplete argument: %ws\n\n", argv[i]);
         usage();
         return 1;
      }
   }

   int modes = (doListMode ? 1 : 0) + (doDiagMode ? 1 : 0) +
               (doStatusMode ? 1 : 0) + (doConnect ? 1 : 0) +
               (doDisconnect ? 1 : 0);
   if (modes == 0)
   {
      usage();
      return 1;
   }
   if (modes > 1)
   {
      printf("pick one of --list, --diag, --status, --connect, --disconnect\n");
      return 1;
   }

   HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
   if (FAILED(hr))
   {
      printf("ERROR: CoInitializeEx = 0x%08lX %s\n",
             (unsigned long) hr, hresultHint(hr));
      return 2;
   }

   IMMDeviceEnumerator* pEnumerator = NULL;
   hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                         __uuidof(IMMDeviceEnumerator), (void**) &pEnumerator);
   if (FAILED(hr) || pEnumerator == NULL)
   {
      printf("ERROR: MMDeviceEnumerator = 0x%08lX %s\n",
             (unsigned long) hr, hresultHint(hr));
      printf("       (is the Windows Audio service running?)\n");
      CoUninitialize();
      return 2;
   }

   int result = 1;
   if (doListMode)
   {
      result = doList(pEnumerator);
   }
   else if (doDiagMode)
   {
      result = doDiag(pEnumerator, match);
   }
   else if (doStatusMode)
   {
      result = doStatus(pEnumerator, match);
   }
   else
   {
      result = doConnectOrDisconnect(pEnumerator, match, doConnect,
                                     timeoutSeconds, force);
   }

   pEnumerator->Release();
   CoUninitialize();
   return result;
}

