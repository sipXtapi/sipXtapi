//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// audio_default.cpp -- save, set and restore the Windows default audio
// devices from the command line.
//
// Why this exists
// ---------------
// Every sipXtapi instance opens the Windows default capture and render
// devices when it starts, before a test selects the device it wants. On
// the laptop bench the defaults are the headset used for real calls, and
// Windows may also promote the bench Bluetooth device to default each
// time it reconnects. So a bench run saves the defaults, points them at
// a device nobody needs, and puts them back afterwards -- including when
// the run fails.
//
// Windows keeps three defaults per direction (console, multimedia,
// communications), six in all. Reading them is documented
// (IMMDeviceEnumerator::GetDefaultAudioEndpoint). Setting them is not:
// Windows has no public API for it. Every tool that does it, including
// the Sound control panel, goes through the undocumented IPolicyConfig
// COM interface of CPolicyConfigClient. Its layout has been stable from
// Windows 7 through 10; only SetDefaultEndpoint is called here, and the
// methods before it are declared only to keep the vtable slots right.
//
// Build, from ~/dev/sipXtapi/sipXmediaLib/src/test/mp in a Cygwin ssh
// shell on the build VM:
//
//   V=$(cygpath -d '/cygdrive/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars64.bat') && cmd /s /c "call $V && cl /nologo /EHsc /W3 audio_default.cpp ole32.lib"
//
// No administrator rights are needed.
//
// Usage:
//   audio_default.exe --list
//   audio_default.exe --save <file>
//   audio_default.exe --set capture|render <name match>
//   audio_default.exe --restore <file>
//
// --set makes the one ACTIVE endpoint whose friendly name contains
// <name match> the default for all three roles of that direction. It
// refuses when nothing or more than one endpoint matches.
//
// --save writes the six current defaults (endpoint ids) to <file>.
// --restore sets each one back, keeps going past failures, and reports
// every role it could not restore. A saved default that is gone (an
// unplugged headset, say) cannot be restored; that is reported, not
// fatal to the other roles.
//
// Exit codes:
//   0  did what was asked
//   1  bad arguments
//   2  COM would not start, or the policy interface is unavailable
//   3  no endpoint matched
//   4  more than one endpoint matched
//   5  setting a default failed (for --restore: at least one role)
//   6  the save file could not be written or read
//

#define _WIN32_WINNT 0x0600
#define INITGUID
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <vector>
#include <string>

// ------------------------------------------------- undocumented interface

interface DECLSPEC_UUID("f8679f50-850a-41cf-9c72-430f290290c8") IPolicyConfig;
class DECLSPEC_UUID("870af99c-171d-4f9e-af0d-e63df40c2bc9") CPolicyConfigClient;

interface IPolicyConfig : public IUnknown
{
   // Slots before SetDefaultEndpoint; never called here.
   virtual HRESULT STDMETHODCALLTYPE GetMixFormat(PCWSTR, void**) = 0;
   virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(PCWSTR, INT, void**) = 0;
   virtual HRESULT STDMETHODCALLTYPE ResetDeviceFormat(PCWSTR) = 0;
   virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(PCWSTR, void*, void*) = 0;
   virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(PCWSTR, INT, void*, void*) = 0;
   virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(PCWSTR, void*) = 0;
   virtual HRESULT STDMETHODCALLTYPE GetShareMode(PCWSTR, void*) = 0;
   virtual HRESULT STDMETHODCALLTYPE SetShareMode(PCWSTR, void*) = 0;
   virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(PCWSTR, const PROPERTYKEY&, PROPVARIANT*) = 0;
   virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(PCWSTR, const PROPERTYKEY&, PROPVARIANT*) = 0;
   // The one this tool uses.
   virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(PCWSTR wszDeviceId, ERole eRole) = 0;
   virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(PCWSTR, INT) = 0;
};

// ---------------------------------------------------------------- helpers

static const EDataFlow kFlows[] = { eRender, eCapture };
static const ERole kRoles[] = { eConsole, eMultimedia, eCommunications };

static const char* flowName(EDataFlow f)
{
   return f == eRender ? "render" : "capture";
}

static const char* roleName(ERole r)
{
   return r == eConsole ? "console" : (r == eMultimedia ? "multimedia" : "communications");
}

static std::wstring friendlyName(IMMDevice* dev)
{
   std::wstring name;
   IPropertyStore* props = NULL;
   if (SUCCEEDED(dev->OpenPropertyStore(STGM_READ, &props)))
   {
      PROPVARIANT v;
      PropVariantInit(&v);
      if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &v)) && v.vt == VT_LPWSTR)
      {
         name = v.pwszVal;
      }
      PropVariantClear(&v);
      props->Release();
   }
   return name;
}

static std::wstring deviceId(IMMDevice* dev)
{
   std::wstring id;
   LPWSTR p = NULL;
   if (SUCCEEDED(dev->GetId(&p)) && p)
   {
      id = p;
      CoTaskMemFree(p);
   }
   return id;
}

/// Current default for one flow and role; empty id when there is none.
static void currentDefault(IMMDeviceEnumerator* en, EDataFlow f, ERole r,
                           std::wstring& id, std::wstring& name)
{
   id.clear();
   name.clear();
   IMMDevice* dev = NULL;
   if (SUCCEEDED(en->GetDefaultAudioEndpoint(f, r, &dev)) && dev)
   {
      id = deviceId(dev);
      name = friendlyName(dev);
      dev->Release();
   }
}

static std::wstring toWide(const char* s)
{
   int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
   if (n <= 0) return std::wstring();
   std::vector<wchar_t> buf(n);
   MultiByteToWideChar(CP_UTF8, 0, s, -1, &buf[0], n);
   return std::wstring(&buf[0]);
}

static std::string toUtf8(const std::wstring& s)
{
   int n = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, NULL, 0, NULL, NULL);
   if (n <= 0) return std::string();
   std::vector<char> buf(n);
   WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, &buf[0], n, NULL, NULL);
   return std::string(&buf[0]);
}

static int setAllRoles(IPolicyConfig* policy, const std::wstring& id)
{
   int failures = 0;
   for (size_t i = 0; i < sizeof(kRoles) / sizeof(kRoles[0]); i++)
   {
      HRESULT hr = policy->SetDefaultEndpoint(id.c_str(), kRoles[i]);
      if (FAILED(hr))
      {
         printf("FAILED %s: SetDefaultEndpoint hr=0x%08lx\n", roleName(kRoles[i]),
                (unsigned long)hr);
         failures++;
      }
   }
   return failures;
}

// ---------------------------------------------------------------- actions

static int doList(IMMDeviceEnumerator* en)
{
   for (size_t f = 0; f < 2; f++)
   {
      for (size_t r = 0; r < 3; r++)
      {
         std::wstring id, name;
         currentDefault(en, kFlows[f], kRoles[r], id, name);
         printf("%-8s %-15s %s  %s\n", flowName(kFlows[f]), roleName(kRoles[r]),
                id.empty() ? "(none)" : toUtf8(name).c_str(),
                id.empty() ? "" : toUtf8(id).c_str());
      }
   }
   return 0;
}

static int doSave(IMMDeviceEnumerator* en, const char* path)
{
   FILE* fp = fopen(path, "w");
   if (!fp)
   {
      printf("FAILED: cannot write %s\n", path);
      return 6;
   }
   fprintf(fp, "# audio_default saved defaults: flow role id (name follows #)\n");
   for (size_t f = 0; f < 2; f++)
   {
      for (size_t r = 0; r < 3; r++)
      {
         std::wstring id, name;
         currentDefault(en, kFlows[f], kRoles[r], id, name);
         fprintf(fp, "%s %s %s # %s\n", flowName(kFlows[f]), roleName(kRoles[r]),
                 id.empty() ? "none" : toUtf8(id).c_str(), toUtf8(name).c_str());
      }
   }
   if (fclose(fp) != 0)
   {
      printf("FAILED: error writing %s\n", path);
      return 6;
   }
   printf("SAVED %s\n", path);
   return 0;
}

static int doSet(IMMDeviceEnumerator* en, IPolicyConfig* policy,
                 const char* flowArg, const wchar_t* match)
{
   EDataFlow flow;
   if (strcmp(flowArg, "render") == 0) flow = eRender;
   else if (strcmp(flowArg, "capture") == 0) flow = eCapture;
   else
   {
      printf("FAILED: direction must be capture or render, not %s\n", flowArg);
      return 1;
   }

   IMMDeviceCollection* coll = NULL;
   if (FAILED(en->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &coll)) || !coll)
   {
      printf("FAILED: cannot enumerate %s endpoints\n", flowArg);
      return 2;
   }
   UINT count = 0;
   coll->GetCount(&count);
   std::vector<std::wstring> ids, names;
   for (UINT i = 0; i < count; i++)
   {
      IMMDevice* dev = NULL;
      if (SUCCEEDED(coll->Item(i, &dev)) && dev)
      {
         std::wstring name = friendlyName(dev);
         if (wcsstr(name.c_str(), match))
         {
            ids.push_back(deviceId(dev));
            names.push_back(name);
         }
         dev->Release();
      }
   }
   coll->Release();

   if (ids.empty())
   {
      printf("FAILED: no ACTIVE %s endpoint matches '%s'\n", flowArg,
             toUtf8(match).c_str());
      return 3;
   }
   if (ids.size() > 1)
   {
      printf("FAILED: %u ACTIVE %s endpoints match '%s':\n", (unsigned)ids.size(),
             flowArg, toUtf8(match).c_str());
      for (size_t i = 0; i < names.size(); i++)
      {
         printf("    %s\n", toUtf8(names[i]).c_str());
      }
      return 4;
   }
   if (setAllRoles(policy, ids[0]) != 0)
   {
      return 5;
   }
   printf("SET %s default (all roles): %s\n", flowArg, toUtf8(names[0]).c_str());
   return 0;
}

static int doRestore(IMMDeviceEnumerator* en, IPolicyConfig* policy, const char* path)
{
   FILE* fp = fopen(path, "r");
   if (!fp)
   {
      printf("FAILED: cannot read %s\n", path);
      return 6;
   }
   int failures = 0;
   int restored = 0;
   char line[1024];
   while (fgets(line, sizeof(line), fp))
   {
      line[strcspn(line, "\r\n")] = 0;
      if (line[0] == '#' || line[0] == 0) continue;
      char flowStr[32], roleStr[32], idStr[512];
      if (sscanf(line, "%31s %31s %511s", flowStr, roleStr, idStr) != 3) continue;

      ERole role;
      if (strcmp(roleStr, "console") == 0) role = eConsole;
      else if (strcmp(roleStr, "multimedia") == 0) role = eMultimedia;
      else if (strcmp(roleStr, "communications") == 0) role = eCommunications;
      else continue;
      EDataFlow flow = (strcmp(flowStr, "render") == 0) ? eRender : eCapture;

      if (strcmp(idStr, "none") == 0)
      {
         printf("SKIP %s %s: there was no default\n", flowStr, roleStr);
         continue;
      }

      std::wstring id = toWide(idStr);
      HRESULT hr = policy->SetDefaultEndpoint(id.c_str(), role);
      std::wstring nowId, nowName;
      currentDefault(en, flow, role, nowId, nowName);
      if (FAILED(hr) || nowId != id)
      {
         const char* hash = strchr(line, '#');
         printf("FAILED %s %s: could not restore %s (hr=0x%08lx)\n", flowStr, roleStr,
                hash ? hash + 2 : idStr, (unsigned long)hr);
         failures++;
      }
      else
      {
         printf("RESTORED %s %s: %s\n", flowStr, roleStr, toUtf8(nowName).c_str());
         restored++;
      }
   }
   fclose(fp);
   printf("%s: %d restored, %d failed\n", failures ? "RESTORE INCOMPLETE" : "RESTORE DONE",
          restored, failures);
   return failures ? 5 : 0;
}

// ------------------------------------------------------------------- main

static int usage(void)
{
   printf("usage: audio_default.exe --list\n"
          "       audio_default.exe --save <file>\n"
          "       audio_default.exe --set capture|render <name match>\n"
          "       audio_default.exe --restore <file>\n");
   return 1;
}

int wmain(int argc, wchar_t** argv)
{
   if (argc < 2) return usage();
   std::string action = toUtf8(argv[1]);
   if (!((action == "--list" && argc == 2) ||
         ((action == "--save" || action == "--restore") && argc == 3) ||
         (action == "--set" && argc == 4)))
   {
      return usage();
   }

   if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
   {
      printf("FAILED: COM would not start\n");
      return 2;
   }

   int rc = 2;
   IMMDeviceEnumerator* en = NULL;
   IPolicyConfig* policy = NULL;
   if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                               __uuidof(IMMDeviceEnumerator), (void**)&en)) || !en)
   {
      printf("FAILED: cannot create the device enumerator\n");
   }
   else if (action == "--list")
   {
      rc = doList(en);
   }
   else if (action == "--save")
   {
      rc = doSave(en, toUtf8(argv[2]).c_str());
   }
   else if (FAILED(CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_ALL,
                                    __uuidof(IPolicyConfig), (void**)&policy)) || !policy)
   {
      printf("FAILED: the policy interface is unavailable on this Windows version\n");
   }
   else if (action == "--set")
   {
      rc = doSet(en, policy, toUtf8(argv[2]).c_str(), argv[3]);
   }
   else
   {
      rc = doRestore(en, policy, toUtf8(argv[2]).c_str());
   }

   if (policy) policy->Release();
   if (en) en->Release();
   CoUninitialize();
   return rc;
}

