//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Device-departure tests at the sipXtapi layer: a real removal of the bench capture
// device, driven by scripts/bench_trigger.py, during a live call between
// two local sipXtapi instances. These are the customer's sequences at the
// customer's layer:
//
//   1. device departs mid-call, the app hangs up
//   2. device departs mid-call, the app calls sipxUnInitialize with the
//      call still up (the reported hang/crash path)
//   3. device returns, a new call must bring up media through a fresh
//      driver
//
// Bench config lives outside the repo (~/.sipx_bench/bench.conf). When it
// is missing the trigger exits 2 and each bench test records one
// "BENCH SKIPPED" point, which the runner reports as a skip.

#include <sipxunittests.h>
#include "sipXtapiTest.h"
#include "EventValidator.h"
#include "callbacks.h"
#include <os/OsTask.h>
#include <os/OsDateTime.h>
#include <os/OsSysLog.h>
#include <utl/UtlString.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#  define BENCH_POPEN  _popen
#  define BENCH_PCLOSE _pclose
#else
#  define BENCH_POPEN  popen
#  define BENCH_PCLOSE pclose
#endif

extern SIPX_INST g_hInst;
extern SIPX_INST g_hInst2;
extern SIPX_INST g_hInst3;
extern SIPX_INST g_hInst4;

// ---------------------------------------------------------------- helpers

static unsigned long benchNowMs(void)
{
   OsTime now;
   OsDateTime::getCurTime(now);
   return (unsigned long)now.cvtToMsecs();
}

/// Run one bench_trigger action. Returns its exit code; its output line
/// (stdout and stderr) goes to outLine. Works from the runner's cwd
/// (x64/Release or the autotools test dir) and from the repo root.
static int benchTrigger(const char* action, UtlString& outLine)
{
   outLine = "";
   const char* candidates[] = {
      "scripts/bench_trigger.py",
      "../../scripts/bench_trigger.py",
      "../../../../scripts/bench_trigger.py",
      NULL
   };
   const char* script = NULL;
   for (int i = 0; candidates[i]; i++)
   {
      FILE* probe = fopen(candidates[i], "r");
      if (probe)
      {
         fclose(probe);
         script = candidates[i];
         break;
      }
   }
   if (!script)
   {
      outLine = "BENCH NOT CONFIGURED: bench_trigger.py not found from cwd";
      return 2;
   }
   char cmd[512];
   snprintf(cmd, sizeof(cmd), "sh -c \"python3 %s %s 2>&1\"", script, action);
   FILE* p = BENCH_POPEN(cmd, "r");
   if (!p)
   {
      outLine = "FAILED: could not run bench_trigger.py";
      return 1;
   }
   char line[512];
   while (fgets(line, sizeof(line), p))
   {
      outLine.append(line);
   }
   outLine.strip(UtlString::both, '\n');
   outLine.strip(UtlString::both, '\r');
   int rc = BENCH_PCLOSE(p);
#ifndef WIN32
   if (rc > 255) rc = rc >> 8;   // wait status -> exit code
#endif
   printf("bench_trigger %s -> %d: %s\n", action, rc, outLine.data());
   fflush(stdout);
   return rc;
}

/// Skip loudly unless the bench is configured and streaming. Returns
/// FALSE when the test must return. Fills match, runs, offsets.
static UtlBoolean benchGate(UtlString& match, int& runs, UtlString& offsets)
{
   UtlString line;
   int rc = benchTrigger("info", line);
   if (rc != 0)
   {
      printf("BENCH SKIPPED: %s\n", line.isNull() ? "bench_trigger.py not runnable" : line.data());
      printf("SIPX_TEST_SKIP: BENCH SKIPPED: bench not configured (see line above)\n");
      fflush(stdout);
      CPPUNIT_ASSERT_MESSAGE("SIPX_TEST_SKIP: BENCH SKIPPED: bench not configured", 0);
      return FALSE;
   }
   match = "";
   runs = 3;
   offsets = "0,250,5000";
   const char* s = strstr(line.data(), "match=");
   if (s)
   {
      s += 6;
      const char* e = strchr(s, ' ');
      match.append(s, e ? (size_t)(e - s) : strlen(s));
   }
   s = strstr(line.data(), "runs=");
   if (s) runs = atoi(s + 5);
   s = strstr(line.data(), "offsets_ms=");
   if (s) offsets = s + 11;
   CPPUNIT_ASSERT_MESSAGE("bench info gave no device match", !match.isNull());

   rc = benchTrigger("status", line);
   if (rc == 3)
   {
      benchTrigger("connect", line);
      rc = benchTrigger("status", line);
   }
   if (rc != 0)
   {
      printf("BENCH SKIPPED: device not ready: %s\n", line.data());
      printf("SIPX_TEST_SKIP: BENCH SKIPPED: bench device not ready (see line above)\n");
      fflush(stdout);
      CPPUNIT_ASSERT_MESSAGE("SIPX_TEST_SKIP: BENCH SKIPPED: bench device not ready", 0);
      return FALSE;
   }
   return TRUE;
}

static int benchOffsetMs(const UtlString& offsets, int iteration)
{
   int values[16];
   int n = 0;
   const char* s = offsets.data();
   while (*s && n < 16)
   {
      values[n++] = atoi(s);
      const char* c = strchr(s, ',');
      if (!c) break;
      s = c + 1;
   }
   return n ? values[iteration % n] : 0;
}

/// Select the bench device as the calling instance's call input device,
/// by name through the public API, the way an application does.
static UtlBoolean benchSelectInputDevice(SIPX_INST hInst, const UtlString& match,
                                         UtlString& chosen)
{
   size_t numDevices = 0;
   if (sipxAudioGetNumInputDevices(hInst, numDevices) != SIPX_RESULT_SUCCESS)
   {
      return FALSE;
   }
   for (size_t i = 0; i < numDevices; i++)
   {
      const char* name = NULL;
      if (sipxAudioGetInputDevice(hInst, (int)i, name) == SIPX_RESULT_SUCCESS
          && name && strstr(name, match.data()))
      {
         chosen = name;
         return sipxAudioSetCallInputDevice(hInst, name) == SIPX_RESULT_SUCCESS;
      }
   }
   chosen = "";
   return FALSE;
}


/// Select the bench device as the call output device, by name through the
/// public API. Selecting the same name again re-enables a device left on
/// the fallback timer by a departure (setSpeakerDevice).
static UtlBoolean benchSelectOutputDevice(SIPX_INST hInst, const UtlString& match,
                                          UtlString& chosen)
{
   size_t numDevices = 0;
   if (sipxAudioGetNumOutputDevices(hInst, numDevices) != SIPX_RESULT_SUCCESS)
   {
      return FALSE;
   }
   for (size_t i = 0; i < numDevices; i++)
   {
      const char* name = NULL;
      if (sipxAudioGetOutputDevice(hInst, (int)i, name) == SIPX_RESULT_SUCCESS
          && name && strstr(name, match.data()))
      {
         chosen = name;
         return sipxAudioSetCallOutputDevice(hInst, name) == SIPX_RESULT_SUCCESS;
      }
   }
   chosen = "";
   return FALSE;
}

/// Re-create an instance exactly as setUp does. Ports can linger in
/// TIME_WAIT right after an uninitialize, so retry briefly.
static void benchRecreateInstance(SIPX_INST& hInst, int sipPort, int tlsPort,
                                  int rtpPort, const char* identity)
{
   hInst = NULL;
   for (int attempt = 0; attempt < 20 && hInst == NULL; attempt++)
   {
      sipxInitialize(&hInst, sipPort, sipPort, tlsPort, rtpPort, 32, identity, "127.0.0.1",
                     false, NULL, NULL, NULL, true, INTERNAL_SAMPLE_RATE, DEVICE_SAMPLE_RATE);
      if (hInst == NULL)
      {
         OsTask::delay(500);
      }
   }
   CPPUNIT_ASSERT_MESSAGE("could not re-create an instance after uninitialize",
                          hInst != NULL);
   if (hInst)
   {
      sipxConfigSetConnectionIdleTimeout(hInst, 7);
   }
}

// ---------------------------------------------------------------- tests

// Path 1: the device departs mid-call and the app hangs up. Asserts the
// departure notification reaches the application, then the normal
// hang-up sequence on both sides. KNOWN DEFECT 1: the notification reaches
// only the most recently created media interface (the called instance
// here), so the calling-side assertion fails until dispatch is fixed.
void sipXtapiTestSuite::testDeviceDepartureThenHangup()
{
   UtlString match, offsets, line, chosen, chosenOut;
   int runs;
   if (!benchGate(match, runs, offsets)) return;

   for (int it = 0; it < runs; it++)
   {
      if (!benchSelectInputDevice(g_hInst, match, chosen))
      {
         CPPUNIT_ASSERT_MESSAGE("bench device not selectable as call input device", 0);
         return;
      }
      // Reselect the output every iteration so each starts from the same
      // state: after a departure the output stays on the fallback timer
      // until it is reselected (KNOWN DEFECT 4).
      if (!benchSelectOutputDevice(g_hInst, match, chosenOut))
      {
         CPPUNIT_ASSERT_MESSAGE("bench device not selectable as call output device", 0);
         return;
      }
      int offset = benchOffsetMs(offsets, it);
      int nowait = (it % 2);
      printf("\nbench iteration %d (%s) offset %d ms, input '%s', output '%s'\n",
             it, nowait ? "in-flight" : "post-removal", offset, chosen.data(), chosenOut.data());

      SIPX_CALL hCallingCall = SIPX_CALL_NULL;
      SIPX_LINE hCallingLine = SIPX_LINE_NULL;
      SIPX_CALL hCalledCall = SIPX_CALL_NULL;
      SIPX_LINE hCalledLine = SIPX_LINE_NULL;
      doCallBasicSetup(g_hInst, "sip:calling@127.0.0.1:8000", "sip:calling@127.0.0.1:8000",
                       g_hInst2, "sip:called@127.0.0.1:9100", "sip:called@127.0.0.1:9100",
                       hCallingCall, hCallingLine, hCalledCall, hCalledLine);

      EventValidator validatorCalling("departure.calling");
      EventValidator validatorCalled("departure.called");
      validatorCalling.ignoreMediaEvent(MEDIA_MIC_ENERGY_LEVEL);
      validatorCalling.ignoreMediaEvent(MEDIA_SPEAKER_ENERGY_LEVEL);
      validatorCalled.ignoreMediaEvent(MEDIA_MIC_ENERGY_LEVEL);
      validatorCalled.ignoreMediaEvent(MEDIA_SPEAKER_ENERGY_LEVEL);
      CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
         sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling));
      CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
         sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled));

      // Let media run on a live stream before the departure.
      OsTask::delay(1500);

      // Always nowait: control must return before the media task fires
      // the not-present event, or the validator misses it. The offset
      // still varies how long media ran before the departure.
      CPPUNIT_ASSERT_EQUAL_MESSAGE("bench disconnect failed", 0,
                                   benchTrigger("disconnect nowait", line));

      // The contract: the application that selected the device is told it
      // is gone.
      // KNOWN DEFECT 1 (phase 2): device notifications go only to the head
      // of the device manager's dispatcher list, which is the most recently
      // created media interface in the process. Here that is the called
      // instance's call (created on the incoming INVITE), so the calling
      // instance gets neither INPUT nor OUTPUT_DEVICE_NOT_PRESENT. This
      // assertion fails until dispatch is fixed. Do not relax it.
      unsigned long t0 = benchNowMs();
      bool bRC = validatorCalling.waitForMediaEvent(MEDIA_INPUT_DEVICE_NOT_PRESENT,
                    MEDIA_CAUSE_DEVICE_UNAVAILABLE, MEDIA_TYPE_AUDIO, false, 10);
      printf("bench iteration %d: INPUT_DEVICE_NOT_PRESENT %s after %lu ms\n",
             it, bRC ? "received" : "MISSING (KNOWN DEFECT 1)", benchNowMs() - t0);
      fflush(stdout);
      CPPUNIT_ASSERT_MESSAGE("KNOWN DEFECT 1: no MEDIA_INPUT_DEVICE_NOT_PRESENT on calling instance within 10 s", bRC);
      // KNOWN DEFECT 1 applies to the output notification too: it goes to
      // the called instance. Consume it here if it ever comes; do not
      // require it.
      validatorCalling.waitForMediaEvent(MEDIA_OUTPUT_DEVICE_NOT_PRESENT,
                    MEDIA_CAUSE_DEVICE_UNAVAILABLE, MEDIA_TYPE_AUDIO, false, 1);
      // Diagnostic only, not asserted: where the notifications actually
      // landed. OUTPUT on every iteration shows the output reselect at the
      // top of the iteration brought the device back from fallback.
      bool calledIn = validatorCalled.waitForMediaEvent(MEDIA_INPUT_DEVICE_NOT_PRESENT,
                    MEDIA_CAUSE_DEVICE_UNAVAILABLE, MEDIA_TYPE_AUDIO, false, 1);
      bool calledOut = validatorCalled.waitForMediaEvent(MEDIA_OUTPUT_DEVICE_NOT_PRESENT,
                    MEDIA_CAUSE_DEVICE_UNAVAILABLE, MEDIA_TYPE_AUDIO, false, 1);
      printf("bench iteration %d: on called instance: INPUT_NOT_PRESENT %s, OUTPUT_NOT_PRESENT %s\n",
             it, calledIn ? "yes" : "no", calledOut ? "yes" : "no");
      fflush(stdout);

      OsTask::delay(offset);

      // Hang up: the normal sequence must complete on both sides.
      SIPX_CALL hDestroyedCall = hCallingCall;
      t0 = benchNowMs();
      CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxCallDestroy(hCallingCall));

      bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
      CPPUNIT_ASSERT_MESSAGE("calling: no MEDIA_LOCAL_STOP after hang-up", bRC);
      bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
      CPPUNIT_ASSERT_MESSAGE("calling: no MEDIA_REMOTE_STOP after hang-up", bRC);
      bRC = validatorCalling.waitForCallEvent(hCallingLine, hDestroyedCall, CALLSTATE_DISCONNECTED,
                                              CALLSTATE_CAUSE_NORMAL, false);
      CPPUNIT_ASSERT_MESSAGE("calling: no DISCONNECTED after hang-up", bRC);
      bRC = validatorCalling.waitForCallEvent(hCallingLine, hDestroyedCall, CALLSTATE_DESTROYED,
                                              CALLSTATE_CAUSE_NORMAL, false);
      CPPUNIT_ASSERT_MESSAGE("calling: no DESTROYED after hang-up", bRC);

      bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
      CPPUNIT_ASSERT_MESSAGE("called: no MEDIA_LOCAL_STOP after hang-up", bRC);
      bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
      CPPUNIT_ASSERT_MESSAGE("called: no MEDIA_REMOTE_STOP after hang-up", bRC);
      // doCallBasicSetup returns the auto-answer callback's handle as
      // hCalledCall; the waits use the global because sipxCallDestroy below
      // clears hCalledCall.
      bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall,
                    CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false, 10);
      CPPUNIT_ASSERT_MESSAGE("called: no DISCONNECTED after hang-up", bRC);
      // doCallBasicSetup removed its AutoAnswerCallback, which is what
      // destroys the called call in doCallBasic; destroy it here.
      CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxCallDestroy(hCalledCall));
      bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall,
                    CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false, 10);
      CPPUNIT_ASSERT_MESSAGE("called: no DESTROYED after hang-up", bRC);
      printf("bench iteration %d: hang-up sequence complete in %lu ms\n", it, benchNowMs() - t0);
      fflush(stdout);

      sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling);
      sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled);
      sipxLineRemove(hCallingLine);
      sipxLineRemove(hCalledLine);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("bench connect failed", 0, benchTrigger("connect", line));
      // sipXtapi's device list refreshes on its own clock; give it a
      // moment after the device is back before the next select.
      OsTask::delay(1000);
   }
   OsTask::delay(TEST_DELAY);
   checkForLeaks();
}

// Path 2: the reported failure. Device departs mid-call; the app then calls
// sipxUnInitialize with the call still up. Must return, bounded, no crash.
// The last iteration also uninitializes the other instances, so the shared
// media factory and its output driver are torn down after the departure,
// as in a single-instance application.
void sipXtapiTestSuite::testDeviceDepartureThenUninitialize()
{
   UtlString match, offsets, line, chosen, chosenOut;
   int runs;
   if (!benchGate(match, runs, offsets)) return;

   for (int it = 0; it < runs; it++)
   {
      if (!benchSelectInputDevice(g_hInst, match, chosen))
      {
         CPPUNIT_ASSERT_MESSAGE("bench device not selectable as call input device", 0);
         return;
      }
      // Reselect the output every iteration so each starts from the same
      // state: after a departure the output stays on the fallback timer
      // until it is reselected (KNOWN DEFECT 4).
      if (!benchSelectOutputDevice(g_hInst, match, chosenOut))
      {
         CPPUNIT_ASSERT_MESSAGE("bench device not selectable as call output device", 0);
         return;
      }
      int offset = benchOffsetMs(offsets, it);
      int nowait = (it % 2);
      const bool lastIt = (it == runs - 1);

      SIPX_CALL hCallingCall = SIPX_CALL_NULL;
      SIPX_LINE hCallingLine = SIPX_LINE_NULL;
      SIPX_CALL hCalledCall = SIPX_CALL_NULL;
      SIPX_LINE hCalledLine = SIPX_LINE_NULL;
      doCallBasicSetup(g_hInst, "sip:calling@127.0.0.1:8000", "sip:calling@127.0.0.1:8000",
                       g_hInst2, "sip:called@127.0.0.1:9100", "sip:called@127.0.0.1:9100",
                       hCallingCall, hCallingLine, hCalledCall, hCalledLine);
      OsTask::delay(1500);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("bench disconnect failed", 0,
                                   benchTrigger(nowait ? "disconnect nowait" : "disconnect", line));
      OsTask::delay(offset);

      // The customer's sequence: uninitialize with the call up and the
      // input device gone.
      unsigned long t0 = benchNowMs();
      SIPX_RESULT rc = sipxUnInitialize(g_hInst, true);
      unsigned long elapsed = benchNowMs() - t0;
      printf("bench iteration %d (%s) offset %d ms: sipxUnInitialize -> %d in %lu ms\n",
             it, nowait ? "in-flight" : "post-removal", offset, (int)rc, elapsed);
      fflush(stdout);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("sipxUnInitialize with the call up must succeed",
                                   SIPX_RESULT_SUCCESS, rc);
      CPPUNIT_ASSERT_MESSAGE("sipxUnInitialize not bounded after device departure",
                             elapsed < 10000);

      // The called side is still up on g_hInst2; drop it and its line.
      // Wait for the called call's DESTROYED before leaving the iteration.
      // It arrives seconds after sipxCallDestroy; if it lands after the next
      // iteration's resetAutoAnswerCallback, AutoAnswerCallback latches this
      // stale handle and the next iteration validates and destroys the
      // wrong call.
      EventValidator validatorCalledEnd("departure.called.end");
      validatorCalledEnd.ignoreMediaEvent(MEDIA_MIC_ENERGY_LEVEL);
      validatorCalledEnd.ignoreMediaEvent(MEDIA_SPEAKER_ENERGY_LEVEL);
      CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
         sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalledEnd));
      SIPX_CALL hEndedCall = hCalledCall;
      t0 = benchNowMs();
      CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxCallDestroy(hCalledCall));
      bool bDestroyed = validatorCalledEnd.waitForCallEvent(hCalledLine, hEndedCall,
                    CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false, 10);
      printf("bench iteration %d: called call DESTROYED %s after %lu ms\n",
             it, bDestroyed ? "received" : "MISSING", benchNowMs() - t0);
      fflush(stdout);
      CPPUNIT_ASSERT_MESSAGE("called: no DESTROYED within 10 s of sipxCallDestroy", bDestroyed);
      sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalledEnd);
      sipxLineRemove(hCalledLine);

      // Last iteration: also release every other instance, so the media
      // factory and its output driver are destroyed after the departure.
      // That is the single-instance application's teardown; while
      // g_hInst2..4 are alive the factory survives the uninitialize above.
      if (lastIt)
      {
         SIPX_INST* others[] = { &g_hInst2, &g_hInst3, &g_hInst4 };
         for (int i = 0; i < 3; i++)
         {
            t0 = benchNowMs();
            rc = sipxUnInitialize(*others[i], true);
            elapsed = benchNowMs() - t0;
            printf("bench iteration %d: sipxUnInitialize(g_hInst%d) -> %d in %lu ms%s\n",
                   it, i + 2, (int)rc, elapsed, (i == 2) ? " (last instance, factory teardown)" : "");
            fflush(stdout);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("sipxUnInitialize of remaining instance must succeed",
                                         SIPX_RESULT_SUCCESS, rc);
            CPPUNIT_ASSERT_MESSAGE("sipxUnInitialize of remaining instance not bounded",
                                   elapsed < 10000);
            *others[i] = NULL;
         }
      }

      // Reconnect before re-creating: sipXtapi copies the device list once,
      // in sipxInitialize (KNOWN DEFECT 3), so an instance created while the
      // device is gone never sees it.
      CPPUNIT_ASSERT_EQUAL_MESSAGE("bench connect failed", 0, benchTrigger("connect", line));
      if (lastIt)
      {
         benchRecreateInstance(g_hInst2, 9100, 9101, 9050, HINST2_ADDRESS);
         benchRecreateInstance(g_hInst3, 10000, 10001, 10050, HINST3_ADDRESS);
         benchRecreateInstance(g_hInst4, 12070, 12071, 12050, HINST4_ADDRESS);
      }
      benchRecreateInstance(g_hInst, 8000, 8001, 8050, HINST_ADDRESS);
   }
   OsTask::delay(TEST_DELAY);
   checkForLeaks();
}

// Path 3: recovery. After a departure cycle the device is back; a new call
// on a freshly selected device must bring media up on both sides.
void sipXtapiTestSuite::testDeviceReturnRecovery()
{
   UtlString match, offsets, line, chosen, chosenOut;
   int runs;
   if (!benchGate(match, runs, offsets)) return;

   for (int it = 0; it < runs; it++)
   {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("bench disconnect failed", 0, benchTrigger("disconnect", line));
      OsTask::delay(1000);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("bench connect failed", 0, benchTrigger("connect", line));

      // Fresh selection by name after the return: the factory's path.
      if (!benchSelectInputDevice(g_hInst, match, chosen))
      {
         CPPUNIT_ASSERT_MESSAGE("bench device not re-selectable as input after return", 0);
         return;
      }
      // Reselecting the output by the same name is the app-side recovery
      // for KNOWN DEFECT 4 (output stays on the fallback timer after the
      // device returns until it is reselected).
      if (!benchSelectOutputDevice(g_hInst, match, chosenOut))
      {
         CPPUNIT_ASSERT_MESSAGE("bench device not re-selectable as output after return", 0);
         return;
      }
      unsigned long t0 = benchNowMs();
      doCallBasic(g_hInst, "sip:calling@127.0.0.1:8000", "sip:calling@127.0.0.1:8000",
                  g_hInst2, "sip:called@127.0.0.1:9100", "sip:called@127.0.0.1:9100",
                  NULL, false, true);
      printf("bench iteration %d: post-return call completed in %lu ms on '%s' / '%s'\n",
             it, benchNowMs() - t0, chosen.data(), chosenOut.data());
      fflush(stdout);
   }
   OsTask::delay(TEST_DELAY);
   checkForLeaks();
}

