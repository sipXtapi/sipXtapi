//
// wavelock_probe.cpp - standalone WinMM lock/deadlock probe.
// Not part of the sipXtapi build.
//
// Answers, for input and for output:
//   1. Does a thread parked inside waveInAddBuffer / waveOutWrite block
//      waveInReset / waveOutReset?
//   2. Does removing the device underneath an open stream do the same?
//   3. After a wedged handle is abandoned, does a fresh open still deliver
//      real audio?
//
// Build (from a VS2019 x64 native tools prompt):
//   cl /EHsc /W3 wavelock_probe.cpp winmm.lib cfgmgr32.lib advapi32.lib
//
// Modes:
//   list      Print the WinMM device list and the MMDevice endpoint list
//             side by side, so the name-based join between them can be
//             checked. No device is opened.
//   suspend   Suspend the thread that recycles buffers, then time reset.
//             Repeatable on output; wedges the process on input.
//   sweep     Same, walking the settle delay to show the window shape.
//   reopen    Wedge one input context, abandon it, then ask whether fresh
//             opens still deliver real audio.
//   deferred  As reopen, but the wave call happens on our own thread
//             (the MpodWinMM pattern) instead of in the callback.
//   hold      Just hold the device open and report per second. For
//             observing an externally triggered device change.
//   parent    Full automated cycle: open, disable the USB parent devnode,
//             time the reset, re-enable, verify audio returns. Needs
//             elevation and --instance set to the PARENT devnode.
//
// Examples:
//   wavelock_probe --dir out --method suspend  --iterations 50
//   wavelock_probe --dir in  --method deferred --iterations 10
//   wavelock_probe --dir in  --method hold     --iterations 60
//   wavelock_probe --dir in  --method parent   --iterations 5 \
//                  --instance "USB\VID_0D8C&PID_0008\7&283D3F96&0&1"
//   wavelock_probe --method list
//
// Find the parent devnode (the USB composite device, NOT the MEDIA node):
//   Get-PnpDevice -Class USB | Where-Object { $_.InstanceId -like "*VID_xxxx*" }
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <cfgmgr32.h>
#include <stdio.h>
#include <string.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>

#define NUM_BUFFERS       8
#define SAMPLES_PER_FRAME 80          // 8 kHz, 10 ms, matches sipX defaults
#define BYTES_PER_FRAME   (SAMPLES_PER_FRAME * 2)
#define RESET_TIMEOUT_MS  3000
#define DEVICE_RETURN_MS  20000       // how long to wait for a re-enabled device
#define OPEN_TIMEOUT_MS   5000

// Globals used by the single-open modes (suspend, sweep, hold-output).
// The context modes use per-open InCtx instead, so that an abandoned
// worker can never touch a later iteration's state.
static HWAVEIN   g_hIn  = NULL;
static HWAVEOUT  g_hOut = NULL;
static WAVEHDR   g_hdr[NUM_BUFFERS];
static char      g_buf[NUM_BUFFERS][BYTES_PER_FRAME];
static volatile LONG  g_callbackTid = 0;
static volatile LONG  g_stopFeeding = 0;
static volatile LONG  g_freeMask    = 0;    // output: bit n set == header n free
static volatile LONG  g_resetIsInput = 0;
static volatile LONG  g_resetResult  = 0;
static HANDLE         g_resetThread  = NULL;
static HANDLE         g_feederThread = NULL;
static volatile LONG  g_feederTid = 0;
static volatile LONG  g_resetReturned = 0;
static volatile LONG  g_deferMode = 0;      // 1 == recycle on our own thread

static DWORD WINAPI resetProc(LPVOID);      // defined below, used early

static int g_deviceId = 0;   // WinMM device index; -1 == WAVE_MAPPER

// Milliseconds to wait for waveInReset / waveOutReset before abandoning it.
// Overridable so the real duration of a wedge can be measured rather than
// truncated by the harness. RESET_TIMEOUT_MS is the default.
static int g_resetTimeoutMs = RESET_TIMEOUT_MS;

// ---------------------------------------------------------------- privilege

static int isElevated(void)
{
   HANDLE tok = NULL;
   TOKEN_ELEVATION el;
   DWORD sz = sizeof(el);
   int result = 0;
   if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tok))
   {
      if (GetTokenInformation(tok, TokenElevation, &el, sizeof(el), &sz))
      {
         result = el.TokenIsElevated ? 1 : 0;
      }
      CloseHandle(tok);
   }
   return result;
}

// ---------------------------------------------------------------- per-open context

// One of these per waveInOpen. The callback receives it via dwInstance, so
// an abandoned worker can never touch a later iteration's handle, headers
// or event. Deliberately leaked when its worker wedges.
struct InCtx
{
   HWAVEIN  hIn;
   WAVEHDR  hdr[NUM_BUFFERS];
   char     buf[NUM_BUFFERS][BYTES_PER_FRAME];
   HANDLE   evt;
   HANDLE   worker;
   volatile LONG stop;
   volatile LONG tid;            // thread to suspend: worker if deferred,
                                 // else WinMM's callback thread
   volatile LONG inWaveCall;     // 1 while inside waveInAddBuffer
   volatile LONG mask;           // bit n set == header n needs re-adding
   volatile LONG dataCallbacks;  // WIM_DATA count
   volatile LONG nonEmpty;       // buffers with dwBytesRecorded > 0
   volatile LONG contentVaried;  // consecutive buffers whose content differs
   volatile LONG deferred;       // 1 == recycle on worker, 0 == in callback
   DWORD    lastSum;
};

// openInputCtx runs waveInOpen on a worker so a blocked open can be
// reported instead of hanging the harness. After a wedged waveInReset,
// waveInOpen has been observed to block indefinitely on any capture
// device in the process, including a different one.
struct OpenArgs
{
   InCtx*   ctx;
   int      deviceId;
   MMRESULT result;
};

static DWORD sumBuffer(const char* p, int n)
{
   DWORD s = 0;
   for (int i = 0; i < n; i++) s = (s * 31u) + (unsigned char)p[i];
   return s;
}

// MSDN permits only SetEvent, the Interlocked family and a short list of
// other calls from a wave callback. Calling another wave function here is
// documented to cause deadlock, which is what the non-deferred path does
// deliberately -- it mirrors MpidWinMM::processAudioInput.
static void CALLBACK waveInProcCtx(HWAVEIN hwi, UINT uMsg,
                                   DWORD_PTR inst, DWORD_PTR p1, DWORD_PTR p2)
{
   if (uMsg != WIM_DATA) return;
   InCtx* c = (InCtx*)inst;
   if (!c) return;

   WAVEHDR* h = (WAVEHDR*)p1;
   InterlockedIncrement(&c->dataCallbacks);

   if (h->dwBytesRecorded > 0)
   {
      InterlockedIncrement(&c->nonEmpty);
      DWORD s = sumBuffer(h->lpData, (int)h->dwBytesRecorded);
      if (c->lastSum != 0 && s != c->lastSum) InterlockedIncrement(&c->contentVaried);
      c->lastSum = s;
   }

   if (c->deferred)
   {
      InterlockedOr(&c->mask, 1 << (int)h->dwUser);
      SetEvent(c->evt);
   }
   else
   {
      InterlockedExchange(&c->tid, (LONG)GetCurrentThreadId());
      if (!c->stop)
      {
         InterlockedExchange(&c->inWaveCall, 1);
         waveInAddBuffer(hwi, h, sizeof(WAVEHDR));
         InterlockedExchange(&c->inWaveCall, 0);
      }
   }
}

static DWORD WINAPI openProc(LPVOID param)
{
   OpenArgs* a = (OpenArgs*)param;
   WAVEFORMATEX fmt;
   memset(&fmt, 0, sizeof(fmt));
   fmt.wFormatTag = WAVE_FORMAT_PCM;
   fmt.nChannels = 1;
   fmt.nSamplesPerSec = 8000;
   fmt.wBitsPerSample = 16;
   fmt.nBlockAlign = 2;
   fmt.nAvgBytesPerSec = 16000;
   a->result = waveInOpen(&a->ctx->hIn, (UINT)a->deviceId, &fmt,
                          (DWORD_PTR)waveInProcCtx, (DWORD_PTR)a->ctx,
                          CALLBACK_FUNCTION);
   return 0;
}

// Mirrors MpodWinMM::ThreadMMProc: the wave call happens on a thread we
// own rather than on WinMM's callback thread.
static DWORD WINAPI deferProcCtx(LPVOID param)
{
   InCtx* c = (InCtx*)param;
   InterlockedExchange(&c->tid, (LONG)GetCurrentThreadId());
   while (!c->stop)
   {
      WaitForSingleObject(c->evt, 50);
      if (c->stop) break;
      LONG mask = InterlockedExchange(&c->mask, 0);
      for (int n = 0; n < NUM_BUFFERS && !c->stop; n++)
      {
         if (mask & (1 << n))
         {
            InterlockedExchange(&c->inWaveCall, 1);
            waveInAddBuffer(c->hIn, &c->hdr[n], sizeof(WAVEHDR));
            InterlockedExchange(&c->inWaveCall, 0);
         }
      }
   }
   return 0;
}

static InCtx* openInputCtx(int deferred)
{
   InCtx* c = new InCtx;
   memset(c, 0, sizeof(InCtx));
   c->deferred = deferred ? 1 : 0;
   c->evt = CreateEvent(NULL, FALSE, FALSE, NULL);

   if (deferred) c->worker = CreateThread(NULL, 0, deferProcCtx, c, 0, NULL);

   OpenArgs oa;
   oa.ctx = c;
   oa.deviceId = g_deviceId;
   oa.result = MMSYSERR_NOERROR;
   HANDLE ot = CreateThread(NULL, 0, openProc, &oa, 0, NULL);
   DWORD ow = WaitForSingleObject(ot, OPEN_TIMEOUT_MS);
   if (ow == WAIT_TIMEOUT)
   {
      printf("  waveInOpen BLOCKED for more than %d ms on device %d\n",
             OPEN_TIMEOUT_MS, g_deviceId);
      // ot and c leaked on purpose: the open is still in progress.
      return NULL;
   }
   CloseHandle(ot);
   if (oa.result != MMSYSERR_NOERROR)
   {
      printf("  waveInOpen failed %u\n", oa.result);
      c->stop = 1;
      if (c->evt) SetEvent(c->evt);
      return NULL;
   }

   for (int n = 0; n < NUM_BUFFERS; n++)
   {
      c->hdr[n].lpData = c->buf[n];
      c->hdr[n].dwBufferLength = BYTES_PER_FRAME;
      c->hdr[n].dwUser = n;
      printf("    calling waveInPrepareHeader[%d]\n", n); fflush(stdout);
      waveInPrepareHeader(c->hIn, &c->hdr[n], sizeof(WAVEHDR));
      printf("    calling waveInAddBuffer[%d]\n", n); fflush(stdout);
      waveInAddBuffer(c->hIn, &c->hdr[n], sizeof(WAVEHDR));
   }
   printf("    calling waveInStart\n"); fflush(stdout);
   waveInStart(c->hIn);
   printf("    open sequence complete\n"); fflush(stdout);

   return c;
}

// Returns 1 if the context closed cleanly, 0 if reset wedged. On a wedge
// the context is deliberately leaked: its worker or WinMM's callback
// thread may still be inside the wave API holding its buffers.
static int closeInputCtx(InCtx* c, double* msOut)
{
   LARGE_INTEGER f, t0, t1;
   QueryPerformanceFrequency(&f);

   c->stop = 1;
   if (c->evt) SetEvent(c->evt);
   if (c->worker) WaitForSingleObject(c->worker, 2000);

   g_hIn = c->hIn;
   InterlockedExchange(&g_resetIsInput, 1);
   InterlockedExchange(&g_resetReturned, 0);
   QueryPerformanceCounter(&t0);
   HANDLE rt = CreateThread(NULL, 0, resetProc, NULL, 0, NULL);
   DWORD w = WaitForSingleObject(rt, (DWORD)g_resetTimeoutMs);
   QueryPerformanceCounter(&t1);
   if (msOut)
      *msOut = (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / (double)f.QuadPart;
   g_hIn = NULL;
   if (w == WAIT_TIMEOUT) return 0;      // rt and c leaked on purpose
   CloseHandle(rt);

   for (int n = 0; n < NUM_BUFFERS; n++)
      waveInUnprepareHeader(c->hIn, &c->hdr[n], sizeof(WAVEHDR));
   waveInClose(c->hIn);
   if (c->worker) CloseHandle(c->worker);
   if (c->evt) CloseHandle(c->evt);
   delete c;
   return 1;
}

// ---------------------------------------------------------------- legacy callbacks

static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg,
                                DWORD_PTR inst, DWORD_PTR p1, DWORD_PTR p2)
{
   if (uMsg == WIM_DATA)
   {
      InterlockedExchange(&g_callbackTid, (LONG)GetCurrentThreadId());
      if (!g_stopFeeding)
      {
         waveInAddBuffer(hwi, (WAVEHDR*)p1, sizeof(WAVEHDR));
      }
   }
}

static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg,
                                 DWORD_PTR inst, DWORD_PTR p1, DWORD_PTR p2)
{
   if (uMsg == WOM_DONE)
   {
      WAVEHDR* h = (WAVEHDR*)p1;
      InterlockedOr(&g_freeMask, 1 << (int)h->dwUser);
   }
}

// Mirrors MpodWinMM: sipX drives waveOutWrite from its own thread, and that
// is the thread that blocks in the field.
static DWORD WINAPI feederProc(LPVOID)
{
   InterlockedExchange(&g_feederTid, (LONG)GetCurrentThreadId());
   while (!g_stopFeeding)
   {
      LONG mask = g_freeMask;
      for (int n = 0; n < NUM_BUFFERS; n++)
      {
         if (mask & (1 << n))
         {
            InterlockedAnd(&g_freeMask, ~(1 << n));
            waveOutWrite(g_hOut, &g_hdr[n], sizeof(WAVEHDR));
         }
      }
      Sleep(1);
   }
   return 0;
}

// ---------------------------------------------------------------- legacy setup

static int openInput(void)
{
   WAVEFORMATEX fmt;
   memset(&fmt, 0, sizeof(fmt));
   fmt.wFormatTag = WAVE_FORMAT_PCM;
   fmt.nChannels = 1;
   fmt.nSamplesPerSec = 8000;
   fmt.wBitsPerSample = 16;
   fmt.nBlockAlign = 2;
   fmt.nAvgBytesPerSec = 16000;

   MMRESULT r = waveInOpen(&g_hIn, (UINT)g_deviceId, &fmt,
                           (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION);
   if (r != MMSYSERR_NOERROR) { printf("waveInOpen failed %u\n", r); return 0; }

   for (int n = 0; n < NUM_BUFFERS; n++)
   {
      memset(&g_hdr[n], 0, sizeof(WAVEHDR));
      g_hdr[n].lpData = g_buf[n];
      g_hdr[n].dwBufferLength = BYTES_PER_FRAME;
      g_hdr[n].dwUser = n;
      waveInPrepareHeader(g_hIn, &g_hdr[n], sizeof(WAVEHDR));
      waveInAddBuffer(g_hIn, &g_hdr[n], sizeof(WAVEHDR));
   }
   waveInStart(g_hIn);
   return 1;
}

static int openOutput(void)
{
   WAVEFORMATEX fmt;
   memset(&fmt, 0, sizeof(fmt));
   fmt.wFormatTag = WAVE_FORMAT_PCM;
   fmt.nChannels = 1;
   fmt.nSamplesPerSec = 8000;
   fmt.wBitsPerSample = 16;
   fmt.nBlockAlign = 2;
   fmt.nAvgBytesPerSec = 16000;

   MMRESULT r = waveOutOpen(&g_hOut, (UINT)g_deviceId, &fmt,
                            (DWORD_PTR)waveOutProc, 0,
                            CALLBACK_FUNCTION);
   if (r != MMSYSERR_NOERROR) { printf("waveOutOpen failed %u\n", r); return 0; }

   for (int n = 0; n < NUM_BUFFERS; n++)
   {
      memset(&g_hdr[n], 0, sizeof(WAVEHDR));
      memset(g_buf[n], 0, BYTES_PER_FRAME);
      g_hdr[n].lpData = g_buf[n];
      g_hdr[n].dwBufferLength = BYTES_PER_FRAME;
      g_hdr[n].dwUser = n;
      waveOutPrepareHeader(g_hOut, &g_hdr[n], sizeof(WAVEHDR));
   }
   g_freeMask = (1 << NUM_BUFFERS) - 1;
   g_stopFeeding = 0;
   g_feederThread = CreateThread(NULL, 0, feederProc, NULL, 0, NULL);
   return 1;
}

static void closeAll(int isInput)
{
   g_stopFeeding = 1;

   if (g_feederThread)
   {
      WaitForSingleObject(g_feederThread, 2000);
      CloseHandle(g_feederThread);
      g_feederThread = NULL;
   }
   if (isInput && g_hIn)
   {
      for (int n = 0; n < NUM_BUFFERS; n++)
         waveInUnprepareHeader(g_hIn, &g_hdr[n], sizeof(WAVEHDR));
      waveInClose(g_hIn);
      g_hIn = NULL;
   }
   if (!isInput && g_hOut)
   {
      for (int n = 0; n < NUM_BUFFERS; n++)
         waveOutUnprepareHeader(g_hOut, &g_hdr[n], sizeof(WAVEHDR));
      waveOutClose(g_hOut);
      g_hOut = NULL;
   }
}

// ---------------------------------------------------------------- devnode

static DEVINST locateDevnode(const char* instanceId)
{
   WCHAR w[512];
   MultiByteToWideChar(CP_ACP, 0, instanceId, -1, w, 512);
   DEVINST dn = 0;
   CONFIGRET cr = CM_Locate_DevNodeW(&dn, w, CM_LOCATE_DEVNODE_NORMAL);
   if (cr != CR_SUCCESS)
   {
      printf("     CM_Locate_DevNodeW failed cr=%u -- check the instance id\n", cr);
      return 0;
   }
   return dn;
}

static const char* crName(CONFIGRET cr)
{
   switch (cr)
   {
   case CR_SUCCESS:         return "CR_SUCCESS";
   case CR_REMOVE_VETOED:   return "CR_REMOVE_VETOED";
   case CR_ACCESS_DENIED:   return "CR_ACCESS_DENIED (needs elevation)";
   case CR_INVALID_DEVNODE: return "CR_INVALID_DEVNODE";
   default:                 return "see CONFIGRET in cfgmgr32.h";
   }
}

static int waitForDevice(int isInput, int timeoutMs)
{
   int waited = 0;
   while (waited < timeoutMs)
   {
      UINT n = isInput ? waveInGetNumDevs() : waveOutGetNumDevs();
      if (n > 0) return waited;
      Sleep(250); waited += 250;
   }
   return -1;
}

// ---------------------------------------------------------------- watchdog

struct WatchdogArgs
{
   int     method;        // 0 suspend, 1 devnode
   HANDLE  targetThread;
   DEVINST devnode;
};

static DWORD WINAPI watchdogProc(LPVOID p)
{
   WatchdogArgs* a = (WatchdogArgs*)p;
   DWORD waited = 0;
   while (waited < RESET_TIMEOUT_MS && !g_resetReturned)
   {
      Sleep(25); waited += 25;
   }
   if (!g_resetReturned)
   {
      printf("      reset DID NOT RETURN within %d ms - undoing provocation\n",
             RESET_TIMEOUT_MS);
      if (a->method == 0 && a->targetThread) ResumeThread(a->targetThread);
      if (a->method == 1 && a->devnode)      CM_Enable_DevNode(a->devnode, 0);
   }
   return 0;
}

static DWORD WINAPI resetProc(LPVOID)
{
   MMRESULT r = g_resetIsInput ? waveInReset(g_hIn) : waveOutReset(g_hOut);
   InterlockedExchange(&g_resetResult, (LONG)r);
   InterlockedExchange(&g_resetReturned, 1);
   return 0;
}

// ---------------------------------------------------------------- hold

// Hold the device open and report once a second. No provocation. For
// watching what an externally triggered device change does to a live
// stream while you drive it by hand from another window.
static int modeHold(int isInput, int seconds)
{
   printf("hold: %s open for %d seconds, reporting every second\n",
          isInput ? "capture" : "render", seconds);

   if (!isInput)
   {
      if (!openOutput()) return 1;
      for (int s = 0; s < seconds; s++)
      {
         Sleep(1000);
         printf("[%3d s] freeMask=0x%02x feederTid=%lu\n",
                s, (unsigned)g_freeMask, (unsigned long)g_feederTid);
      }
      closeAll(0);
      return 0;
   }

   InCtx* c = openInputCtx((int)g_deferMode);
   if (!c) return 1;
   int lastCb = 0;
   for (int s = 0; s < seconds; s++)
   {
      Sleep(1000);
      int cb = (int)c->dataCallbacks;
      printf("[%3d s] callbacks=%-6d (+%-4d) nonEmpty=%-6d varied=%-6d\n",
             s, cb, cb - lastCb, (int)c->nonEmpty, (int)c->contentVaried);
      lastCb = cb;
   }

   printf("closing...\n");
   double ms = 0.0;
   int clean = closeInputCtx(c, &ms);
   printf("close %s after %.1f ms\n", clean ? "returned" : "WEDGED", ms);
   return 0;
}

// ---------------------------------------------------------------- parent cycle

// Full automated device-removal cycle against the USB parent devnode.
// The parent has no open wave handle, so unlike the MEDIA node it does
// not veto while a stream is running.
//
// Per iteration: open, let audio flow, disable the parent, watch the
// callbacks stop, time the reset, re-enable, wait for the device to come
// back, and confirm a fresh open still delivers real audio.
static int modeParent(int isInput, int iterations, const char* instanceId)
{
   if (!isElevated())
   {
      printf("ERROR: --method parent needs an elevated prompt.\n"
             "       Start menu -> cmd -> right click -> Run as administrator\n");
      return 1;
   }
   if (!instanceId)
   {
      printf("ERROR: --method parent needs --instance \"<USB parent devnode>\"\n"
             "       Find it with:\n"
             "         Get-PnpDevice -Class USB | Where-Object "
             "{ $_.InstanceId -like \"*VID_xxxx*\" }\n"
             "       Use the composite/parent node, not the MEDIA node.\n");
      return 1;
   }

   printf("parent: %s, %d cycles against %s\n",
          isInput ? "capture" : "render", iterations, instanceId);
   printf("        deferred=%d\n", (int)g_deferMode);

   int wedged = 0, cleanClose = 0, audioBack = 0, audioLost = 0;

   for (int it = 1; it <= iterations; it++)
   {
      // Vary both delays so the removal lands at different points in the
      // buffer cycle and the reset at different points after removal.
      int settleMs  = 200 + (it * 37) % 400;
      int observeMs = 50  + (it * 53) % 450;

      printf("\n[%2d] settle=%dms observe=%dms\n", it, settleMs, observeMs);

      InCtx* c = NULL;
      if (isInput)
      {
         c = openInputCtx((int)g_deferMode);
         if (!c) { printf("     open failed\n"); break; }
      }
      else
      {
         if (!openOutput()) { printf("     open failed\n"); break; }
      }
      Sleep(settleMs);

      int cbBefore = isInput ? (int)c->dataCallbacks : 0;
      printf("     before removal: callbacks=%d\n", cbBefore);

      DEVINST dn = locateDevnode(instanceId);
      if (!dn)
      {
         if (c) closeInputCtx(c, NULL); else closeAll(0);
         break;
      }

      CONFIGRET cr = CM_Disable_DevNode(dn, 0);
      printf("     CM_Disable_DevNode cr=%u %s\n", cr, crName(cr));
      if (cr != CR_SUCCESS)
      {
         printf("     cannot remove device -- stopping\n");
         if (c) closeInputCtx(c, NULL); else closeAll(0);
         break;
      }

      Sleep(observeMs);
      int cbAfter = isInput ? (int)c->dataCallbacks : 0;
      printf("     after removal:  callbacks=%d (+%d)\n",
             cbAfter, cbAfter - cbBefore);

      double ms = 0.0;
      int clean;
      if (isInput)
      {
         clean = closeInputCtx(c, &ms);
      }
      else
      {
         LARGE_INTEGER f, t0, t1;
         QueryPerformanceFrequency(&f);
         g_stopFeeding = 1;
         if (g_feederThread) WaitForSingleObject(g_feederThread, 2000);
         InterlockedExchange(&g_resetIsInput, 0);
         InterlockedExchange(&g_resetReturned, 0);
         QueryPerformanceCounter(&t0);
         HANDLE rt = CreateThread(NULL, 0, resetProc, NULL, 0, NULL);
         DWORD w = WaitForSingleObject(rt, RESET_TIMEOUT_MS);
         QueryPerformanceCounter(&t1);
         ms = (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / (double)f.QuadPart;
         clean = (w != WAIT_TIMEOUT);
         if (clean) { CloseHandle(rt); closeAll(0); }
         else       { g_hOut = NULL; g_feederThread = NULL; }
      }

      printf("     reset %s after %.1f ms\n",
             clean ? "returned" : "WEDGED", ms);
      if (clean) cleanClose++; else wedged++;

      CM_Enable_DevNode(dn, 0);
      int back = waitForDevice(isInput, DEVICE_RETURN_MS);
      if (back < 0)
      {
         printf("     device did NOT return within %d ms -- stopping\n",
                DEVICE_RETURN_MS);
         break;
      }
      printf("     device returned after %d ms\n", back);
      Sleep(500);   // let the endpoint settle before reopening

      // Does a fresh open still deliver real audio?
      if (isInput)
      {
         InCtx* v = openInputCtx((int)g_deferMode);
         if (!v) { printf("     verify open FAILED\n"); audioLost++; }
         else
         {
            Sleep(400);
            int ok = (v->dataCallbacks > 0 && v->nonEmpty > 0 && v->contentVaried > 0);
            printf("     verify: callbacks=%d nonEmpty=%d varied=%d  %s\n",
                   (int)v->dataCallbacks, (int)v->nonEmpty,
                   (int)v->contentVaried, ok ? "AUDIO OK" : "NO AUDIO");
            if (ok) audioBack++; else audioLost++;
            if (!closeInputCtx(v, NULL))
            {
               printf("     verify close WEDGED -- process unreliable, stopping\n");
               break;
            }
         }
      }
      else
      {
         if (openOutput()) { Sleep(300); closeAll(0); audioBack++; }
         else              { printf("     verify open FAILED\n"); audioLost++; }
      }

      if (!clean && isInput)
      {
         printf("     input wedged -- later results in this process are\n"
                "     unreliable, stopping\n");
         break;
      }
      Sleep(500);
   }

   printf("\nparent: wedged=%d cleanClose=%d audioBack=%d audioLost=%d\n",
          wedged, cleanClose, audioBack, audioLost);
   return 0;
}

// ---------------------------------------------------------------- reopen

// Wedge one input context via SuspendThread, abandon it, then ask whether
// fresh opens still deliver real audio.
static int modeReopen(int isInput, int iterations)
{
   if (!isInput)
   {
      printf("reopen: input only -- use --method suspend for output\n");
      return 1;
   }

   int deferred = (int)g_deferMode;
   printf("reopen: wedge one context, abandon it, then %d fresh opens\n", iterations);
   printf("        deferred=%d (1 = wave call on our thread, 0 = in callback)\n",
          deferred);

   InCtx* victim = openInputCtx(deferred);
   if (!victim) return 1;
   Sleep(250);

   if (victim->tid == 0) { printf("  no callback thread seen\n"); return 1; }
   HANDLE target = OpenThread(THREAD_SUSPEND_RESUME, FALSE, (DWORD)victim->tid);
   if (!target) { printf("  OpenThread failed %lu\n", GetLastError()); return 1; }

   if (deferred)
   {
      // The worker is only inside waveInAddBuffer briefly. Suspend it
      // repeatedly until we catch it there, so the provocation matches
      // the non-deferred case where the callback thread is inside the
      // wave call almost continuously.
      int tries = 0;
      for (; tries < 20000; tries++)
      {
         SuspendThread(target);
         if (victim->inWaveCall) break;
         ResumeThread(target);
      }
      printf("  landed inside waveInAddBuffer after %d attempts%s\n",
             tries, (tries >= 20000) ? " (GAVE UP)" : "");
      if (tries >= 20000) { CloseHandle(target); return 1; }
   }
   else
   {
      SuspendThread(target);
   }

   g_hIn = victim->hIn;
   InterlockedExchange(&g_resetIsInput, 1);
   InterlockedExchange(&g_resetReturned, 0);
   HANDLE rt = CreateThread(NULL, 0, resetProc, NULL, 0, NULL);
   DWORD w = WaitForSingleObject(rt, RESET_TIMEOUT_MS + 2000);
   printf("  provoking reset: %s\n",
          (w == WAIT_TIMEOUT) ? "ABANDONED (as expected)" : "returned");
   ResumeThread(target);
   CloseHandle(target);
   g_hIn = NULL;

   if (w != WAIT_TIMEOUT)
   {
      CloseHandle(rt);
      printf("  no wedge produced -- rerun to try again\n");
      return 1;
   }
   // rt and victim leaked on purpose.

   int good = 0, deaf = 0;
   for (int i = 1; i <= iterations; i++)
   {
      InCtx* c = openInputCtx(deferred);
      if (!c) { printf("  [%2d] open FAILED\n", i); deaf++; continue; }
      Sleep(300);

      int cb = (int)c->dataCallbacks;
      int ne = (int)c->nonEmpty;
      int cv = (int)c->contentVaried;
      int audio = (cb > 0 && ne > 0 && cv > 0);
      if (audio) good++; else deaf++;

      printf("  [%2d] callbacks=%-4d nonEmpty=%-4d varied=%-4d  %s\n",
             i, cb, ne, cv, audio ? "AUDIO OK" : "NO AUDIO");

      if (!closeInputCtx(c, NULL))
      {
         printf("  [%2d] close wedged -- context leaked, stopping\n", i);
         break;
      }
      Sleep(30);
   }

   printf("\nreopen audioOk=%d noAudio=%d\n", good, deaf);
   return 0;
}

// ---------------------------------------------------------------- sweep

static int modeSweep(int isInput, int maxDelayMs)
{
   printf("sweep: settle delay 0..%d ms, one iteration each\n", maxDelayMs);
   int hits = 0;
   for (int d = 0; d <= maxDelayMs; d++)
   {
      g_callbackTid = 0; g_feederTid = 0; g_stopFeeding = 0;
      g_resetReturned = 0;
      if (isInput ? !openInput() : !openOutput()) return 1;
      Sleep(d);

      DWORD tid = isInput ? (DWORD)g_callbackTid : (DWORD)g_feederTid;
      if (tid == 0)
      {
         printf("[%3d ms] no callback yet\n", d);
         closeAll(isInput); continue;
      }
      HANDLE target = OpenThread(THREAD_SUSPEND_RESUME, FALSE, tid);
      if (!target) { closeAll(isInput); continue; }
      SuspendThread(target);

      InterlockedExchange(&g_resetIsInput, isInput ? 1 : 0);
      InterlockedExchange(&g_resetReturned, 0);
      HANDLE rt = CreateThread(NULL, 0, resetProc, NULL, 0, NULL);
      DWORD w = WaitForSingleObject(rt, 2000);
      ResumeThread(target); CloseHandle(target);

      if (w == WAIT_TIMEOUT)
      {
         printf("[%3d ms] WEDGED\n", d);
         hits++;
         g_hIn = NULL; g_hOut = NULL;
         if (g_feederThread)
         {
            g_stopFeeding = 1;
            CloseHandle(g_feederThread);
            g_feederThread = NULL;
         }
         if (isInput)
         {
            printf("         input wedged -- process is now unreliable, stopping\n");
            break;
         }
         printf("         (handle abandoned; continuing)\n");
      }
      else
      {
         printf("[%3d ms] clean\n", d);
         CloseHandle(rt);
         closeAll(isInput);
      }
      Sleep(30);
   }
   printf("\nsweep hits=%d of %d\n", hits, maxDelayMs + 1);
   return 0;
}

// Print the WinMM device list and the MMDevice endpoint list side by side.
// sipX joins these two worlds by name, and WinMM truncates names to 31
// characters, so this shows whether that join is actually unambiguous.
static void listEndpoints(EDataFlow flow, const char* label)
{
   IMMDeviceEnumerator* pEnum = NULL;
   HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
                                 CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                 (void**)&pEnum);
   if (FAILED(hr)) { printf("  CoCreateInstance failed 0x%08lx\n", hr); return; }

   IMMDeviceCollection* pColl = NULL;
   hr = pEnum->EnumAudioEndpoints(flow, DEVICE_STATEMASK_ALL, &pColl);
   if (SUCCEEDED(hr))
   {
      UINT n = 0;
      pColl->GetCount(&n);
      printf("  %s MMDevice endpoints (all states): %u\n", label, n);
      for (UINT i = 0; i < n; i++)
      {
         IMMDevice* pDev = NULL;
         if (FAILED(pColl->Item(i, &pDev))) continue;
         LPWSTR id = NULL;
         DWORD state = 0;
         pDev->GetId(&id);
         pDev->GetState(&state);

         wchar_t name[256] = L"";
         IPropertyStore* pProps = NULL;
         if (SUCCEEDED(pDev->OpenPropertyStore(STGM_READ, &pProps)))
         {
            PROPVARIANT pv; PropVariantInit(&pv);
            if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &pv))
                && pv.vt == VT_LPWSTR)
            {
               wcsncpy(name, pv.pwszVal, 255);
            }
            PropVariantClear(&pv);
            pProps->Release();
         }

         const char* st = (state == DEVICE_STATE_ACTIVE)     ? "ACTIVE"
                        : (state == DEVICE_STATE_DISABLED)   ? "DISABLED"
                        : (state == DEVICE_STATE_NOTPRESENT) ? "NOTPRESENT"
                        : (state == DEVICE_STATE_UNPLUGGED)  ? "UNPLUGGED"
                                                             : "?";
         printf("    [%u] %-10s %-45S\n         %S\n", i, st, name, id ? id : L"");
         if (id) CoTaskMemFree(id);
         pDev->Release();
      }
      pColl->Release();
   }
   pEnum->Release();
}

static int modeList(void)
{
   CoInitializeEx(NULL, COINIT_MULTITHREADED);

   UINT nIn = waveInGetNumDevs();
   printf("WinMM capture devices: %u\n", nIn);
   for (UINT i = 0; i < nIn; i++)
   {
      WAVEINCAPS caps;
      if (waveInGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
         printf("    [%u] \"%s\"\n", i, caps.szPname);
   }

   UINT nOut = waveOutGetNumDevs();
   printf("WinMM render devices: %u\n", nOut);
   for (UINT i = 0; i < nOut; i++)
   {
      WAVEOUTCAPS caps;
      if (waveOutGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
         printf("    [%u] \"%s\"\n", i, caps.szPname);
   }

   printf("\n");
   listEndpoints(eCapture, "capture");
   printf("\n");
   listEndpoints(eRender, "render");

   // What WAVE_MAPPER actually resolves to for capture.
   WAVEINCAPS mapCaps;
   if (waveInGetDevCaps(WAVE_MAPPER, &mapCaps, sizeof(mapCaps)) == MMSYSERR_NOERROR)
      printf("\nWAVE_MAPPER capture resolves to \"%s\"\n", mapCaps.szPname);

   CoUninitialize();
   return 0;
}

// As modeReopen, but the wedge comes from a real device removal driven
// externally (swdevice_audio) rather than from SuspendThread. Waits for
// the callbacks to stop, then does the reset and the reopen loop.
//
// Compare --method removed against --method removed-deferred: the only
// difference is whether waveInAddBuffer runs in the callback or on a
// thread we own, which is the MpodWinMM pattern the restructure would
// adopt. Whether the deferred case recovers is the whole question.
static int modeRemoved(int iterations)
{
   int deferred = (int)g_deferMode;
   printf("removed: waiting for an external device removal, then %d fresh opens\n",
          iterations);
   printf("         deferred=%d  device=%d\n", deferred, g_deviceId);

   {
      UINT preDevs = waveInGetNumDevs();
      WAVEINCAPSA caps;
      memset(&caps, 0, sizeof(caps));
      MMRESULT capRes = (g_deviceId >= 0)
                      ? waveInGetDevCapsA((UINT_PTR) g_deviceId, &caps, sizeof(caps))
                      : MMSYSERR_NOERROR;
      printf("         waveInGetNumDevs=%u  devCaps(%d)=%u name='%s'\n",
             preDevs, g_deviceId, capRes,
             (capRes == MMSYSERR_NOERROR) ? caps.szPname : "?");
      fflush(stdout);
   }

   {
      WAVEFORMATEX wfx;
      HWAVEIN      hTest = NULL;
      MMRESULT     mr;
      memset(&wfx, 0, sizeof(wfx));
      wfx.wFormatTag      = WAVE_FORMAT_PCM;
      wfx.nChannels       = 1;
      wfx.nSamplesPerSec  = 8000;
      wfx.wBitsPerSample  = 16;
      wfx.nBlockAlign     = 2;
      wfx.nAvgBytesPerSec = 16000;
      mr = waveInOpen(&hTest, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
      printf("         WAVE_MAPPER open 8k mono = %u\n", mr);
      if (mr == MMSYSERR_NOERROR) waveInClose(hTest);
      mr = waveInOpen(&hTest, (UINT) g_deviceId, &wfx, 0, 0, CALLBACK_NULL);
      printf("         device %d open 8k mono   = %u\n", g_deviceId, mr);
      if (mr == MMSYSERR_NOERROR) waveInClose(hTest);
      fflush(stdout);
   }

   // WinMM returns MMSYSERR_ERROR (1) with no detail. WASAPI returns a real
   // HRESULT, which names the cause: AUDCLNT_E_SERVICE_NOT_RUNNING,
   // E_ACCESSDENIED, AUDCLNT_E_ENDPOINT_CREATE_FAILED all point elsewhere.
   {
      IMMDeviceEnumerator* pEnum = NULL;
      IMMDevice*           pDev  = NULL;
      IAudioClient*        pCli  = NULL;
      WAVEFORMATEX*        pMix  = NULL;
      HRESULT hr;

      CoInitializeEx(NULL, COINIT_MULTITHREADED);
      hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                            __uuidof(IMMDeviceEnumerator), (void**) &pEnum);
      printf("         WASAPI CoCreateInstance     = 0x%08lX\n", (unsigned long) hr);
      if (SUCCEEDED(hr))
      {
         hr = pEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &pDev);
         printf("         WASAPI GetDefaultEndpoint   = 0x%08lX\n", (unsigned long) hr);
      }
      if (SUCCEEDED(hr))
      {
         hr = pDev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**) &pCli);
         printf("         WASAPI Activate             = 0x%08lX\n", (unsigned long) hr);
      }
      if (SUCCEEDED(hr))
      {
         hr = pCli->GetMixFormat(&pMix);
         printf("         WASAPI GetMixFormat         = 0x%08lX\n", (unsigned long) hr);
      }
      if (SUCCEEDED(hr))
      {
         hr = pCli->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, pMix, NULL);
         printf("         WASAPI Initialize           = 0x%08lX\n", (unsigned long) hr);
      }
      if (pMix)  CoTaskMemFree(pMix);
      if (pCli)  pCli->Release();
      if (pDev)  pDev->Release();
      if (pEnum) pEnum->Release();
      fflush(stdout);
   }

   InCtx* victim = openInputCtx(deferred);
   if (!victim) return 1;

   // Wait for the device to die: callbacks stop arriving.
   int lastCb = -1, quietSeconds = 0, waited = 0;
   while (waited < 120)
   {
      Sleep(1000); waited++;
      int cb = (int)victim->dataCallbacks;
      printf("  [%3d s] callbacks=%-6d (+%d)\n", waited, cb,
             (lastCb < 0) ? cb : cb - lastCb);
      if (lastCb >= 0 && cb == lastCb) quietSeconds++; else quietSeconds = 0;
      lastCb = cb;
      if (quietSeconds >= 3) break;
   }
   if (quietSeconds < 3)
   {
      printf("  device never went quiet -- is the removal harness running?\n");
      closeInputCtx(victim, NULL);
      return 1;
   }
   printf("  device is gone (quiet for %d s)\n", quietSeconds);

   double ms = 0.0;
   int clean = closeInputCtx(victim, &ms);
   printf("  reset %s after %.1f ms\n", clean ? "returned" : "WEDGED", ms);

   // Continue either way. A clean close is the expected result when the
   // wave call runs on our own thread, and we still want to confirm that
   // capture works afterwards rather than assuming it.
   if (clean) printf("  no wedge -- checking that capture still works\n");
   // On a wedge, victim is leaked on purpose.

   // The removed endpoint does not come back -- swdevice_audio has exited.
   // Reopen on device 0, which is always present. The question is whether
   // WinMM input still works in this process at all, and device 0 answers
   // that as well as the removed device would.
   int reopenDevice = 0;
   printf("  calling waveInGetNumDevs\n"); fflush(stdout);
   UINT nDevs = waveInGetNumDevs();
   printf("  reopening on device %d (%u capture devices present)\n",
          reopenDevice, nDevs);

   int good = 0, deaf = 0;
   for (int i = 1; i <= iterations; i++)
   {
      int saved = g_deviceId;
      g_deviceId = reopenDevice;
      InCtx* c = openInputCtx(deferred);
      g_deviceId = saved;
      if (!c) { printf("  [%2d] open FAILED\n", i); deaf++; continue; }
      Sleep(300);
      int cb = (int)c->dataCallbacks, ne = (int)c->nonEmpty, cv = (int)c->contentVaried;
      int audio = (cb > 0 && ne > 0 && cv > 0);
      if (audio) good++; else deaf++;
      printf("  [%2d] callbacks=%-4d nonEmpty=%-4d varied=%-4d  %s\n",
             i, cb, ne, cv, audio ? "AUDIO OK" : "NO AUDIO");
      if (!closeInputCtx(c, NULL))
      {
         printf("  [%2d] close wedged -- stopping\n", i);
         break;
      }
      Sleep(30);
   }
   printf("\nremoved audioOk=%d noAudio=%d\n", good, deaf);
   return 0;
}

// ---------------------------------------------------------------- main

static void usage(void)
{
   printf("wavelock_probe --dir in|out --method <mode> [--iterations N]\n"
          "               [--instance \"<PnP instance id>\"]\n\n"
          "  --device N  WinMM device index, default 0. Use -1 for\n"
          "              WAVE_MAPPER, which re-routes on device change and\n"
          "              so hides removals. sipX opens a specific index.\n"
          "  --reset-timeout N  ms to wait for reset before abandoning it,\n"
          "              default 3000. Raise it to measure how long a wedge\n"
          "              actually lasts; the default truncates the answer.\n"
          "  modes: list suspend sweep reopen deferred removed\n"
          "         removed-deferred hold parent\n\n"
          "  list      print WinMM devices and MMDevice endpoints side by\n"
          "            side, with endpoint states and what WAVE_MAPPER\n"
          "            resolves to. Takes no other arguments.\n"
          "  suspend   suspend the recycling thread, time reset\n"
          "  sweep     as suspend, walking the settle delay\n"
          "  reopen    wedge one input context, then test fresh opens\n"
          "  deferred  as reopen, wave call on our own thread\n"
          "  removed   wait for an external device removal (swdevice_audio),\n"
          "            then test whether fresh opens still deliver audio.\n"
          "  removed-deferred\n"
          "            as removed, with the wave call on our own thread.\n"
          "  hold      hold the device open, report per second\n"
          "  parent    disable/enable the USB parent devnode per cycle\n"
          "            (needs elevation and --instance)\n");
}

int main(int argc, char** argv)
{
   const char* dir = "out";
   const char* method = "suspend";
   const char* instanceId = NULL;
   int iterations = 20;

   for (int i = 1; i < argc; i++)
   {
      if (!strcmp(argv[i], "--help")) { usage(); return 0; }
      if (i >= argc - 1) break;
      if (!strcmp(argv[i], "--dir"))             dir = argv[++i];
      else if (!strcmp(argv[i], "--method"))     method = argv[++i];
      else if (!strcmp(argv[i], "--instance"))   instanceId = argv[++i];
      else if (!strcmp(argv[i], "--iterations")) iterations = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--device"))     g_deviceId = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--reset-timeout")) g_resetTimeoutMs = atoi(argv[++i]);
   }

   int isInput = !strcmp(dir, "in");

   if (!strcmp(method, "deferred"))
   {
      g_deferMode = 1;
      return modeReopen(isInput, iterations);
   }
   if (!strcmp(method, "reopen")) return modeReopen(isInput, iterations);
   if (!strcmp(method, "sweep"))  return modeSweep(isInput, iterations);
   if (!strcmp(method, "hold"))   return modeHold(isInput, iterations);
   if (!strcmp(method, "parent")) return modeParent(isInput, iterations, instanceId);
   if (!strcmp(method, "list"))   return modeList();
   if (!strcmp(method, "removed"))          return modeRemoved(iterations);
   if (!strcmp(method, "removed-deferred")) { g_deferMode = 1; return modeRemoved(iterations); }

   if (strcmp(method, "suspend"))
   {
      printf("unknown method '%s'\n\n", method);
      usage();
      return 1;
   }

   printf("dir=%s method=suspend iterations=%d\n", dir, iterations);

   int blocked = 0, clean = 0, skipped = 0;

   for (int it = 1; it <= iterations; it++)
   {
      g_callbackTid = 0; g_feederTid = 0; g_resetReturned = 0; g_stopFeeding = 0;

      if (isInput ? !openInput() : !openOutput()) return 1;
      Sleep(120 + (it * 7) % 60);

      DWORD tid = isInput ? (DWORD)g_callbackTid : (DWORD)g_feederTid;
      if (tid == 0)
      {
         printf("[%3d] no callback thread seen - skipping\n", it);
         InterlockedExchange(&g_resetReturned, 1);
         closeAll(isInput); skipped++; continue;
      }
      HANDLE target = OpenThread(THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,
                                 FALSE, tid);
      if (!target)
      {
         printf("[%3d] OpenThread failed %lu - skipping\n", it, GetLastError());
         InterlockedExchange(&g_resetReturned, 1);
         closeAll(isInput); skipped++; continue;
      }
      SuspendThread(target);

      WatchdogArgs wa;
      wa.method = 0;
      wa.targetThread = target;
      wa.devnode = 0;
      HANDLE wd = CreateThread(NULL, 0, watchdogProc, &wa, 0, NULL);

      LARGE_INTEGER f, t0, t1;
      QueryPerformanceFrequency(&f);
      QueryPerformanceCounter(&t0);
      InterlockedExchange(&g_resetIsInput, isInput ? 1 : 0);
      InterlockedExchange(&g_resetResult, 0);
      g_resetThread = CreateThread(NULL, 0, resetProc, NULL, 0, NULL);

      DWORD w = WaitForSingleObject(g_resetThread, RESET_TIMEOUT_MS + 4000);
      QueryPerformanceCounter(&t1);
      double ms = (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / (double)f.QuadPart;

      if (w == WAIT_TIMEOUT)
      {
         blocked++;
         InterlockedExchange(&g_resetReturned, 1);
         printf("[%3d] reset NEVER RETURNED (abandoned) after %.1f ms\n", it, ms);
         g_resetThread = NULL;
         WaitForSingleObject(wd, 5000); CloseHandle(wd);
         ResumeThread(target); CloseHandle(target);
         g_hIn = NULL; g_hOut = NULL; g_feederThread = NULL;
         printf("      stopping: harness cannot continue after an "
                "unrecoverable reset\n");
         break;
      }

      const char* verdict = (ms > 500.0) ? "BLOCKED" : "clean";
      if (ms > 500.0) blocked++; else clean++;
      printf("[%3d] reset returned %u after %.1f ms  %s\n",
             it, (unsigned)g_resetResult, ms, verdict);
      CloseHandle(g_resetThread); g_resetThread = NULL;

      WaitForSingleObject(wd, 5000);
      CloseHandle(wd);
      ResumeThread(target);
      CloseHandle(target);

      closeAll(isInput);
      Sleep(50);
   }

   printf("\nblocked=%d clean=%d skipped=%d\n", blocked, clean, skipped);
   return 0;
}

