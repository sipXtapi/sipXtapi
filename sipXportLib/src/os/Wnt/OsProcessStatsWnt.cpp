//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Windows implementation of utlRscGetProcessStats (see utl/UtlRscTrace.h).

// SYSTEM INCLUDES
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
// psapi.lib covers both PSAPI_VERSION 1 and 2 (in 2, GetProcessMemoryInfo
// maps to K32GetProcessMemoryInfo in kernel32).
#pragma comment(lib, "psapi.lib")

// APPLICATION INCLUDES
#include "utl/UtlRscTrace.h"

void utlRscGetProcessStats(UtlRscProcessStats& stats)
{
   stats.handles = -1;
   stats.privateKB = -1;
   stats.threads = -1;

   DWORD handleCount = 0;
   if (GetProcessHandleCount(GetCurrentProcess(), &handleCount))
   {
      stats.handles = (long)handleCount;
   }

   PROCESS_MEMORY_COUNTERS_EX mem;
   ZeroMemory(&mem, sizeof(mem));
   mem.cb = sizeof(mem);
   if (GetProcessMemoryInfo(GetCurrentProcess(),
                            (PROCESS_MEMORY_COUNTERS*)&mem, sizeof(mem)))
   {
      stats.privateKB = (long)(mem.PrivateUsage / 1024);
   }

   // Toolhelp lists every thread in the system; count ours.
   HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
   if (snap != INVALID_HANDLE_VALUE)
   {
      DWORD pid = GetCurrentProcessId();
      long count = 0;
      THREADENTRY32 entry;
      entry.dwSize = sizeof(entry);
      if (Thread32First(snap, &entry))
      {
         do
         {
            if (entry.th32OwnerProcessID == pid)
            {
               count++;
            }
            entry.dwSize = sizeof(entry);
         } while (Thread32Next(snap, &entry));
      }
      CloseHandle(snap);
      stats.threads = count;
   }
}
