//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// swdevice_audio.cpp -- create and destroy a software-enumerated audio
// devnode whose lifetime is owned by this process.
//
// Unlike an INF-installed root-enumerated device, a devnode created with
// SwDeviceCreate is destroyed unconditionally when its handle closes.  No
// IRP_MN_QUERY_REMOVE_DEVICE is sent, so portcls never gets the chance to
// veto the removal while a stream is open.  This is believed to be the
// mechanism by which third-party virtual audio drivers (e.g. Sanas) drop
// their endpoints out from under live WinMM handles.
//
// The driver package must already be in the driver store, e.g.
//    pnputil /add-driver vbMmeCable64_win10.inf /install
//
// Build:
//    cl /EHsc /W3 swdevice_audio.cpp SwDevice.lib cfgmgr32.lib advapi32.lib
//
// Usage:
//    swdevice_audio.exe --hold 60
//    swdevice_audio.exe --event Global\sipx_swdev_kill
//    swdevice_audio.exe --hwid VBAudioVACWDM --instance-id sipx_vaudio_0 --hold 30
//
// Signal the event from PowerShell with:
//    $e = [System.Threading.EventWaitHandle]::OpenExisting("Global\sipx_swdev_kill")
//    $e.Set()
//

#define _WIN32_WINNT 0x0602

#include <windows.h>
#include <swdevice.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define DEFAULT_HWID        L"VBAudioVACWDM"
#define DEFAULT_ENUMERATOR  L"SIPXVAUDIO"
#define DEFAULT_INSTANCE_ID L"sipx_vaudio_0"
#define DEFAULT_EVENT_NAME  L"Global\\sipx_swdev_kill"
#define DEFAULT_DESC        L"sipXtapi test virtual audio device"

struct CreateContext
{
   HANDLE  completed;
   HRESULT result;
   wchar_t instanceId[512];
};

static void stamp(const char* msg)
{
   SYSTEMTIME st;
   GetLocalTime(&st);
   printf("[%02d:%02d:%02d.%03d] %s\n",
          st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, msg);
   fflush(stdout);
}

static VOID WINAPI creationCallback(HSWDEVICE hSwDevice,
                                    HRESULT hrCreateResult,
                                    PVOID pContext,
                                    PCWSTR pszDeviceInstanceId)
{
   CreateContext* ctx = (CreateContext*) pContext;

   (void) hSwDevice;

   ctx->result = hrCreateResult;
   ctx->instanceId[0] = L'\0';
   if (SUCCEEDED(hrCreateResult) && pszDeviceInstanceId != NULL)
   {
      wcsncpy_s(ctx->instanceId,
                sizeof(ctx->instanceId) / sizeof(wchar_t),
                pszDeviceInstanceId,
                _TRUNCATE);
   }
   SetEvent(ctx->completed);
}

static int isElevated()
{
   BOOL  elevated = FALSE;
   HANDLE token = NULL;

   if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
   {
      TOKEN_ELEVATION te;
      DWORD returned = 0;
      if (GetTokenInformation(token, TokenElevation, &te, sizeof(te), &returned))
      {
         elevated = te.TokenIsElevated;
      }
      CloseHandle(token);
   }
   return elevated ? 1 : 0;
}

static void usage()
{
   printf("swdevice_audio -- create a process-owned audio devnode\n\n");
   printf("  --hwid <id>          hardware id to match (default %ws)\n", DEFAULT_HWID);
   printf("  --enumerator <name>  enumerator name (default %ws)\n", DEFAULT_ENUMERATOR);
   printf("  --instance-id <id>   instance id (default %ws)\n", DEFAULT_INSTANCE_ID);
   printf("  --desc <text>        device description\n");
   printf("  --hold <seconds>     hold the device this long, then destroy it\n");
   printf("  --event <name>       hold until this named event is signalled\n");
   printf("  --help               this text\n\n");
   printf("If neither --hold nor --event is given, holds until Enter is pressed.\n");
   fflush(stdout);
}

int wmain(int argc, wchar_t** argv)
{
   const wchar_t* hwid       = DEFAULT_HWID;
   const wchar_t* enumerator = DEFAULT_ENUMERATOR;
   const wchar_t* instanceId = DEFAULT_INSTANCE_ID;
   const wchar_t* desc       = DEFAULT_DESC;
   const wchar_t* eventName  = NULL;
   int            holdSecs   = 0;
   int            i;

   for (i = 1; i < argc; i++)
   {
      if (_wcsicmp(argv[i], L"--help") == 0)
      {
         usage();
         return 0;
      }
      else if (_wcsicmp(argv[i], L"--hwid") == 0 && i + 1 < argc)
      {
         hwid = argv[++i];
      }
      else if (_wcsicmp(argv[i], L"--enumerator") == 0 && i + 1 < argc)
      {
         enumerator = argv[++i];
      }
      else if (_wcsicmp(argv[i], L"--instance-id") == 0 && i + 1 < argc)
      {
         instanceId = argv[++i];
      }
      else if (_wcsicmp(argv[i], L"--desc") == 0 && i + 1 < argc)
      {
         desc = argv[++i];
      }
      else if (_wcsicmp(argv[i], L"--hold") == 0 && i + 1 < argc)
      {
         holdSecs = _wtoi(argv[++i]);
      }
      else if (_wcsicmp(argv[i], L"--event") == 0 && i + 1 < argc)
      {
         eventName = argv[++i];
      }
      else
      {
         printf("unknown argument: %ws\n\n", argv[i]);
         usage();
         return 2;
      }
   }

   if (!isElevated())
   {
      printf("ERROR: must be run elevated\n");
      return 3;
   }

   // Hardware id list is a multi-sz: one string, then an extra terminator.
   wchar_t hwidList[256];
   size_t  hwidLen = wcslen(hwid);
   if (hwidLen + 2 > sizeof(hwidList) / sizeof(wchar_t))
   {
      printf("ERROR: hardware id too long\n");
      return 2;
   }
   wmemcpy(hwidList, hwid, hwidLen);
   hwidList[hwidLen]     = L'\0';
   hwidList[hwidLen + 1] = L'\0';

   CreateContext ctx;
   memset(&ctx, 0, sizeof(ctx));
   ctx.result    = E_FAIL;
   ctx.completed = CreateEventW(NULL, TRUE, FALSE, NULL);
   if (ctx.completed == NULL)
   {
      printf("ERROR: CreateEvent failed, gle=%lu\n", GetLastError());
      return 4;
   }

   SW_DEVICE_CREATE_INFO info;
   memset(&info, 0, sizeof(info));
   info.cbSize               = sizeof(info);
   info.pszInstanceId        = instanceId;
   info.pszzHardwareIds      = hwidList;
   info.pszzCompatibleIds    = NULL;
   info.pContainerId         = NULL;
   info.CapabilityFlags      = SWDeviceCapabilitiesRemovable |
                               SWDeviceCapabilitiesSilentInstall |
                               SWDeviceCapabilitiesDriverRequired;
   info.pszDeviceDescription = desc;
   info.pszDeviceLocation    = NULL;
   info.pSecurityDescriptor  = NULL;

   printf("enumerator : %ws\n", enumerator);
   printf("hardware id: %ws\n", hwid);
   printf("instance id: %ws\n", instanceId);
   fflush(stdout);

   HSWDEVICE hDevice = NULL;
   stamp("SwDeviceCreate");
   HRESULT hr = SwDeviceCreate(enumerator,
                               L"HTREE\\ROOT\\0",
                               &info,
                               0,
                               NULL,
                               creationCallback,
                               &ctx,
                               &hDevice);
   if (FAILED(hr))
   {
      printf("ERROR: SwDeviceCreate failed hr=0x%08lX\n", (unsigned long) hr);
      CloseHandle(ctx.completed);
      return 5;
   }

   DWORD waited = WaitForSingleObject(ctx.completed, 30000);
   if (waited != WAIT_OBJECT_0)
   {
      printf("ERROR: creation callback did not fire within 30s\n");
      SwDeviceClose(hDevice);
      CloseHandle(ctx.completed);
      return 6;
   }

   if (FAILED(ctx.result))
   {
      printf("ERROR: device creation failed hr=0x%08lX\n",
             (unsigned long) ctx.result);
      printf("       (0x800701C1 / 0x800F0203 usually mean no matching driver in the store)\n");
      SwDeviceClose(hDevice);
      CloseHandle(ctx.completed);
      return 7;
   }

   printf("created    : %ws\n", ctx.instanceId);
   stamp("READY -- device is live, driver binding may take a moment");

   if (eventName != NULL)
   {
      HANDLE killEvent = CreateEventW(NULL, TRUE, FALSE, eventName);
      if (killEvent == NULL)
      {
         printf("ERROR: CreateEvent(%ws) failed, gle=%lu\n",
                eventName, GetLastError());
      }
      else
      {
         printf("waiting on event: %ws\n", eventName);
         fflush(stdout);
         WaitForSingleObject(killEvent, INFINITE);
         CloseHandle(killEvent);
      }
   }
   else if (holdSecs > 0)
   {
      printf("holding for %d seconds\n", holdSecs);
      fflush(stdout);
      Sleep((DWORD) holdSecs * 1000);
   }
   else
   {
      printf("press Enter to destroy the device\n");
      fflush(stdout);
      (void) getchar();
   }

   stamp("SwDeviceClose -- destroying devnode");
   SwDeviceClose(hDevice);
   stamp("destroyed");

   CloseHandle(ctx.completed);
   return 0;
}
