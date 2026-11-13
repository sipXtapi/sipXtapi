//
// Copyright (C) 2026 SIPez LLC  All rights reserved.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// Test-only crash and hang reporting. See SipxPortUnitTestFaultReport.h.

// APPLICATION INCLUDES
// First: on Windows the sipX headers pull in winsock2.h, which must come
// before windows.h.
#include <SipxPortUnitTestEnvironment.h>
#include <SipxPortUnitTestClass.h>
#include <SipxPortUnitTestFaultReport.h>

// SYSTEM INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#  include <tlhelp32.h>
#  include <dbghelp.h>
#  pragma comment(lib, "dbghelp.lib")
#else
#  include <pthread.h>
#  include <unistd.h>
#endif

// CONSTANTS
#define FAULT_MAX_FRAMES   48
#define FAULT_MAX_REPORTS  3

// Print and flush at once: the process may be about to die.
static void faultPrint(const char* text)
{
   fputs(text, stdout);
   fflush(stdout);
}

// "Class::method" of the test running now, or "(no test running)".
static void currentTestName(char* buf, size_t len)
{
   SipxPortUnitTestClass* pTest = SipxPortUnitTestEnvironment::getCurrentTestClass();
   if (pTest == NULL)
   {
      snprintf(buf, len, "(no test running)");
      return;
   }
   const char* cls = pTest->getClassName();
   const char* method = pTest->getCurrentMethodName();
   const char* file = pTest->getTestPointFilename();
   snprintf(buf, len, "%s::%s (last test point in %s)",
            cls ? cls : "?", method ? method : "?",
            (file && *file) ? file : "(none yet)");
}

#ifdef _WIN32 // [

static CRITICAL_SECTION sDbgLock;   // dbghelp is not thread safe
static volatile LONG sCrashReports = 0;

#if defined(_M_X64) || defined(_M_AMD64)
#  define FAULT_MACHINE IMAGE_FILE_MACHINE_AMD64
#  define FAULT_PC(c)    (c).Rip
#  define FAULT_FRAME(c) (c).Rbp
#  define FAULT_STACK(c) (c).Rsp
#else
#  define FAULT_MACHINE IMAGE_FILE_MACHINE_I386
#  define FAULT_PC(c)    (c).Eip
#  define FAULT_FRAME(c) (c).Ebp
#  define FAULT_STACK(c) (c).Esp
#endif

// Walk a stack from a context (a copy: StackWalk64 modifies it). The
// caller holds sDbgLock. Returns the number of program counters stored.
static int captureStack(HANDLE thread, CONTEXT ctx, DWORD64* pcs, int maxPcs)
{
   STACKFRAME64 frame;
   memset(&frame, 0, sizeof(frame));
   frame.AddrPC.Offset = FAULT_PC(ctx);
   frame.AddrPC.Mode = AddrModeFlat;
   frame.AddrFrame.Offset = FAULT_FRAME(ctx);
   frame.AddrFrame.Mode = AddrModeFlat;
   frame.AddrStack.Offset = FAULT_STACK(ctx);
   frame.AddrStack.Mode = AddrModeFlat;

   int n = 0;
   while (n < maxPcs
          && StackWalk64(FAULT_MACHINE, GetCurrentProcess(), thread, &frame, &ctx,
                         NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)
          && frame.AddrPC.Offset != 0)
   {
      pcs[n++] = frame.AddrPC.Offset;
   }
   return n;
}

// Print "  N] module!symbol+0xoff  file:line" per frame. Holds sDbgLock.
static void printStack(const DWORD64* pcs, int n)
{
   HANDLE process = GetCurrentProcess();
   char symBuf[sizeof(SYMBOL_INFO) + 256];
   char line[640];
   for (int i = 0; i < n; i++)
   {
      SYMBOL_INFO* pSym = (SYMBOL_INFO*)symBuf;
      memset(symBuf, 0, sizeof(symBuf));
      pSym->SizeOfStruct = sizeof(SYMBOL_INFO);
      pSym->MaxNameLen = 255;

      IMAGEHLP_MODULE64 mod;
      memset(&mod, 0, sizeof(mod));
      mod.SizeOfStruct = sizeof(mod);
      const char* modName = SymGetModuleInfo64(process, pcs[i], &mod) ? mod.ModuleName : "?";

      DWORD64 symOff = 0;
      if (SymFromAddr(process, pcs[i], &symOff, pSym))
      {
         snprintf(line, sizeof(line), "  %2d] %s!%s+0x%llx", i, modName, pSym->Name,
                  (unsigned long long)symOff);
      }
      else
      {
         snprintf(line, sizeof(line), "  %2d] %s!0x%llx", i, modName,
                  (unsigned long long)pcs[i]);
      }
      faultPrint(line);

      IMAGEHLP_LINE64 src;
      memset(&src, 0, sizeof(src));
      src.SizeOfStruct = sizeof(src);
      DWORD lineOff = 0;
      if (SymGetLineFromAddr64(process, pcs[i], &lineOff, &src))
      {
         snprintf(line, sizeof(line), "  %s:%lu", src.FileName, (unsigned long)src.LineNumber);
         faultPrint(line);
      }
      faultPrint("\n");
   }
}

static bool isFatalCode(DWORD code)
{
   switch (code)
   {
   case EXCEPTION_ACCESS_VIOLATION:
   case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
   case EXCEPTION_DATATYPE_MISALIGNMENT:
   case EXCEPTION_FLT_DIVIDE_BY_ZERO:
   case EXCEPTION_ILLEGAL_INSTRUCTION:
   case EXCEPTION_IN_PAGE_ERROR:
   case EXCEPTION_INT_DIVIDE_BY_ZERO:
   case EXCEPTION_PRIV_INSTRUCTION:
   case EXCEPTION_STACK_OVERFLOW:
   case EXCEPTION_NONCONTINUABLE_EXCEPTION:
   case 0xC0000374: // STATUS_HEAP_CORRUPTION
      return true;
   default:
      return false;
   }
}

static LONG CALLBACK faultVectoredHandler(PEXCEPTION_POINTERS pInfo)
{
   DWORD code = pInfo->ExceptionRecord->ExceptionCode;
   if (!isFatalCode(code)
       || InterlockedIncrement(&sCrashReports) > FAULT_MAX_REPORTS)
   {
      return EXCEPTION_CONTINUE_SEARCH;
   }

   char test[384];
   char head[640];
   currentTestName(test, sizeof(test));
   snprintf(head, sizeof(head),
            "\nFAULT: exception 0x%08lX at %p in thread %lu while running %s\n"
            "FAULT: (seen first-chance: if nothing else follows, it was handled)\n",
            (unsigned long)code, pInfo->ExceptionRecord->ExceptionAddress,
            (unsigned long)GetCurrentThreadId(), test);
   faultPrint(head);

   // A stack overflow leaves too little stack to walk safely.
   if (code != EXCEPTION_STACK_OVERFLOW)
   {
      DWORD64 pcs[FAULT_MAX_FRAMES];
      EnterCriticalSection(&sDbgLock);
      SymRefreshModuleList(GetCurrentProcess());
      int n = captureStack(GetCurrentThread(), *pInfo->ContextRecord, pcs, FAULT_MAX_FRAMES);
      printStack(pcs, n);
      LeaveCriticalSection(&sDbgLock);
   }
   faultPrint("FAULT: end of stack\n");
   return EXCEPTION_CONTINUE_SEARCH;
}

// Write a full minidump of this process. Returns TRUE on success.
static BOOL writeHangDump(int stallNumber, char* path, size_t len)
{
   const char* dir = getenv("SIPX_TEST_DUMP_DIR");
   snprintf(path, len, "%s\\sipx_hang_%lu_%d.dmp", (dir && *dir) ? dir : ".",
            (unsigned long)GetCurrentProcessId(), stallNumber);
   HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL, NULL);
   if (file == INVALID_HANDLE_VALUE)
   {
      return FALSE;
   }
   EnterCriticalSection(&sDbgLock);
   BOOL ok = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file,
                               (MINIDUMP_TYPE)(MiniDumpWithFullMemory
                                               | MiniDumpWithHandleData
                                               | MiniDumpWithThreadInfo
                                               | MiniDumpWithUnloadedModules),
                               NULL, NULL, NULL);
   LeaveCriticalSection(&sDbgLock);
   CloseHandle(file);
   return ok;
}

// Print every other thread's stack. Each thread is suspended only while
// its stack is walked; symbols are looked up after it resumes, so a
// suspended thread holding the heap lock cannot block the lookups.
static void printAllStacks()
{
   DWORD self = GetCurrentThreadId();
   DWORD pid = GetCurrentProcessId();
   HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
   if (snap == INVALID_HANDLE_VALUE)
   {
      faultPrint("HANG: cannot list threads\n");
      return;
   }

   // Load every module's symbols before suspending anything.
   EnterCriticalSection(&sDbgLock);
   SymRefreshModuleList(GetCurrentProcess());
   LeaveCriticalSection(&sDbgLock);

   THREADENTRY32 entry;
   entry.dwSize = sizeof(entry);
   for (BOOL more = Thread32First(snap, &entry); more; more = Thread32Next(snap, &entry))
   {
      if (entry.th32OwnerProcessID != pid || entry.th32ThreadID == self)
      {
         entry.dwSize = sizeof(entry);
         continue;
      }
      HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT
                                 | THREAD_QUERY_INFORMATION, FALSE, entry.th32ThreadID);
      if (thread == NULL)
      {
         entry.dwSize = sizeof(entry);
         continue;
      }

      DWORD64 pcs[FAULT_MAX_FRAMES];
      int n = 0;
      char head[96];
      snprintf(head, sizeof(head), "HANG: thread %lu\n", (unsigned long)entry.th32ThreadID);
      faultPrint(head);

      EnterCriticalSection(&sDbgLock);
      if (SuspendThread(thread) != (DWORD)-1)
      {
         CONTEXT ctx;
         memset(&ctx, 0, sizeof(ctx));
         ctx.ContextFlags = CONTEXT_FULL;
         if (GetThreadContext(thread, &ctx))
         {
            n = captureStack(thread, ctx, pcs, FAULT_MAX_FRAMES);
         }
         ResumeThread(thread);
      }
      printStack(pcs, n);
      LeaveCriticalSection(&sDbgLock);

      CloseHandle(thread);
      entry.dwSize = sizeof(entry);
   }
   CloseHandle(snap);
}

static DWORD WINAPI watchdogThread(LPVOID pArg)
{
   int limitSec = (int)(intptr_t)pArg;
   long lastProgress = SipxPortUnitTestEnvironment::getProgressCount();
   int stalledSec = 0;
   int stallNumber = 0;
   bool reported = false;

   for (;;)
   {
      Sleep(1000);
      long progress = SipxPortUnitTestEnvironment::getProgressCount();
      if (progress != lastProgress)
      {
         lastProgress = progress;
         stalledSec = 0;
         reported = false;
         continue;
      }
      if (reported || ++stalledSec < limitSec)
      {
         continue;
      }
      reported = true;
      stallNumber++;

      char test[384];
      char text[1024];
      currentTestName(test, sizeof(test));
      snprintf(text, sizeof(text),
               "\nHANG: no test progress for %d s while running %s\n", limitSec, test);
      faultPrint(text);

      char path[MAX_PATH];
      if (writeHangDump(stallNumber, path, sizeof(path)))
      {
         snprintf(text, sizeof(text), "HANG: full dump written to %s\n", path);
      }
      else
      {
         snprintf(text, sizeof(text), "HANG: could not write dump %s (error %lu)\n",
                  path, (unsigned long)GetLastError());
      }
      faultPrint(text);

      printAllStacks();
      faultPrint("HANG: end of thread stacks\n");
   }
   return 0;
}

void sipxUnitInstallFaultReporting()
{
   static bool installed = false;
   if (installed)
   {
      return;
   }
   installed = true;

   InitializeCriticalSection(&sDbgLock);

   // Symbols come from the PDBs next to the executable (and its DLLs).
   char exeDir[MAX_PATH];
   DWORD len = GetModuleFileNameA(NULL, exeDir, sizeof(exeDir));
   char* slash = (len > 0 && len < sizeof(exeDir)) ? strrchr(exeDir, '\\') : NULL;
   if (slash)
   {
      *slash = '\0';
   }
   SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS
                 | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_NO_PROMPTS);
   SymInitialize(GetCurrentProcess(), slash ? exeDir : NULL, TRUE);

   AddVectoredExceptionHandler(1, faultVectoredHandler);

   const char* watch = getenv("SIPX_TEST_WATCHDOG_SEC");
   int limitSec = watch ? atoi(watch) : 0;
   if (limitSec > 0)
   {
      HANDLE thread = CreateThread(NULL, 0, watchdogThread,
                                   (LPVOID)(intptr_t)limitSec, 0, NULL);
      if (thread)
      {
         CloseHandle(thread);
      }
   }
}

#else // _WIN32 ][

static void* watchdogThread(void* pArg)
{
   int limitSec = (int)(intptr_t)pArg;
   long lastProgress = SipxPortUnitTestEnvironment::getProgressCount();
   int stalledSec = 0;
   bool reported = false;

   for (;;)
   {
      sleep(1);
      long progress = SipxPortUnitTestEnvironment::getProgressCount();
      if (progress != lastProgress)
      {
         lastProgress = progress;
         stalledSec = 0;
         reported = false;
         continue;
      }
      if (reported || ++stalledSec < limitSec)
      {
         continue;
      }
      reported = true;

      char test[384];
      char text[640];
      currentTestName(test, sizeof(test));
      snprintf(text, sizeof(text),
               "\nHANG: no test progress for %d s while running %s\n"
               "HANG: thread stacks are not available from inside the process "
               "on this platform; attach gdb -p %d\n",
               limitSec, test, (int)getpid());
      faultPrint(text);
   }
   return NULL;
}

void sipxUnitInstallFaultReporting()
{
   static bool installed = false;
   if (installed)
   {
      return;
   }
   installed = true;

   // Crashes on the main thread are already reported, with a backtrace,
   // by SipxPortUnitTestEnvironment::signalHandler.
   const char* watch = getenv("SIPX_TEST_WATCHDOG_SEC");
   int limitSec = watch ? atoi(watch) : 0;
   if (limitSec > 0)
   {
      pthread_t thread;
      if (pthread_create(&thread, NULL, watchdogThread, (void*)(intptr_t)limitSec) == 0)
      {
         pthread_detach(thread);
      }
   }
}

#endif // _WIN32 ]
